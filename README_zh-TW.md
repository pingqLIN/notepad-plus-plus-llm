# NppAIAssistant for Notepad++

[English](README.md)

一個為 Notepad++ 打造的輕量化 AI 助手外掛，強調流程可見、行為可預期、部署方式貼近原生 Notepad++ 外掛體驗。

本專案將 AI 能力包裝為一般 Notepad++ 外掛，而不是把功能深度綁進核心程式。這樣的做法更容易安裝、更容易維護，也更適合整理後公開推送到 GitHub。

## 為什麼做這個專案

許多編輯器 AI 整合在使用一段時間後會變得不透明：系統提示詞看不到、記憶行為不清楚、與主程式耦合過深，後續調整與維護成本都很高。

`NppAIAssistant` 走的是另一條路：

- 採用輕量化外掛架構，而不是永久性修改 Notepad++ 核心
- 提供提示詞可視性，能直接看到實際送出的 prompt
- 採用單輪對話模型，不在不同請求之間偷偷保留記憶
- Provider 與模型可在登入或設定 API Key 後動態取得
- 內建右鍵快捷 AI 操作，適合編輯器內快速處理文字或程式碼
- 介面方向兼顧 English 與繁體中文使用情境

## 專案特色

### 輕量化設計
AI 助手以獨立 Notepad++ 外掛形式運作。這代表主程式更乾淨、安裝方式更接近既有外掛習慣，也讓後續功能演進不必把大量 AI 邏輯重新綁回主程式執行檔。

### 提示詞可視性
設定視窗內建 `Prompt Preview`。當你切換 preset、回覆語言、編碼建議、回覆詳略、情境模組或輸出規則時，畫面上的提示詞預覽會同步更新。

### 單輪對話，不保留隱藏記憶
每一次請求都被刻意設計成單輪。外掛不依賴跨請求的聊天記憶，每次回覆只基於當前請求內容生成，讓行為更容易推測，也更容易除錯與審查。

### 面向編輯器工作流
常用 AI 動作可以直接從右鍵選單觸發，選取文字後就能快速做解釋、重構、加註解或修正。

## 畫面展示

### 設定中的提示詞預覽
![Prompt Preview](docs/assets/screenshots/settings-prompt-preview.png)

### 以 Preset 驅動的單輪 Prompt Builder
![Preset Dropdown](docs/assets/screenshots/settings-preset-dropdown.png)

### 右鍵 AI 操作
![Context Menu Actions](docs/assets/screenshots/context-menu-actions.png)

## 安裝方式

### 標準 Notepad++ 外掛安裝結構
1. 以 `Release | x64` 建置專案。
2. 將產生的 DLL 複製到：
   `<Notepad++>\plugins\NppAIAssistant\NppAIAssistant.dll`
3. 重新啟動 Notepad++。

目前 DLL 輸出位置：
`build/NppAIAssistant/x64/Release/plugins/NppAIAssistant/NppAIAssistant.dll`

### 可選安裝腳本
專案也提供安裝腳本：
`scripts/install-npp-ai-plugin.ps1`

## 使用方式

1. 從外掛選單開啟 AI 面板。
2. 在設定中填入 Provider 的 API Key。
3. 讓外掛依據目前 Provider 動態載入模型。
4. 選擇 preset，或手動調整單輪 prompt 設定。
5. 在送出前先檢查提示詞預覽。
6. 可直接在 AI 面板提問，或先選取文字後使用右鍵 AI 功能。

延伸文件：
- [使用說明](docs/USAGE.md)
- [專案結構](PROJECT_STRUCTURE.md)
- [外掛建置說明](plugins/NppAIAssistant/README.md)

## 專案結構

雖然此 repo 仍保留上游 Notepad++ 原始碼樹，但 AI 相關工作目前主要集中在：

- `plugins/NppAIAssistant/`：外掛本體實作
- `PowerEditor/src/MISC/Common/`：HTTP、Provider 與安全儲存等共用基礎元件
- `docs/`：GitHub 用文件與展示截圖

更多說明可見：
- [專案結構](PROJECT_STRUCTURE.md)

## 隱私與公開整理說明

這個 repo 已開始朝公開分享方向整理。本地工作痕跡與內部討論殘留會盡量排除在 Git 追蹤之外。

此外掛的互動模型刻意保持明確：
- 提示詞可以預覽
- 請求採單輪方式
- 不預設攜帶跨請求記憶

## 建置

若以外掛為主進行開發，Windows 下可用 MSBuild：

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "plugins\NppAIAssistant\NppAIAssistant.vcxproj" /p:Configuration=Release /p:Platform=x64 /m
```

## 目前方向

這是一個持續整理中的 Notepad++ AI 外掛改造專案，目前重點放在：
- 安裝方式務實可用
- 提示詞透明可視
- 降低與主程式耦合
- 維持可預期的單輪行為
