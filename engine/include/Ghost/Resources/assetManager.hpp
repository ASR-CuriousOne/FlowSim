#pragma once
#include <Ghost/Core/vulkanDevice.hpp>
#include <Ghost/Resources/ghostTexture.hpp>
#include <Ghost/Resources/mesh.hpp>
#include <Ghost/Resources/vertex.hpp>
#include <unordered_map>

namespace Ghost {
class AssetManager {
  public:
    AssetManager(VulkanDevice &device);
    ~AssetManager();

    AssetManager(const AssetManager &) = delete;
    AssetManager operator=(const AssetManager &) = delete;

    std::shared_ptr<Mesh> getMesh(const std::string &filepath);

    template <typename T, typename... Args>
    std::shared_ptr<T> getTexture(const std::string &filepath, Args &&...args) {
        static_assert(std::is_base_of_v<TextureBase, T>,
                      "T must inherit from TextureBase");

        auto it = m_textureCache.find(filepath);
        if (it != m_textureCache.end()) {
            auto castedTexture = std::dynamic_pointer_cast<T>(it->second);
            if (castedTexture) {
                return castedTexture;
            } else {
                throw std::runtime_error(
                    "Texture type mismatch for cached asset: " + filepath);
            }
        }

        std::shared_ptr<T> newTexture = std::make_shared<T>(
            m_device, filepath, std::forward<Args>(args)...);

        if (newTexture) {
            m_textureCache[filepath] = newTexture;
        }

        return newTexture;
    }

    void clear();

  private:
    VulkanDevice &m_device;

    std::shared_ptr<Mesh>
    createModelFromFile(const std::filesystem::path &filepath,
                        VertexType vertexType = VertexType::Standard);

    std::unordered_map<std::string, std::shared_ptr<Mesh>> m_models;
    std::unordered_map<std::string, std::shared_ptr<TextureBase>>
        m_textureCache;
};
} // namespace Ghost
