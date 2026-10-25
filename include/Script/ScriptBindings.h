#ifndef GIRIGARA_SCRIPT_SCRIPT_BINDINGS_H
#define GIRIGARA_SCRIPT_SCRIPT_BINDINGS_H

extern "C" {
    #include <lua.h>
}

class LoggerSystem;

namespace ScriptBindings {
    // 註冊所有引擎與 UI 相關的 C API 至 Lua 虛擬機
    void registerAll(lua_State* L, LoggerSystem& logger);
}

#endif // GIRIGARA_SCRIPT_SCRIPT_BINDINGS_H
