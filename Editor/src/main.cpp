// dear imgui: "null" example application
// (compile and link imgui, create context, run headless with NO INPUTS, NO GRAPHICS OUTPUT)
// This is useful to test building, but you cannot interact with anything here!
#include "imgui.h"
#include <stdio.h>

#include "rlImGui.h" // include the API header
#include "myriad_ext.h"

using namespace Myriad;

class MyriadEditor : public MyrGameApplication
{
public:
  virtual ~MyriadEditor() { printf("Destroying MyriadEditor.\n"); }

  Myriad::EngineConfig_t Setup() override
  {
    Screen.width = 1024.0f;
    Screen.height = 768.0f;
    Myriad::EngineConfig_t config;
    config.window_title = "Test";
    config.screen_dimensions = {Screen.width, Screen.height};
    config.threads_enabled = false;
    config.num_threads = 0;
    config.fps = 60;
    config.asset_root_path = "res";
    config.max_component_slots = 100000;
    config.max_gameobject_slots = 1000;
    return config;
  }

  virtual void Start() override
  {
    rlImGuiSetup(true); // sets up ImGui with ether a dark or light default theme
  }

  void Render() override
  {
    Myriad::Renderer &r = engine_.GetRenderer();
    Myriad::TexHandle_T thandle = Assets().GetTexture("res/Druid.png");
    r.BeginDrawing();
    r.ClearBackground({0, 0, 0, 255});

    // start ImGui Conent
    rlImGuiBegin();

    // show ImGui Content
    bool open = true;
    ImGui::ShowDemoWindow(&open);

    open = true;
    if (ImGui::Begin("Test Window", &open))
    {
      ImGui::TextUnformatted(ICON_FA_JEDI);

      rlImGuiImage((const Texture *)Assets().GetTexturePointer(thandle)->GetTexPtr());
    }
    ImGui::End();

    // end ImGui Content
    rlImGuiEnd();
    r.EndDrawing();
  }

  void PreShutdown() override
  {
    rlImGuiShutdown();
  }
};

Myriad::MyrApplication *Myriad::CreateApplication()
{
  return new MyriadEditor();
}
