# Project Structure

This repository is a Notepad++ source tree plus a lightweight AI plugin layer.

The goal is to keep AI behavior modular and publishable without turning the whole editor into an AI-specific fork.

## Recommended Public-Facing Structure

### Root
- `README.md`
  Project overview, value proposition, screenshots, install summary.
- `BUILD.md`
  General build notes inherited from the Notepad++ source tree.
- `PROJECT_STRUCTURE.md`
  Developer map for this fork.
- `.gitignore`
  Includes extra rules to avoid committing local AI working artifacts.

### Documentation
- `docs/USAGE.md`
  User-focused setup and workflow guide.
- `docs/assets/screenshots/`
  Curated screenshots used by GitHub documentation.

### Plugin
- `plugins/NppAIAssistant/`
  Main plugin project.
- `plugins/NppAIAssistant/src/NppAIAssistant.cpp`
  Dockable panel, settings UI, commands, prompt builder, send flow.
- `plugins/NppAIAssistant/src/NppAIAssistantResources.h`
  Resource IDs for plugin dialogs and controls.
- `plugins/NppAIAssistant/src/NppAIAssistantResources.rc`
  Dialog layouts and resource wiring.
- `plugins/NppAIAssistant/README.md`
  Plugin-specific build and deployment notes.

### Shared Infrastructure
- `PowerEditor/src/MISC/Common/HttpClient.*`
  WinHTTP wrapper used by plugin-side provider calls.
- `PowerEditor/src/MISC/Common/LLMApiClient.*`
  OpenAI, Gemini, Claude, and related provider integration helpers.
- `PowerEditor/src/MISC/Common/SecureStorage.*`
  DPAPI-backed local storage for API keys and related settings.

## What Should Stay Out of GitHub Commits

These are local working artifacts, not public project assets:
- ad hoc patch dumps such as `diff*.patch`
- temporary backup folders such as `tmp_backup/`
- raw desktop screenshots stored at repo root
- local experiment files that are not part of docs, build, or source

For public screenshots, use:
- `docs/assets/screenshots/`

## Mental Model for Contributors

Use this rule of thumb:

- Edit `plugins/NppAIAssistant/` for plugin UX and AI behavior
- Edit `PowerEditor/src/MISC/Common/` for reusable transport/provider/storage code
- Avoid re-introducing AI-specific coupling into broad Notepad++ core files unless it is truly necessary

## Repository Publishing Notes

If this repo is pushed to GitHub, keep the public story focused on:
- lightweight plugin architecture
- visible prompts
- single-turn, no-memory behavior
- practical context-menu driven editing workflow

Avoid publishing:
- internal drafting traces
- local scratch artifacts
- screenshots that reveal unrelated workspace or private discussions
