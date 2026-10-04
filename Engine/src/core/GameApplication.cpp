#include "io/Logging.h" //for internal logging
#include "myriad.h"
#include "myriad_config.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace Myriad
{
    GameApplication::GameApplication()
        : Application(), engine_(*this, window_, renderer_)
    {
        MYR_CORE_TRACE("GameApplication created.");
    }
    GameApplication::~GameApplication()
    {
        MYR_CORE_TRACE("GameApplication destroyed.");
    }

    bool GameApplication::LoadConfig(const std::string &config_path,
                                     EngineConfig &config)
    {
        bool loaded = false;
        if (std::filesystem::exists(config_path))
        {
            std::ifstream config_file(config_path);
            std::stringstream config_stream;
            config_stream << config_file.rdbuf();

            s8::Serialiser s;
            s8::PlaintextCodec codec;
            // Create the serialiser state
            s8::CodecResult res = s.Deserialise(config_stream.str(), codec);
            if (!res.Ok())
            {
                MYR_ERROR("Failed to deserialise config from file using "
                          "s8::Serialiser: (%d): %s",
                          res.status, res.error_message.c_str());
            }
            else
            {
                if (!s.ReadObject(s.GetRootID(), config))
                {
                    const s8::SerialiserResult &result = s.GetLastResult();
                    MYR_ERROR("Failed to load config from file using "
                              "s8::Serialiser: %s",
                              result.error_message.c_str());
                }
                else
                {
                    loaded = true;
                }
            }
        }
        else
        {
            MYR_WARN("Config file not found");
        }

        return loaded;
    }

    bool GameApplication::SaveConfig(const std::string &config_path,
                                     const EngineConfig &config)
    {
        s8::Serialiser s;
        s8::PlaintextCodec codec;
        codec.SetOptions({.pretty_print = true});
        std::string config_out;
        s.WriteObject(s.GetRootID(), config);
        s8::CodecResult res = s.Serialise(codec, config_out);
        if (res.Ok())
        {
            std::ofstream config_file(config_path);
            config_file << config_out;
        }
        else
        {
            MYR_ERROR(
                "Failed to serialise config to file using s8::Serialiser: %s",
                res.error_message.c_str());
        }

        return res.Ok();
    }

    void GameApplication::Run()
    {
        MYR_CORE_TRACE("GameApplication run.");
        EngineConfig config;
        bool config_loaded_from_file = LoadConfig(DEFAULT_CONFIG_PATH, config);

        if (!config_loaded_from_file)
        {
            MYR_WARN("Failed to load config from file, using default config.");

            config = {
                .window_config = {.resolution = {800.0f, 600.0f},
                                  .fullscreen = false,
                                  .resizable = true,
                                  .vsync = false,
                                  .borderless = false,
                                  .title = "Myriad Engine"},
                .resource_base_path = DEFAULT_RESOURCE_BASE_PATH.c_str(),
                .target_framerate = 60,
            };
        }

        if (engine_.Init(config))
        {
            MYR_CORE_TRACE("GameEngine initialized successfully.");
            // If the config was not loaded from file but
            // the engine successfully initialized, save the config to file.
            if (!config_loaded_from_file)
            {
                if (!SaveConfig(DEFAULT_CONFIG_PATH, engine_.GetConfig()))
                {
                    MYR_ERROR("Failed to save config to file.");
                }
            }
            Start();
        }
        else
        {
            MYR_CORE_CRITICAL("GameEngine failed to initialize.");
        }

        while (engine_.IsRunning())
        {
            engine_.Frame();
        }

        if (engine_.GetState() == EngineState_t::SHUTDOWN)
        {
            engine_.Shutdown();
        }
    }

    void GameApplication::Start() { MYR_CORE_TRACE("GameApplication start."); }

    void GameApplication::PreUpdate()
    {
        // R_CORE_TRACE("GameApplication pre-update.");
    }
    void GameApplication::Update(float delta_ms)
    {
        // R_CORE_TRACE("GameApplication update.");
    }
    void GameApplication::PostUpdate()
    {
        // R_CORE_TRACE("GameApplication post-update.");
    }

    void GameApplication::PreRender()
    {
        // R_CORE_TRACE("GameApplication pre-render.");
    }
    void GameApplication::Render(float delta_ms)
    {
        // R_CORE_TRACE("GameApplication render.");
    }
    void GameApplication::PostRender()
    {
        // R_CORE_TRACE("GameApplication post-render.");
    }

    void GameApplication::PreShutdown()
    {
        MYR_CORE_TRACE("GameApplication pre-shutdown.");
    }
    void GameApplication::Shutdown()
    {
        MYR_CORE_TRACE("GameApplication shutdown.");
    }
    void GameApplication::PostShutdown()
    {
        MYR_CORE_TRACE("GameApplication post-shutdown.");
    }
} // namespace Myriad
