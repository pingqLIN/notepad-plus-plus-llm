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

// Resource IDs for settings dialog
#define IDD_AIASSISTANT_SETTINGS 30100
#define IDC_OPENAI_KEY_EDIT 30101
#define IDC_GEMINI_KEY_EDIT 30102
#define IDC_CLAUDE_KEY_EDIT 30103
#define IDC_COPILOT_KEY_EDIT 30104
#define IDC_DEFAULT_PROVIDER_COMBO 30105
#define IDC_TEST_CONNECTION_BTN 30106
#define IDOK_SETTINGS 30107
#define IDCANCEL_SETTINGS 30108

#ifndef RC_INVOKED
#include "StaticDialog.h"
#include "aiAssistantPanel.h"
#include <string>

class AIAssistantSettings : public StaticDialog {
public:
  AIAssistantSettings() : StaticDialog() {}

  void init(HINSTANCE hInst, HWND hParent);

  // Show settings dialog (modal)
  INT_PTR doDialog();

  // Get/Set configuration
  AIAssistantConfig getConfig() const { return _config; }
  void setConfig(const AIAssistantConfig &config) { _config = config; }

  // Load API keys from secure storage (static method)
  static AIAssistantConfig loadSecureConfig();

  // Check if settings were saved
  bool wasOkPressed() const { return _okPressed; }

protected:
  intptr_t CALLBACK run_dlgProc(UINT message, WPARAM wParam,
                                LPARAM lParam) override;

private:
  AIAssistantConfig _config;
  bool _okPressed = false;

  void loadSettingsToUI();
  void saveSettingsFromUI();
  void testConnection();
  void togglePasswordVisibility(int editId);
};
#endif
