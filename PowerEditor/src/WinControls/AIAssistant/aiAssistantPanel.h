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

#pragma once

#include "DockingDlgInterface.h"
#include "aiAssistantPanel_rc.h"
#include "../../MISC/Common/LLMApiClient.h"
#include <string>
#include <vector>


#define AI_PANELTITLE L"AI Assistant"

class ScintillaEditView;
class AIAssistantSettings;

// LLM Provider types
enum class LLMProvider { OpenAI = 0, Gemini, Claude, Copilot, ProviderCount };

// Chat message structure
struct ChatMessage {
  bool isUser;
  std::wstring content;
  std::wstring timestamp;
};

// AI Assistant configuration
struct AIAssistantConfig {
  std::wstring openAIKey;
  std::wstring geminiKey;
  std::wstring claudeKey;
  std::wstring copilotKey;
  LLMProvider defaultProvider = LLMProvider::OpenAI;
};

class AIAssistantPanel : public DockingDlgInterface {
public:
  AIAssistantPanel()
      : DockingDlgInterface(IDD_AIASSISTANT_PANEL), _ppEditView(nullptr) {}

  void init(HINSTANCE hInst, HWND hPere, ScintillaEditView **ppEditView) {
    DockingDlgInterface::init(hInst, hPere);
    _ppEditView = ppEditView;
  }

  void setParent(HWND parent2set) { _hParent = parent2set; }

  // Send message to LLM
  void sendMessage(const std::wstring &message);

  // Get selected text from editor
  std::wstring getSelectedText();

  // Quick actions
  void explainCode();
  void refactorCode();
  void addComments();
  void fixCode();

  // Provider management
  void setProvider(LLMProvider provider);
  LLMProvider getProvider() const { return _currentProvider; }

  // Configuration
  void setConfig(const AIAssistantConfig &config) { _config = config; }
  AIAssistantConfig getConfig() const { return _config; }

  // Clear chat history
  void clearChatHistory();

  void setBackgroundColor(COLORREF bgColour) override { _bgColor = bgColour; }
  void setForegroundColor(COLORREF fgColour) override { _fgColor = fgColour; }

protected:
  intptr_t CALLBACK run_dlgProc(UINT message, WPARAM wParam,
                                LPARAM lParam) override;

private:
  ScintillaEditView **_ppEditView = nullptr;
  std::vector<ChatMessage> _chatHistory;
  LLMProvider _currentProvider = LLMProvider::OpenAI;
  AIAssistantConfig _config;
  COLORREF _bgColor = RGB(255, 255, 255);
  COLORREF _fgColor = RGB(0, 0, 0);
  
  // GitHub Copilot OAuth state
  CopilotTokens _copilotTokens;
  CopilotDeviceCode _copilotDeviceCode;
  bool _copilotAuthInProgress = false;
  UINT_PTR _copilotPollTimerId = 0;
  DWORD _copilotLastPendingTick = 0;
  DWORD _copilotPollIntervalMs = 5000;
  std::wstring _copilotLastDebugInfo;
  
  // Font management
  HFONT _chatFont = nullptr;
  int _fontSize = 10;
  
  // Model selection
  std::wstring _currentModel;

  // Helper functions
  void updateChatDisplay();
  void addMessageToChat(bool isUser, const std::wstring &content);
  std::wstring callLLMAPI(const std::wstring &prompt);
  std::wstring getProviderName(LLMProvider provider);
  void initControls();
  void resizeControls();
  
  // Copilot OAuth helpers
  void initiateCopilotSignIn();
  void pollCopilotAuth();
  void onCopilotAuthComplete(bool success);
  void loadCopilotTokenFromStorage();
  void saveCopilotTokenToStorage();
  
  // Theme and font helpers
  void applyDarkModeTheme();
  void updateChatFont();
  void increaseFontSize();
  void decreaseFontSize();
  void updateModelCombo();
};
