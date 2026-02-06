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

// AI Assistant Panel Dialog ID (using 8100+ range to avoid conflicts)
#define IDD_AIASSISTANT_PANEL          8100

// AI Assistant Panel Controls
#define IDC_AI_CHAT_HISTORY            (IDD_AIASSISTANT_PANEL + 1)
#define IDC_AI_INPUT_EDIT              (IDD_AIASSISTANT_PANEL + 2)
#define IDC_AI_SEND_BUTTON             (IDD_AIASSISTANT_PANEL + 3)
#define IDC_AI_PROVIDER_COMBO          (IDD_AIASSISTANT_PANEL + 4)
#define IDC_AI_CLEAR_BUTTON            (IDD_AIASSISTANT_PANEL + 5)
#define IDC_AI_SETTINGS_BUTTON         (IDD_AIASSISTANT_PANEL + 6)
#define IDC_AI_COPILOT_SIGNIN_BUTTON   (IDD_AIASSISTANT_PANEL + 7)
#define IDC_AI_COPILOT_STATUS_STATIC   (IDD_AIASSISTANT_PANEL + 8)
#define IDC_AI_FONT_INCREASE_BUTTON    (IDD_AIASSISTANT_PANEL + 9)
#define IDC_AI_FONT_DECREASE_BUTTON    (IDD_AIASSISTANT_PANEL + 10)
#define IDC_AI_MODEL_COMBO             (IDD_AIASSISTANT_PANEL + 11)

// AI Floating Toolbar Dialog ID
#define IDD_AI_FLOATING_TOOLBAR        8120
#define IDC_AI_BTN_EXPLAIN             (IDD_AI_FLOATING_TOOLBAR + 1)
#define IDC_AI_BTN_REFACTOR            (IDD_AI_FLOATING_TOOLBAR + 2)
#define IDC_AI_BTN_COMMENT             (IDD_AI_FLOATING_TOOLBAR + 3)
#define IDC_AI_BTN_FIX                 (IDD_AI_FLOATING_TOOLBAR + 4)
#define IDC_AI_BTN_MORE                (IDD_AI_FLOATING_TOOLBAR + 5)

// AI Floating Dialog ID
#define IDD_AI_FLOATING_DIALOG         8140
#define IDC_AI_FLOATING_CHAT_HISTORY   (IDD_AI_FLOATING_DIALOG + 1)
#define IDC_AI_FLOATING_INPUT_EDIT     (IDD_AI_FLOATING_DIALOG + 2)
#define IDC_AI_FLOATING_SEND_BUTTON    (IDD_AI_FLOATING_DIALOG + 3)

// AI Settings Dialog ID
#define IDD_AI_SETTINGS_DIALOG         8160
#define IDC_AI_OPENAI_KEY_EDIT         (IDD_AI_SETTINGS_DIALOG + 1)
#define IDC_AI_GEMINI_KEY_EDIT         (IDD_AI_SETTINGS_DIALOG + 2)
#define IDC_AI_CLAUDE_KEY_EDIT         (IDD_AI_SETTINGS_DIALOG + 3)
#define IDC_AI_COPILOT_KEY_EDIT        (IDD_AI_SETTINGS_DIALOG + 4)
#define IDC_AI_DEFAULT_PROVIDER_COMBO  (IDD_AI_SETTINGS_DIALOG + 5)
