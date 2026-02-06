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

#include "aiAssistantSettings.h"
#include "../../MISC/Common/LLMApiClient.h"
#include "../../MISC/Common/SecureStorage.h"
#include "NppDarkMode.h"
#include <CommCtrl.h>

// Key names for secure storage
static const wchar_t *OPENAI_KEY_NAME = L"openai_apikey";
static const wchar_t *GEMINI_KEY_NAME = L"gemini_apikey";
static const wchar_t *CLAUDE_KEY_NAME = L"claude_apikey";
static const wchar_t *COPILOT_KEY_NAME = L"copilot_apikey";

void AIAssistantSettings::init(HINSTANCE hInst, HWND hParent) {
  Window::init(hInst, hParent);
}

INT_PTR AIAssistantSettings::doDialog() {
  _okPressed = false;
  return ::DialogBoxParam(_hInst, MAKEINTRESOURCE(IDD_AIASSISTANT_SETTINGS),
                          _hParent, dlgProc, reinterpret_cast<LPARAM>(this));
}

void AIAssistantSettings::loadSettingsToUI() {
  // OpenAI Key
  HWND hOpenAI = ::GetDlgItem(_hSelf, IDC_OPENAI_KEY_EDIT);
  if (hOpenAI && !_config.openAIKey.empty()) {
    ::SetWindowText(hOpenAI, _config.openAIKey.c_str());
  }

  // Gemini Key
  HWND hGemini = ::GetDlgItem(_hSelf, IDC_GEMINI_KEY_EDIT);
  if (hGemini && !_config.geminiKey.empty()) {
    ::SetWindowText(hGemini, _config.geminiKey.c_str());
  }

  // Claude Key
  HWND hClaude = ::GetDlgItem(_hSelf, IDC_CLAUDE_KEY_EDIT);
  if (hClaude && !_config.claudeKey.empty()) {
    ::SetWindowText(hClaude, _config.claudeKey.c_str());
  }

  // Copilot Key
  HWND hCopilot = ::GetDlgItem(_hSelf, IDC_COPILOT_KEY_EDIT);
  if (hCopilot && !_config.copilotKey.empty()) {
    ::SetWindowText(hCopilot, _config.copilotKey.c_str());
  }

  // Default provider combo
  HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_DEFAULT_PROVIDER_COMBO);
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
                  static_cast<int>(_config.defaultProvider), 0);
  }

  // Set password style for key edit boxes
  ::SendMessage(hOpenAI, EM_SETPASSWORDCHAR, static_cast<WPARAM>(L'●'), 0);
  ::SendMessage(hGemini, EM_SETPASSWORDCHAR, static_cast<WPARAM>(L'●'), 0);
  ::SendMessage(hClaude, EM_SETPASSWORDCHAR, static_cast<WPARAM>(L'●'), 0);
  ::SendMessage(hCopilot, EM_SETPASSWORDCHAR, static_cast<WPARAM>(L'●'), 0);
}

void AIAssistantSettings::saveSettingsFromUI() {
  wchar_t buffer[512] = {};

  // OpenAI Key
  HWND hOpenAI = ::GetDlgItem(_hSelf, IDC_OPENAI_KEY_EDIT);
  if (hOpenAI) {
    ::GetWindowText(hOpenAI, buffer, 512);
    _config.openAIKey = buffer;
    // Save to secure storage
    SecureStorage::saveApiKey(OPENAI_KEY_NAME, _config.openAIKey);
  }

  // Gemini Key
  HWND hGemini = ::GetDlgItem(_hSelf, IDC_GEMINI_KEY_EDIT);
  if (hGemini) {
    ::GetWindowText(hGemini, buffer, 512);
    _config.geminiKey = buffer;
    SecureStorage::saveApiKey(GEMINI_KEY_NAME, _config.geminiKey);
  }

  // Claude Key
  HWND hClaude = ::GetDlgItem(_hSelf, IDC_CLAUDE_KEY_EDIT);
  if (hClaude) {
    ::GetWindowText(hClaude, buffer, 512);
    _config.claudeKey = buffer;
    SecureStorage::saveApiKey(CLAUDE_KEY_NAME, _config.claudeKey);
  }

  // Copilot Key
  HWND hCopilot = ::GetDlgItem(_hSelf, IDC_COPILOT_KEY_EDIT);
  if (hCopilot) {
    ::GetWindowText(hCopilot, buffer, 512);
    _config.copilotKey = buffer;
    SecureStorage::saveApiKey(COPILOT_KEY_NAME, _config.copilotKey);
  }

  // Default provider
  HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_DEFAULT_PROVIDER_COMBO);
  if (hProviderCombo) {
    int sel =
        static_cast<int>(::SendMessage(hProviderCombo, CB_GETCURSEL, 0, 0));
    if (sel >= 0 && sel < static_cast<int>(LLMProvider::ProviderCount)) {
      _config.defaultProvider = static_cast<LLMProvider>(sel);
    }
  }
}

