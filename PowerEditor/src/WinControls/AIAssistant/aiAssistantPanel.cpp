// This file is part of Notepad++ project
// Copyright (C)2025 Don HO <don.h@free.fr>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "aiAssistantPanel.h"
#include "../../MISC/Common/LLMApiClient.h"
#include "../../MISC/Common/SecureStorage.h"
#include "NppDarkMode.h"
#include "ScintillaEditView.h"
#include "aiAssistantSettings.h"
#include <ctime>
#include <shellapi.h>
#include <sstream>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace {
constexpr size_t AI_INPUT_BUFFER_SIZE = 4096;
constexpr UINT_PTR COPILOT_POLL_TIMER_ID = 9001;
constexpr DWORD COPILOT_POLL_INTERVAL_MS = 5000;
constexpr int DEFAULT_FONT_SIZE = 10;
constexpr int MIN_FONT_SIZE = 8;
constexpr int MAX_FONT_SIZE = 18;
} // namespace

void AIAssistantPanel::initControls() {
  _config = AIAssistantSettings::loadSecureConfig();
  loadCopilotTokenFromStorage();

  HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_AI_PROVIDER_COMBO);
  if (hProviderCombo) {
    ::SendMessage(hProviderCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"OpenAI"));
    ::SendMessage(hProviderCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"Gemini"));
    ::SendMessage(hProviderCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"Claude"));
    ::SendMessage(hProviderCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"Copilot"));
    ::SendMessage(hProviderCombo, CB_SETCURSEL,
                  static_cast<int>(_currentProvider), 0);
  }

  updateModelCombo();
}

void AIAssistantPanel::resizeControls() {
  RECT rc;
  getClientRect(rc);

  int width = rc.right - rc.left;
  int height = rc.bottom - rc.top;

  // Resize controls based on panel size
  HWND hChatHistory = ::GetDlgItem(_hSelf, IDC_AI_CHAT_HISTORY);
  HWND hInputEdit = ::GetDlgItem(_hSelf, IDC_AI_INPUT_EDIT);
  HWND hSendButton = ::GetDlgItem(_hSelf, IDC_AI_SEND_BUTTON);

  if (hChatHistory && hInputEdit && hSendButton) {
    int inputHeight = 50;
    int buttonWidth = 50;
    int margin = 5;
    int topBarHeight = 30;

    // Chat history takes most of the space
    ::MoveWindow(hChatHistory, margin, topBarHeight + margin,
                 width - 2 * margin,
                 height - inputHeight - topBarHeight - 3 * margin, TRUE);

    // Input edit at the bottom
    ::MoveWindow(hInputEdit, margin, height - inputHeight - margin,
                 width - buttonWidth - 3 * margin, inputHeight, TRUE);

    // Send button
    ::MoveWindow(hSendButton, width - buttonWidth - margin,
                 height - inputHeight - margin, buttonWidth, inputHeight, TRUE);
  }
}

std::wstring AIAssistantPanel::getSelectedText() {
  if (!_ppEditView || !*_ppEditView)
    return L"";

  ScintillaEditView *pEditView = *_ppEditView;

  auto selStart = pEditView->execute(SCI_GETSELECTIONSTART);
  auto selEnd = pEditView->execute(SCI_GETSELECTIONEND);

  if (selStart == selEnd)
    return L"";

  size_t length = selEnd - selStart;
  std::vector<char> buffer(length + 1);
  pEditView->execute(SCI_GETSELTEXT, 0,
                     reinterpret_cast<LPARAM>(buffer.data()));
  buffer[length] = '\0';

  // Convert to wide string
  int wideLen = MultiByteToWideChar(CP_UTF8, 0, buffer.data(), -1, nullptr, 0);
  std::wstring result(wideLen - 1, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, buffer.data(), -1, &result[0], wideLen);

  return result;
}

void AIAssistantPanel::sendMessage(const std::wstring &message) {
  if (message.empty())
    return;

  // Add user message to chat
  addMessageToChat(true, message);

  // Call LLM API (placeholder - actual implementation would need HTTP client)
  std::wstring response = callLLMAPI(message);

  // Add response to chat
  addMessageToChat(false, response);

  // Update display
  updateChatDisplay();

  // Clear input
  HWND hInputEdit = ::GetDlgItem(_hSelf, IDC_AI_INPUT_EDIT);
  if (hInputEdit) {
    ::SetWindowText(hInputEdit, L"");
  }
}

