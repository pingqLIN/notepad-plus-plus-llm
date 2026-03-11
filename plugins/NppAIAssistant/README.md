# NppAIAssistant Plugin

`NppAIAssistant` is a lightweight AI assistant plugin for Notepad++.

Key characteristics:
- plugin-based, not hardwired into Notepad++ core
- single-turn request model
- visible prompt preview
- provider/model selection
- context menu actions for selected text
- DPAPI-backed local API key storage

## Build

```powershell
& "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" "plugins\NppAIAssistant\NppAIAssistant.vcxproj" /p:Configuration=Release /p:Platform=x64 /m
```

Output:
`build/NppAIAssistant/x64/Release/plugins/NppAIAssistant/NppAIAssistant.dll`

## Install

Copy the DLL to:
`<Notepad++>\plugins\NppAIAssistant\NppAIAssistant.dll`

Or use:
`scripts/install-npp-ai-plugin.ps1`

## Documentation

- Root overview: [../../README.md](../../README.md)
- Usage guide: [../../docs/USAGE.md](../../docs/USAGE.md)
- Project structure: [../../PROJECT_STRUCTURE.md](../../PROJECT_STRUCTURE.md)
