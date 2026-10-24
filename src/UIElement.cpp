#include <UI/UIElement.h>
#include <iostream>

void UIElement::update(float dt) {
    onUpdate(dt);
}

void UIElement::setScriptUpdate(LuaRef callback) {
    m_scriptUpdate = std::move(callback);
}

void UIElement::onUpdate(float dt) {
    if (m_scriptUpdate.isValid()) {
        lua_State* L = m_scriptUpdate.getState();

        m_scriptUpdate.push();
        lua_pushlightuserdata(L, this);
        lua_pushnumber(L, static_cast<lua_Number>(dt));

        if (lua_pcall(L, 2, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(L, -1);
            std::cerr << "[Lua Script Error in Update] " << (err ? err : "Unknown") << "\n";
            lua_pop(L, 1);
        }
    }
}