void AIAssistantPanel::addMessageToChat(bool isUser,
                                        const std::wstring &content) {
  ChatMessage msg;
  msg.isUser = isUser;
  msg.content = content;

  // Get current timestamp
  time_t now = time(nullptr);
  struct tm timeinfo = {};
  errno_t err = localtime_s(&timeinfo, &now);
  wchar_t timestamp[64];
  if (err == 0) {
    wcsftime(timestamp, 64, L"%H:%M:%S", &timeinfo);
  } else {
    wcscpy_s(timestamp, L"--:--:--");
  }
  msg.timestamp = timestamp;

  _chatHistory.push_back(msg);
}

void AIAssistantPanel::updateChatDisplay() {
  HWND hChatHistory = ::GetDlgItem(_hSelf, IDC_AI_CHAT_HISTORY);
  if (!hChatHistory)
    return;

  std::wstringstream ss;
  for (const auto &msg : _chatHistory) {
    ss << L"[" << msg.timestamp << L"] ";
    ss << (msg.isUser ? L"You" : getProviderName(_currentProvider));
    ss << L":\r\n" << msg.content << L"\r\n\r\n";
  }

  ::SetWindowText(hChatHistory, ss.str().c_str());

  // Scroll to bottom
  int len =
      static_cast<int>(::SendMessage(hChatHistory, WM_GETTEXTLENGTH, 0, 0));
  ::SendMessage(hChatHistory, EM_SETSEL, len, len);
  ::SendMessage(hChatHistory, EM_SCROLLCARET, 0, 0);
}

std::wstring AIAssistantPanel::callLLMAPI(const std::wstring &prompt) {
  std::wstring providerName = getProviderName(_currentProvider);

  if (_currentProvider == LLMProvider::Copilot) {
    if (!_copilotTokens.isAuthenticated) {
      return L"[Notice] GitHub Copilot requires sign-in.\n\n"
             L"Please click the 'Sign in' button to authenticate with your "
             L"GitHub account.";
    }

    LLMResponse response =
        LLMApiClient::callCopilot(_copilotTokens, prompt, _currentModel);

    if (!_copilotTokens.isAuthenticated) {
      return L"[Error] Copilot session expired. Please sign in again.";
    }

    saveCopilotTokenToStorage();

    if (response.success) {
      return response.content;
    } else {
      return L"[Error] Copilot API call failed:\n" + response.errorMessage;
    }
  }

  std::wstring apiKey;
  switch (_currentProvider) {
  case LLMProvider::OpenAI:
    apiKey = _config.openAIKey;
    break;
  case LLMProvider::Gemini:
    apiKey = _config.geminiKey;
    break;
  case LLMProvider::Claude:
    apiKey = _config.claudeKey;
    break;
  default:
    break;
  }

  if (apiKey.empty()) {
    return L"[Error] API key not configured for " + providerName +
           L". Please go to Settings to configure your API key.";
  }

  LLMResponse response;

  switch (_currentProvider) {
  case LLMProvider::OpenAI:
    response = LLMApiClient::callOpenAI(apiKey, prompt, _currentModel);
    break;
  case LLMProvider::Gemini:
    response = LLMApiClient::callGemini(apiKey, prompt, _currentModel);
    break;
  case LLMProvider::Claude:
    response = LLMApiClient::callClaude(apiKey, prompt, _currentModel);
    break;
  default:
    return L"[Error] Unknown provider selected.";
  }

  if (response.success) {
    return response.content;
  } else {
    return L"[Error] " + providerName + L" API call failed:\n" +
           response.errorMessage;
  }
}

std::wstring AIAssistantPanel::getProviderName(LLMProvider provider) {
  switch (provider) {
  case LLMProvider::OpenAI:
    return L"OpenAI";
  case LLMProvider::Gemini:
    return L"Gemini";
  case LLMProvider::Claude:
    return L"Claude";
  case LLMProvider::Copilot:
    return L"Copilot";
  default:
    return L"Unknown";
  }
}

