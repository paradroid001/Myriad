#include "editor_app.h"

#include "project_paths.h"

#include "MyrEntrypoint.h"

#include <cstdio>
#include <filesystem>
#include <string>

Myriad::MyrApplication *Myriad::CreateApplication()
{
  const std::filesystem::path project_root = Editor::FindProjectRoot();
  std::fprintf(stderr, "[Editor] CreateApplication working directory: %s\n", std::filesystem::current_path().string().c_str());
  std::fprintf(stderr, "[Editor] CreateApplication detected project root: %s\n", project_root.empty() ? "<none>" : project_root.string().c_str());
  for (const auto &line : Editor::GetEditorSettingsDiscoveryLog(project_root))
  {
    std::fprintf(stderr, "[Editor] CreateApplication %s\n", line.c_str());
  }
  std::fflush(stderr);
  return new MyriadEditor();
}
