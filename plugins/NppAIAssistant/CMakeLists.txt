cmake_minimum_required(VERSION 3.20)

project(NppAIAssistant LANGUAGES CXX RC)

add_library(NppAIAssistant SHARED
  src/NppAIAssistant.cpp
  src/NppAIAssistantResources.rc
  ../../PowerEditor/src/MISC/Common/HttpClient.cpp
  ../../PowerEditor/src/MISC/Common/LLMApiClient.cpp
  ../../PowerEditor/src/MISC/Common/SecureStorage.cpp
)

target_compile_features(NppAIAssistant PRIVATE cxx_std_20)

target_compile_definitions(NppAIAssistant PRIVATE
  UNICODE
  _UNICODE
  WIN32_LEAN_AND_MEAN
  NOMINMAX
)

target_include_directories(NppAIAssistant PRIVATE
  src
  ../../PowerEditor/src/MISC/Common
  ../../PowerEditor/src/MISC/PluginsManager
  ../../PowerEditor/src/WinControls/DockingWnd
  ../../scintilla/include
)

target_link_libraries(NppAIAssistant PRIVATE
  comctl32
  crypt32
  shell32
  shlwapi
  winhttp
)

set_target_properties(NppAIAssistant PROPERTIES
  OUTPUT_NAME "NppAIAssistant"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/NppAIAssistant"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/NppAIAssistant"
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins/NppAIAssistant"
)
