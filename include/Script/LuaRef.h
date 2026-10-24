#ifndef GIRIGARA_SCRIPT_LUA_REF_H
#define GIRIGARA_SCRIPT_LUA_REF_H

extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
}

class LuaRef {
public:
    LuaRef() = default;
    LuaRef(lua_State* L, int refIndex) : m_L(L), m_ref(refIndex) {}
    ~LuaRef() { reset(); }

    LuaRef(const LuaRef&) = delete;
    LuaRef& operator=(const LuaRef&) = delete;

    LuaRef(LuaRef&& other) noexcept : m_L(other.m_L), m_ref(other.m_ref) {
        other.m_L = nullptr;
        other.m_ref = LUA_NOREF;
    }

    LuaRef& operator=(LuaRef&& other) noexcept {
        if (this != &other) {
            reset();
            m_L = other.m_L;
            m_ref = other.m_ref;
            other.m_L = nullptr;
            other.m_ref = LUA_NOREF;
        }
        return *this;
    }

    bool isValid() const { return m_L != nullptr && m_ref != LUA_NOREF && m_ref != LUA_REFNIL; }
    int getRef() const { return m_ref; }
    lua_State* getState() const { return m_L; }

    // 將此引用壓入棧頂
    void push() const {
        if (isValid()) {
            lua_rawgeti(m_L, LUA_REGISTRYINDEX, m_ref);
        } else {
            lua_pushnil(m_L);
        }
    }

    void reset() {
        if (isValid()) {
            luaL_unref(m_L, LUA_REGISTRYINDEX, m_ref);
            m_ref = LUA_NOREF;
            m_L = nullptr;
        }
    }

    // 輔助函式：從目前棧頂擷取值建立 Ref 並彈出
    static LuaRef fromStack(lua_State* L) {
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        return LuaRef(L, ref);
    }

private:
    lua_State* m_L = nullptr;
    int m_ref = LUA_NOREF;
};

#endif // GIRIGARA_SCRIPT_LUA_REF_H
