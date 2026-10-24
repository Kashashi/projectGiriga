#include <Resource/ResourceManager.h>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

std::string ResourceManager::resolvePath(const std::vector<std::string>& candidates) const {
    for (const auto& path : candidates) {
        if (fs::exists(path)) {
            return path;
        }
    }
    return "";
}

Shader& ResourceManager::loadShader(const std::string& name,
                                    const std::string& vertPath,
                                    const std::string& fragPath) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return *(it->second);
    }

    auto shader = std::make_unique<Shader>(vertPath, fragPath);
    auto& ref = *shader;
    m_shaders.emplace(name, std::move(shader));
    return ref;
    }

Shader& ResourceManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it == m_shaders.end()) {
        throw std::runtime_error("ResourceManager: Shader not found: " + name);
    }
        return *(it->second);
    }

FontLoader& ResourceManager::loadFont(const std::string& name,
                                            const std::vector<std::string>& candidatePaths,
                                            unsigned int fontSize) {
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
            return *(it->second);
    }

    std::string validPath = resolvePath(candidatePaths);
    if (validPath.empty()) {
        throw std::runtime_error("ResourceManager: No valid font file found for " + name);
    }

    auto font = std::make_unique<FontLoader>(validPath, fontSize);
    auto& ref = *font;
    m_fonts.emplace(name, std::move(font));
    return ref;
}

FontLoader& ResourceManager::getFont(const std::string& name) {
    auto it = m_fonts.find(name);
    if (it == m_fonts.end()) {
        throw std::runtime_error("ResourceManager: Font not found: " + name);
    }
    return *(it->second);
}

ScriptResource& ResourceManager::loadScript(const std::string& name,
                                            const std::vector<std::string>& candidatePaths) {
    auto it = m_scripts.find(name);
    if (it != m_scripts.end()) {
        return *(it->second);
    }

    std::string validPath = resolvePath(candidatePaths);
    if (validPath.empty()) {
        throw std::runtime_error("[ResourceManager] Script candidate paths not found for: " + name);
    }

    std::ifstream file(validPath);
    if (!file.is_open()) {
        throw std::runtime_error("[ResourceManager] Cannot open script file: " + validPath);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    auto script = std::make_unique<ScriptResource>(validPath, buffer.str());
    ScriptResource& ref = *script;
    m_scripts[name] = std::move(script);

    std::cout << "[ResourceManager] Loaded script [" << name << "] from: " << validPath << "\n";
    return ref;
}

ScriptResource& ResourceManager::loadScript(const std::string& name,
                                            const std::string& path) {
    return loadScript(name, std::vector<std::string>{path});
}

ScriptResource& ResourceManager::getScript(const std::string& name) {
    auto it = m_scripts.find(name);
    if (it == m_scripts.end()) {
        throw std::runtime_error("[ResourceManager] Script not found: " + name);
    }
    return *(it->second);
}

bool ResourceManager::reloadScript(const std::string& name) {
    auto it = m_scripts.find(name);
    if (it == m_scripts.end()) {
        std::cerr << "[ResourceManager] Cannot reload non-existent script: " << name << "\n";
        return false;
    }
    return it->second->reload();
}