void AIAssistantPanel::setProvider(LLMProvider provider) {
  _currentProvider = provider;
  HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_AI_PROVIDER_COMBO);
  if (hProviderCombo) {
    ::SendMessage(hProviderCombo, CB_SETCURSEL, static_cast<int>(provider), 0);
  }
}

void AIAssistantPanel::clearChatHistory() {
  _chatHistory.clear();
  updateChatDisplay();
}

void AIAssistantPanel::explainCode() {
  std::wstring selectedText = getSelectedText();
  if (selectedText.empty()) {
    sendMessage(L"Please select some code first, then I can explain it.");
    return;
  }
  sendMessage(L"Please explain this code:\n\n" + selectedText);
}

void AIAssistantPanel::refactorCode() {
  std::wstring selectedText = getSelectedText();
  if (selectedText.empty()) {
    sendMessage(L"Please select some code first, then I can help refactor it.");
    return;
  }
  sendMessage(
      L"Please refactor this code for better readability and performance:\n\n" +
      selectedText);
}

void AIAssistantPanel::addComments() {
  std::wstring selectedText = getSelectedText();
  if (selectedText.empty()) {
    sendMessage(L"Please select some code first, then I can add comments.");
    return;
  }
  sendMessage(L"Please add appropriate comments to this code:\n\n" +
              selectedText);
}

void AIAssistantPanel::fixCode() {
  std::wstring selectedText = getSelectedText();
  if (selectedText.empty()) {
    sendMessage(
        L"Please select some code first, then I can help fix any issues.");
    return;
  }
  sendMessage(L"Please review this code and fix any bugs or issues:\n\n" +
              selectedText);
}

intptr_t CALLBACK AIAssistantPanel::run_dlgProc(UINT message, WPARAM wParam,
                                                LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG: {
    initControls();
    updateChatFont();
    applyDarkModeTheme();

    addMessageToChat(false, L"Welcome to AI Assistant! I can help you with:\n"
                            L"- Explaining code\n"
                            L"- Refactoring suggestions\n"
                            L"- Adding comments\n"
                            L"- Finding and fixing bugs\n\n"
                            L"Select some code and use the quick actions, or "
                            L"just ask me anything!");
    updateChatDisplay();
    return TRUE;
  }

  case WM_SIZE: {
    resizeControls();
    return TRUE;
  }

  case WM_DESTROY: {
    if (_chatFont) {
      ::DeleteObject(_chatFont);
      _chatFont = nullptr;
    }
    return TRUE;
  }

  case NPPM_INTERNAL_REFRESHDARKMODE: {
    applyDarkModeTheme();
    return TRUE;
  }

  case WM_COMMAND: {
    switch (LOWORD(wParam)) {
    case IDC_AI_SEND_BUTTON: {
      wchar_t buffer[AI_INPUT_BUFFER_SIZE];
      HWND hInputEdit = ::GetDlgItem(_hSelf, IDC_AI_INPUT_EDIT);
      if (hInputEdit) {
        ::GetWindowText(hInputEdit, buffer, AI_INPUT_BUFFER_SIZE);
        sendMessage(buffer);
      }
      return TRUE;
    }

    case IDC_AI_CLEAR_BUTTON: {
      clearChatHistory();
      return TRUE;
    }

    case IDC_AI_PROVIDER_COMBO: {
      if (HIWORD(wParam) == CBN_SELCHANGE) {
        HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_AI_PROVIDER_COMBO);
        int sel =
            static_cast<int>(::SendMessage(hProviderCombo, CB_GETCURSEL, 0, 0));
        if (sel >= 0 && sel < static_cast<int>(LLMProvider::ProviderCount)) {
          _currentProvider = static_cast<LLMProvider>(sel);
          updateModelCombo();
        }
      }
      return TRUE;
    }

    case IDC_AI_MODEL_COMBO: {
      if (HIWORD(wParam) == CBN_SELCHANGE) {
        HWND hModelCombo = ::GetDlgItem(_hSelf, IDC_AI_MODEL_COMBO);
        int sel =
            static_cast<int>(::SendMessage(hModelCombo, CB_GETCURSEL, 0, 0));
        if (sel >= 0) {
          wchar_t buffer[128];
          ::SendMessage(hModelCombo, CB_GETLBTEXT, sel,
                        reinterpret_cast<LPARAM>(buffer));
          _currentModel = buffer;
        }
      }
      return TRUE;
    }

    case IDC_AI_SETTINGS_BUTTON: {
      AIAssistantSettings settingsDlg;
      settingsDlg.init(_hInst, _hSelf);
      settingsDlg.setConfig(_config);

      if (settingsDlg.doDialog() == IDOK) {
        _config = settingsDlg.getConfig();
        _currentProvider = _config.defaultProvider;

        HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_AI_PROVIDER_COMBO);
        if (hProviderCombo) {
          ::SendMessage(hProviderCombo, CB_SETCURSEL,
                        static_cast<int>(_currentProvider), 0);
        }
      }
      return TRUE;
    }

    case IDC_AI_COPILOT_SIGNIN_BUTTON: {
      if (_copilotAuthInProgress) {
        ::KillTimer(_hSelf, COPILOT_POLL_TIMER_ID);
        _copilotAuthInProgress = false;
        addMessageToChat(false, L"Copilot sign-in cancelled.");
        updateChatDisplay();
        updateModelCombo();
      } else {
        initiateCopilotSignIn();
      }
      return TRUE;
    }

    case IDC_AI_FONT_INCREASE_BUTTON: {
      increaseFontSize();
      return TRUE;
    }

    case IDC_AI_FONT_DECREASE_BUTTON: {
      decreaseFontSize();
      return TRUE;
    }
    }
    break;
  }

  case WM_TIMER: {
    if (wParam == COPILOT_POLL_TIMER_ID) {
      pollCopilotAuth();
      return TRUE;
    }
    break;
  }

  case WM_CTLCOLOREDIT:
  case WM_CTLCOLORSTATIC: {
    if (NppDarkMode::isEnabled()) {
      return NppDarkMode::onCtlColorCtrl(reinterpret_cast<HDC>(wParam));
    }
    break;
  }

  case WM_ERASEBKGND: {
    if (NppDarkMode::isEnabled()) {
      RECT rc{};
      getClientRect(rc);
      ::FillRect(reinterpret_cast<HDC>(wParam), &rc,
                 NppDarkMode::getDlgBackgroundBrush());
      return TRUE;
    }
    break;
  }

  default:
    return DockingDlgInterface::run_dlgProc(message, wParam, lParam);
  }

  return FALSE;
}