// Static method to load API keys from secure storage
AIAssistantConfig AIAssistantSettings::loadSecureConfig() {
  AIAssistantConfig config;
  config.openAIKey = SecureStorage::loadApiKey(OPENAI_KEY_NAME);
  config.geminiKey = SecureStorage::loadApiKey(GEMINI_KEY_NAME);
  config.claudeKey = SecureStorage::loadApiKey(CLAUDE_KEY_NAME);
  config.copilotKey = SecureStorage::loadApiKey(COPILOT_KEY_NAME);
  return config;
}

void AIAssistantSettings::testConnection() {
  // Get the currently selected provider
  HWND hProviderCombo = ::GetDlgItem(_hSelf, IDC_DEFAULT_PROVIDER_COMBO);
  int sel = static_cast<int>(::SendMessage(hProviderCombo, CB_GETCURSEL, 0, 0));

  // Save current settings first
  saveSettingsFromUI();

  std::wstring apiKey;
  std::wstring providerName;
  LLMProvider provider = static_cast<LLMProvider>(sel);

  switch (provider) {
  case LLMProvider::OpenAI:
    apiKey = _config.openAIKey;
    providerName = L"OpenAI";
    break;
  case LLMProvider::Gemini:
    apiKey = _config.geminiKey;
    providerName = L"Gemini";
    break;
  case LLMProvider::Claude:
    apiKey = _config.claudeKey;
    providerName = L"Claude";
    break;
  case LLMProvider::Copilot:
    ::MessageBox(_hSelf,
                 L"GitHub Copilot requires OAuth authentication.\nTest "
                 L"connection is not available for this provider.",
                 L"Test Connection", MB_OK | MB_ICONINFORMATION);
    return;
  default:
    return;
  }

  if (apiKey.empty()) {
    ::MessageBox(_hSelf,
                 (L"No API key configured for " + providerName + L".").c_str(),
                 L"Test Connection", MB_OK | MB_ICONWARNING);
    return;
  }

  // Set cursor to wait
  HCURSOR hOldCursor = ::SetCursor(::LoadCursor(nullptr, IDC_WAIT));

  // Test with a simple prompt
  LLMResponse response;
  switch (provider) {
  case LLMProvider::OpenAI:
    response = LLMApiClient::callOpenAI(
        apiKey, L"Hello, please respond with 'Connection successful!'");
    break;
  case LLMProvider::Gemini:
    response = LLMApiClient::callGemini(
        apiKey, L"Hello, please respond with 'Connection successful!'");
    break;
  case LLMProvider::Claude:
    response = LLMApiClient::callClaude(
        apiKey, L"Hello, please respond with 'Connection successful!'");
    break;
  default:
    break;
  }

  // Restore cursor
  ::SetCursor(hOldCursor);

  if (response.success) {
    ::MessageBox(_hSelf,
                 (L"Connection to " + providerName +
                  L" successful!\n\nResponse: " +
                  response.content.substr(0, 200))
                     .c_str(),
                 L"Test Connection", MB_OK | MB_ICONINFORMATION);
  } else {
    ::MessageBox(_hSelf,
                 (L"Connection to " + providerName + L" failed:\n\n" +
                  response.errorMessage)
                     .c_str(),
                 L"Test Connection", MB_OK | MB_ICONERROR);
  }
}

intptr_t CALLBACK AIAssistantSettings::run_dlgProc(UINT message, WPARAM wParam,
                                                   LPARAM lParam) {
  UNREFERENCED_PARAMETER(lParam);
  switch (message) {
  case WM_INITDIALOG: {
    loadSettingsToUI();

    // Apply dark mode if enabled
    if (NppDarkMode::isEnabled()) {
      NppDarkMode::autoSubclassAndThemeChildControls(_hSelf);
    }

    // Center dialog on parent
    RECT rcParent, rcDlg;
    ::GetWindowRect(_hParent, &rcParent);
    ::GetWindowRect(_hSelf, &rcDlg);
    int x = (rcParent.left + rcParent.right - (rcDlg.right - rcDlg.left)) / 2;
    int y = (rcParent.top + rcParent.bottom - (rcDlg.bottom - rcDlg.top)) / 2;
    ::SetWindowPos(_hSelf, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    return TRUE;
  }

  case WM_COMMAND: {
    switch (LOWORD(wParam)) {
    case IDOK_SETTINGS:
    case IDOK: {
      saveSettingsFromUI();
      _okPressed = true;
      ::EndDialog(_hSelf, IDOK);
      return TRUE;
    }

    case IDCANCEL_SETTINGS:
    case IDCANCEL: {
      ::EndDialog(_hSelf, IDCANCEL);
      return TRUE;
    }

    case IDC_TEST_CONNECTION_BTN: {
      testConnection();
      return TRUE;
    }
    }
    break;
  }

  case WM_ERASEBKGND: {
    if (NppDarkMode::isEnabled()) {
      RECT rc{};
      ::GetClientRect(_hSelf, &rc);
      ::FillRect(reinterpret_cast<HDC>(wParam), &rc,
                 NppDarkMode::getDlgBackgroundBrush());
      return TRUE;
    }
    break;
  }
  }

  return FALSE;
}
