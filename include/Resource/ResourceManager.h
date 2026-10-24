#ifndef GIRIGARA_RESOURCE_RESOURCE_MANAGER_H
#define GIRIGARA_RESOURCE_RESOURCE_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <Shader/Shader.h>
#include <FontLoader/TextTexture.h>
#include <Resource/ScriptResource.h>

class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() = default;

    // 禁止拷貝，支援移動
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) noexcept = default;
    ResourceManager& operator=(ResourceManager&&) noexcept = default;

    // 載入或取得快取的 Shader
    Shader& loadShader(const std::string& name,
                       const std::string& vertPath,
                       const std::string& fragPath);
    Shader& getShader(const std::string& name);

    // 載入或取得快取的字型（自動進行候選路徑搜尋與降級）
    FontLoader& loadFont(const std::string& name,
                         const std::vector<std::string>& candidatePaths,
                         unsigned int fontSize);
    FontLoader& getFont(const std::string& name);

    // 載入或取得快取的腳本資源（支援單一路徑或多個候選備用路徑）
    ScriptResource& loadScript(const std::string& name,
                               const std::vector<std::string>& candidatePaths);
    ScriptResource& loadScript(const std::string& name,
                               const std::string& path);
    ScriptResource& getScript(const std::string& name);

    // 熱重載指定名稱的腳本
    bool reloadScript(const std::string& name);

private:
    std::unordered_map<std::string, std::unique_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::unique_ptr<FontLoader>> m_fonts;
    std::unordered_map<std::string, std::unique_ptr<ScriptResource>> m_scripts;

    // 依序搜尋候選路徑，回傳第一個存在的檔案路徑
    std::string resolvePath(const std::vector<std::string>& candidates) const;
};

#endif // GIRIGARA_RESOURCE_RESOURCE_MANAGER_H
