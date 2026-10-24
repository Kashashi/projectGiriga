#include <Script/ScriptEngine.h>
#include <utility>

ScriptEngine::ScriptEngine() {
    m_L = luaL_newstate();
    if (m_L) {
        luaL_openlibs(m_L); // 開啟標準函式庫 (base, string, table, math 等)
    } else {
        std::cerr << "[ScriptEngine] Failed to initialize Lua state.\n";
    }
}

ScriptEngine::~ScriptEngine() {
    release();
}

ScriptEngine::ScriptEngine(ScriptEngine&& other) noexcept : m_L(other.m_L) {
    other.m_L = nullptr;
}

ScriptEngine& ScriptEngine::operator=(ScriptEngine&& other) noexcept {
    if (this != &other) {
        release();
        m_L = other.m_L;
        other.m_L = nullptr;
    }
    return *this;
}

void ScriptEngine::release() {
    if (m_L) {
        lua_close(m_L);
        m_L = nullptr;
    }
}

void ScriptEngine::reportErrors(int status) {
    if (status != LUA_OK && m_L) {
        const char* msg = lua_tostring(m_L, -1);
        std::cerr << "[Lua Error] " << (msg ? msg : "Unknown error") << "\n";
        lua_pop(m_L, 1);
    }
}

bool ScriptEngine::executeFile(const std::string& filepath) {
    if (!m_L) return false;
    int status = luaL_dofile(m_L, filepath.c_str());
    if (status != LUA_OK) {
        reportErrors(status);
        return false;
    }
    return true;
}

bool ScriptEngine::executeString(const std::string& code) {
    if (!m_L) return false;
    int status = luaL_dostring(m_L, code.c_str());
    if (status != LUA_OK) {
        reportErrors(status);
        return false;
    }
    return true;
}

bool ScriptEngine::callFunction(const std::string& funcName) {
    if (!m_L) return false;
    lua_getglobal(m_L, funcName.c_str());
    if (!lua_isfunction(m_L, -1)) {
        lua_pop(m_L, 1);
        return false;
    }

    int status = lua_pcall(m_L, 0, 0, 0);
    if (status != LUA_OK) {
        reportErrors(status);
        return false;
    }
    return true;
}

bool ScriptEngine::executeResource(const ScriptResource& script) {
    // 使用 luaL_loadbuffer 載入腳本源碼，以腳本檔案路徑作為 chunk 名稱（方便出錯時噴出正確行號）
    int status = luaL_loadbuffer(m_L, script.getCode().c_str(), script.getCode().size(), script.getPath().c_str());
    if (status != LUA_OK) {
        reportErrors(status);
        return false;
    }

    status = lua_pcall(m_L, 0, 0, 0);
    if (status != LUA_OK) {
        reportErrors(status);
        return false;
    }
    return true;
}
