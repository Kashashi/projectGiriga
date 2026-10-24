#include <Script/ScriptBindings.h>
#include <Script/LuaRef.h>
#include <UI/UIElement.h>
#include <UI/UIRegistry.h>
#include <UI/UIWindow.h>
#include <iostream>

extern "C" {
    #include <lauxlib.h>
}

namespace {
    // 1. 綁定 Update 回調: BindUIUpdate(elementPtr, function(self, dt) ... end)
    int lua_BindUIUpdate(lua_State* L) {
        auto* element = static_cast<UIElement*>(lua_touserdata(L, 1));
        if (!element || !lua_isfunction(L, 2)) {
            return luaL_error(L, "Invalid arguments to BindUIUpdate: expected (element, function)");
        }

        lua_pushvalue(L, 2);
        element->setScriptUpdate(LuaRef::fromStack(L));
        return 0;
    }

    // 2. 設置元件位置: UI_SetPosition(elementPtr, x, y)
    int lua_UI_SetPosition(lua_State* L) {
        auto* element = static_cast<UIElement*>(lua_touserdata(L, 1));
        float x = static_cast<float>(luaL_checknumber(L, 2));
        float y = static_cast<float>(luaL_checknumber(L, 3));

        if (element) {
            element->setPosition(x, y);
        }
        return 0;
    }

    // 3. 輸出引擎日誌: EngineLog("訊息")
    int lua_EngineLog(lua_State* L) {
        const char* msg = luaL_checkstring(L, 1);
        std::cout << "[Lua] " << (msg ? msg : "") << "\n";
        return 0;
    }

    // 4. 依名字尋找 UI: FindUI("GlobalConsole")
    int lua_FindUI(lua_State* L) {
        const char* name = luaL_checkstring(L, 1);
        UIElement* elem = UIRegistry::getInstance().findByName(name);
        if (elem) {
            lua_pushlightuserdata(L, elem);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // 5. 依 UUID 尋找 UI: GetUIByUUID(uuid)
    int lua_GetUIByUUID(lua_State* L) {
        uint64_t uuid = static_cast<uint64_t>(luaL_checkinteger(L, 1));
        UIElement* elem = UIRegistry::getInstance().findByUUID(uuid);
        if (elem) {
            lua_pushlightuserdata(L, elem);
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // 6. 取得 UUID: UI_GetUUID(elementPtr)
    int lua_UI_GetUUID(lua_State* L) {
        auto* elem = static_cast<UIElement*>(lua_touserdata(L, 1));
        if (elem) {
            lua_pushinteger(L, static_cast<lua_Integer>(elem->getUUID()));
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // 7. 取得名稱: UI_GetName(elementPtr)
    int lua_UI_GetName(lua_State* L) {
        auto* elem = static_cast<UIElement*>(lua_touserdata(L, 1));
        if (elem) {
            lua_pushstring(L, elem->getName().c_str());
        } else {
            lua_pushnil(L);
        }
        return 1;
    }

    // 8. 設置可見性: UI_SetVisible(elementPtr, visible)
    int lua_UI_SetVisible(lua_State* L) {
        auto* elem = static_cast<UIElement*>(lua_touserdata(L, 1));
        bool visible = lua_toboolean(L, 2) != 0;
        if (elem) {
            elem->setVisible(visible);
        }
        return 0;
    }
    // 1. 取得 X 座標: local x = UI_GetX(elem)
    static int lua_UI_GetX(lua_State* L) {
        auto* elem = static_cast<UIElement*>(lua_touserdata(L, 1));
        if (elem) {
            lua_pushnumber(L, static_cast<lua_Number>(elem->getX()));
        } else {
            lua_pushnumber(L, 0.0);
        }
        return 1;
    }

    // 2. 取得 Y 座標: local y = UI_GetY(elem)
    static int lua_UI_GetY(lua_State* L) {
        auto* elem = static_cast<UIElement*>(lua_touserdata(L, 1));
        if (elem) {
            lua_pushnumber(L, static_cast<lua_Number>(elem->getY()));
        } else {
            lua_pushnumber(L, 0.0);
        }
        return 1;
    }
    static int lua_UI_IsDragging(lua_State* L) {
        auto* win = dynamic_cast<UIWindow*>(static_cast<UIElement*>(lua_touserdata(L, 1)));
        if (win) {
            lua_pushboolean(L, win->isDragging() ? 1 : 0);
        } else {
            lua_pushboolean(L, 0);
        }
        return 1;
    }
}

namespace ScriptBindings {
    void registerAll(lua_State* L) {
        if (!L) return;

        // 核心更新與輸出 API
        lua_register(L, "BindUIUpdate", lua_BindUIUpdate);
        lua_register(L, "UI_SetPosition", lua_UI_SetPosition);
        lua_register(L, "EngineLog", lua_EngineLog);

        // 物件定位與屬性 API
        lua_register(L, "FindUI", lua_FindUI);
        lua_register(L, "GetUIByUUID", lua_GetUIByUUID);
        lua_register(L, "UI_GetUUID", lua_UI_GetUUID);
        lua_register(L, "UI_GetName", lua_UI_GetName);
        lua_register(L, "UI_SetVisible", lua_UI_SetVisible);
        lua_register(L, "UI_GetX", lua_UI_GetX);
        lua_register(L, "UI_GetY", lua_UI_GetY);
        lua_register(L, "UI_IsDragging", lua_UI_IsDragging);
    }
}