void AIAssistantPanel::initiateCopilotSignIn() {
  if (_copilotAuthInProgress) {
    return;
  }

  _copilotDeviceCode = LLMApiClient::initiateCopilotDeviceFlow();

  if (_copilotDeviceCode.userCode.empty() ||
      _copilotDeviceCode.deviceCode.empty()) {
    std::wstring message =
        L"[Error] Failed to initiate GitHub Copilot sign-in. Please try again.";
    std::wstring debugInfo = LLMApiClient::getLastCopilotAuthDebug();
    if (!debugInfo.empty()) {
      message += L"\n\nDetails:\n" + debugInfo;
    }
    addMessageToChat(false, message);
    updateChatDisplay();
    return;
  }

  _copilotAuthInProgress = true;
  _copilotLastPendingTick = GetTickCount();
  _copilotLastDebugInfo.clear();
  updateModelCombo();

  std::wstring authMessage =
      L"To sign in to GitHub Copilot:\n\n"
      L"1. Go to: " +
      _copilotDeviceCode.verificationUri +
      L"\n"
      L"2. Enter code: " +
      _copilotDeviceCode.userCode +
      L"\n\n"
      L"Waiting for authorization...\n"
      L"This can take up to a couple of minutes. Keep this window open.";

  addMessageToChat(false, authMessage);
  updateChatDisplay();

  ::ShellExecuteW(nullptr, L"open", _copilotDeviceCode.verificationUri.c_str(),
                  nullptr, nullptr, SW_SHOWNORMAL);

  DWORD interval = static_cast<DWORD>(_copilotDeviceCode.interval) * 1000;
  if (interval < COPILOT_POLL_INTERVAL_MS) {
    interval = COPILOT_POLL_INTERVAL_MS;
  }
  _copilotPollIntervalMs = interval;
  ::SetTimer(_hSelf, COPILOT_POLL_TIMER_ID, _copilotPollIntervalMs, nullptr);
}

