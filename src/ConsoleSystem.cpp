#include <System/ConsoleSystem.h>
#include <Script/ScriptEngine.h>

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

void ConsoleSystem::executeLua(const std::string& code) {
    lua_State* L = m_scriptEngine->getRawState();

    // 先嘗試當作 expression 執行 (例如輸入 "math.sin(1)" 相當於 "return math.sin(1)")
    std::string evalCode = "return " + code;
    if (luaL_dostring(L, evalCode.c_str()) == LUA_OK) {
        if (lua_gettop(L) > 0) {
            const char* res = lua_tostring(L, -1);
            if (res) {
                log(std::string("[Result] ") + res);
            }
            lua_pop(L, 1);
        }
        return;
    } else {
        // 若加上 return 語法錯誤，則將原句當作一般 statement 執行
        lua_pop(L, 1); // 彈出錯誤訊息
    }

    // 當作一般 statement 執行
    if (luaL_dostring(L, code.c_str()) != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        log(std::string("[Lua Error] ") + (err ? err : "Unknown error"));
        lua_pop(L, 1);
    }
}
