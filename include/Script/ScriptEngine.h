#ifndef GIRIGARA_SCRIPT_SCRIPT_ENGINE_H
#define GIRIGARA_SCRIPT_SCRIPT_ENGINE_H

#include <string>
#include <iostream>

#include <Resource/ScriptResource.h>

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    ScriptEngine(const ScriptEngine&) = delete;
    ScriptEngine& operator=(const ScriptEngine&) = delete;
    ScriptEngine(ScriptEngine&& other) noexcept;
    ScriptEngine& operator=(ScriptEngine&& other) noexcept;

    // 載入並執行 Lua 檔案
    bool executeFile(const std::string& filepath);

    // 執行一段 Lua 字串
    bool executeString(const std::string& code);

    // 呼叫無參數的 Lua 全域函式
    bool callFunction(const std::string& funcName);
    bool executeResource(const ScriptResource& script);
    lua_State* getRawState() const { return m_L; }

private:
    lua_State* m_L = nullptr;

    void release();
    void reportErrors(int status);
};

#endif // GIRIGARA_SCRIPT_SCRIPT_ENGINE_H