void AIAssistantPanel::pollCopilotAuth() {
  if (!_copilotAuthInProgress) {
    ::KillTimer(_hSelf, COPILOT_POLL_TIMER_ID);
    return;
  }

  CopilotTokens tempTokens;
  int pollResult = LLMApiClient::pollCopilotAccessToken(
      _copilotDeviceCode.deviceCode, tempTokens);

  if (pollResult == 1 && !tempTokens.oauthToken.empty()) {
    _copilotTokens = tempTokens;
    onCopilotAuthComplete(true);
  } else if (pollResult == -1) {
    onCopilotAuthComplete(false);
  } else if (pollResult == 0) {
    std::wstring debugInfo = LLMApiClient::getLastCopilotAuthDebug();
    if (debugInfo.find(L"slow_down") != std::wstring::npos) {
      _copilotPollIntervalMs += 5000;
      ::KillTimer(_hSelf, COPILOT_POLL_TIMER_ID);
      ::SetTimer(_hSelf, COPILOT_POLL_TIMER_ID, _copilotPollIntervalMs,
                 nullptr);
      addMessageToChat(
          false, L"GitHub asked to slow down. Polling less frequently...");
      updateChatDisplay();
    }
    if (!debugInfo.empty() && debugInfo != _copilotLastDebugInfo) {
      addMessageToChat(false, L"Last response:\n" + debugInfo);
      updateChatDisplay();
      _copilotLastDebugInfo = debugInfo;
    }
    DWORD nowTick = GetTickCount();
    if (nowTick - _copilotLastPendingTick >= 30000) {
      addMessageToChat(false, L"Still waiting for GitHub authorization...");
      updateChatDisplay();
      _copilotLastPendingTick = nowTick;
    }
  }
}

void AIAssistantPanel::onCopilotAuthComplete(bool success) {
  ::KillTimer(_hSelf, COPILOT_POLL_TIMER_ID);
  _copilotAuthInProgress = false;

  if (success) {
    saveCopilotTokenToStorage();
    addMessageToChat(false, L"Successfully signed in to GitHub Copilot! You "
                            L"can now use Copilot as your AI provider.");
  } else {
    std::wstring message =
        L"[Error] Failed to complete GitHub Copilot sign-in. Please try again.";
    std::wstring debugInfo = LLMApiClient::getLastCopilotAuthDebug();
    if (!debugInfo.empty()) {
      message += L"\n\nDetails:\n" + debugInfo;
    }
    addMessageToChat(false, message);
  }
  updateChatDisplay();
  updateModelCombo();
}

void AIAssistantPanel::loadCopilotTokenFromStorage() {
  std::wstring oauthToken = SecureStorage::loadApiKey(L"copilot_oauth_token");
  if (!oauthToken.empty()) {
    _copilotTokens.oauthToken = oauthToken;
    if (LLMApiClient::refreshCopilotToken(_copilotTokens)) {
      _copilotTokens.isAuthenticated = true;
    }
  }
}

void AIAssistantPanel::saveCopilotTokenToStorage() {
  if (!_copilotTokens.oauthToken.empty()) {
    SecureStorage::saveApiKey(L"copilot_oauth_token",
                              _copilotTokens.oauthToken);
  }
}

void AIAssistantPanel::applyDarkModeTheme() {
  NppDarkMode::autoSubclassAndThemeChildControls(_hSelf);
  NppDarkMode::autoSubclassAndThemeWindowNotify(_hSelf);

  if (NppDarkMode::isEnabled()) {
    _bgColor = NppDarkMode::getCtrlBackgroundColor();
    _fgColor = NppDarkMode::getTextColor();
  } else {
    _bgColor = RGB(255, 255, 255);
    _fgColor = RGB(0, 0, 0);
  }

  ::InvalidateRect(_hSelf, nullptr, TRUE);
}

