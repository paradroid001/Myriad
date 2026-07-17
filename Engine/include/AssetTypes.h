#ifndef _MYRIAD_ASSETTYPES_H_
#define _MYRIAD_ASSETTYPES_H_

#include <string>

#include "EngineConfig.h" //MYR_API
#include "InterfaceTypes.h"

namespace Myriad
{
    class Asset
    {
      protected:
        AssetID_t id_;
        std::string path_;
        void *data_ptr_; // This is a pointer to the actual data of the asset,
                         // which will be used by the renderer or other systems.

      public:
        Asset(AssetID_t id, const std::string path, void *data_ptr = nullptr)
            : id_(id), path_(path), data_ptr_(data_ptr) {};

        virtual ~Asset() {}

        // For comparing asset equality
        bool operator==(const Asset &other) const
        {
            return id_ == other.id_ && path_ == other.path_ &&
                   data_ptr_ == other.data_ptr_;
        }

        virtual AssetID_t GetID() { return id_; };
        virtual std::string GetPath() { return path_; };
        virtual void *GetDataPtr() { return data_ptr_; };
    };

    /* TODO */
    // Font and Texture2D - and other types,
    // should be exposed so editor can use them, and so
    // they can be serialised in meta information for assets.
    // Asset manager can return asset ids, thats fine.
    // But it should also be able to return Font or Texture2D.
    // Then you can programmatically do things, like calculate
    // frames etc based on the width and height of the texture2d,
    // and bake that in, plus all the rects etc.
    class Font : public Asset
    {
      public:
        AssetID_t font_id; // todo this could be a resource id?

        // Default constructor so arrays can be declared
        Font() : Asset(MYRIAD_INVALID_ID, "", nullptr) {}
        // Now some metrics.
        Font(AssetID_t id, const std::string path, void *data_ptr = nullptr)
            : Asset(id, path, data_ptr) {};
    };

    class Texture2D : public Asset
    {
      public:
        AssetID_t texture_id;
        Vector2i texture_dimensions;
        uint8_t colour_depth;
        uint32_t pixel_format; // TODO
        // Default constructor so arrays can be declared
        Texture2D() : Asset(MYRIAD_INVALID_ID, "", nullptr) {};
        Texture2D(AssetID_t id, const std::string path,
                  void *data_ptr = nullptr)
            : Asset(id, path, data_ptr) {};
    };

    class AssetProvider; // fwd declarre

    class MYR_API AssetManager
    {
      public:
        virtual ~AssetManager() = 0;
        virtual AssetID_t GenerateID() = 0;
        // These are not the correct fn signatures.
        // We actually want the ones below, but lets
        // just start with this, and once we have
        // allocators and the ability to return
        // ids, we can change the interface to be more correct.

        virtual Texture2D GetTexture(std::string path) = 0;
        virtual Asset *GetFont(std::string path) = 0;

        virtual std::shared_ptr<AssetProvider> GetTextureProvider() = 0;
        virtual std::shared_ptr<AssetProvider> GetFontProvider() = 0;

        // virtual TexHandle_T GetTexture(std::string path) = 0;
        // virtual void *GetTexturePointer(MYR_ID_t handle) = 0;
        // virtual void ReleaseTexture(TexHandle_T) = 0;
        // virtual FontHandle_T GetFont(std::string path) = 0;
        // virtual void *GetFontPointer(MYR_ID_t handle) = 0;
        // virtual void ReleaseFont(FontHandle_T) = 0;
    };
    inline AssetManager::~AssetManager() = default; // define ~AssetManager

    // Not sure I even need this publicly
    class AssetProvider : public IProvider
    {
      protected:
        AssetManager *asset_manager_;

      public:
        AssetProvider(AssetManager *asset_manager)
            : IProvider(), asset_manager_(asset_manager) {};
        virtual ~AssetProvider() = 0;
        bool Init() = 0;
        bool Shutdown() = 0;
        virtual AssetID_t Load(const std::string path) = 0;
        virtual bool Unload(AssetID_t id) = 0;
    };
    inline AssetProvider::~AssetProvider() = default; // define ~AssetProvider

} // namespace Myriad

#endif
