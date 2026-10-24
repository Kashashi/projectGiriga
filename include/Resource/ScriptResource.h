#ifndef GIRIGARA_RESOURCE_SCRIPT_RESOURCE_H
#define GIRIGARA_RESOURCE_SCRIPT_RESOURCE_H

#include <string>

class ScriptResource {
public:
    ScriptResource(std::string resolvedPath, std::string sourceCode)
    : m_path(std::move(resolvedPath)), m_code(std::move(sourceCode)) {}

    const std::string& getPath() const { return m_path; }
    const std::string& getCode() const { return m_code; }

    // 支援重新載入（供熱重載使用）
    bool reload();

private:
    std::string m_path;
    std::string m_code;
};

#endif // GIRIGARA_RESOURCE_SCRIPT_RESOURCE_H