void AIAssistantPanel::updateChatFont() {
  if (_chatFont) {
    ::DeleteObject(_chatFont);
  }

  HDC hdc = ::GetDC(_hSelf);
  int logPixelsY = ::GetDeviceCaps(hdc, LOGPIXELSY);
  ::ReleaseDC(_hSelf, hdc);

  int fontHeight = -MulDiv(_fontSize, logPixelsY, 72);

  _chatFont = ::CreateFontW(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            DEFAULT_PITCH | FF_DONTCARE, L"Consolas");

  if (!_chatFont) {
    _chatFont = ::CreateFontW(fontHeight, 0, 0, 0, FW_NORMAL, FALSE, FALSE,
                              FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                              CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                              DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
  }

  HWND hChatHistory = ::GetDlgItem(_hSelf, IDC_AI_CHAT_HISTORY);
  HWND hInputEdit = ::GetDlgItem(_hSelf, IDC_AI_INPUT_EDIT);

  if (hChatHistory && _chatFont) {
    ::SendMessage(hChatHistory, WM_SETFONT, reinterpret_cast<WPARAM>(_chatFont),
                  TRUE);
  }
  if (hInputEdit && _chatFont) {
    ::SendMessage(hInputEdit, WM_SETFONT, reinterpret_cast<WPARAM>(_chatFont),
                  TRUE);
  }
}

void AIAssistantPanel::increaseFontSize() {
  if (_fontSize < MAX_FONT_SIZE) {
    _fontSize += 1;
    updateChatFont();
  }
}

void AIAssistantPanel::decreaseFontSize() {
  if (_fontSize > MIN_FONT_SIZE) {
    _fontSize -= 1;
    updateChatFont();
  }
}

void AIAssistantPanel::updateModelCombo() {
  HWND hModelCombo = ::GetDlgItem(_hSelf, IDC_AI_MODEL_COMBO);
  HWND hSignInBtn = ::GetDlgItem(_hSelf, IDC_AI_COPILOT_SIGNIN_BUTTON);
  if (!hModelCombo)
    return;

  ::SendMessage(hModelCombo, CB_RESETCONTENT, 0, 0);

  bool isCopilot = (_currentProvider == LLMProvider::Copilot);
  if (hSignInBtn) {
    ::ShowWindow(hSignInBtn, isCopilot ? SW_SHOW : SW_HIDE);
    if (isCopilot && _copilotAuthInProgress) {
      ::SetWindowTextW(hSignInBtn, L"Cancel");
      ::EnableWindow(hSignInBtn, TRUE);
    } else if (isCopilot && _copilotTokens.isAuthenticated) {
      ::SetWindowTextW(hSignInBtn, L"Signed");
      ::EnableWindow(hSignInBtn, FALSE);
    } else if (isCopilot) {
      ::SetWindowTextW(hSignInBtn, L"Sign in");
      ::EnableWindow(hSignInBtn, TRUE);
    }
  }

  switch (_currentProvider) {
  case LLMProvider::OpenAI:
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gpt-4o"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gpt-4o-mini"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gpt-4-turbo"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"o1"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"o1-mini"));
    _currentModel = L"gpt-4o-mini";
    break;
  case LLMProvider::Gemini:
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gemini-2.0-flash"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gemini-1.5-pro"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gemini-1.5-flash"));
    _currentModel = L"gemini-2.0-flash";
    break;
  case LLMProvider::Claude:
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"claude-sonnet-4-20250514"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"claude-3-5-sonnet-20241022"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"claude-3-5-haiku-20241022"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"claude-3-opus-20240229"));
    _currentModel = L"claude-sonnet-4-20250514";
    break;
  case LLMProvider::Copilot:
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gpt-4o"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"gpt-4"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"claude-3.5-sonnet"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"o1-mini"));
    ::SendMessage(hModelCombo, CB_ADDSTRING, 0,
                  reinterpret_cast<LPARAM>(L"o1-preview"));
    _currentModel = L"gpt-4o";
    break;
  default:
    break;
  }

  ::SendMessage(hModelCombo, CB_SETCURSEL, 0, 0);

  // Ensure dropdown is wide enough for long model names (e.g.
  // "claude-sonnet-4-20250514")
  ::SendMessage(hModelCombo, CB_SETDROPPEDWIDTH, 250, 0);
}

void AIAssistantPanel::setPromptText(const std::wstring &text) {
  HWND hInput = ::GetDlgItem(_hSelf, IDC_AI_INPUT_EDIT);
  if (hInput) {
    ::SetWindowTextW(hInput, text.c_str());
    // Auto-send the prompt
    sendMessage(text);
  }
}
