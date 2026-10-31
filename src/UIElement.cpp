#include <UI/UIElement.h>
#include <iostream>

UIElement::UIElement() : Core::Identifiable() {
    UIRegistry::getInstance().registerElement(m_uuid, "", this);
}

UIElement::UIElement(float x, float y, float width, float height, std::string name)
: Core::Identifiable(std::move(name)),
m_x(x), m_y(y), m_width(width), m_height(height) {
    UIRegistry::getInstance().registerElement(m_uuid, m_name, this);
}

UIElement::~UIElement() {
    // 物件銷毀時自動脫離全域註冊表
    UIRegistry::getInstance().unregisterElement(m_uuid, m_name);
}

void UIElement::setName(const std::string& name) {
    UIRegistry::getInstance().unregisterElement(0, m_name);
    Identifiable::setName(name);
    UIRegistry::getInstance().registerElement(m_uuid, m_name, this);
}

void UIElement::setVisible(bool visible) {
    m_visible = visible;
    if (!visible) {
        m_isFocused = false;
    }
}

void UIElement::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
    m_posUnit = UIUnit::Pixels;
}

void UIElement::setSize(float width, float height) {
    m_width = width;
    m_height = height;
    m_sizeUnit = UIUnit::Pixels;
}

void UIElement::setPositionPercent(float xPercent, float yPercent) {
    m_x = xPercent;
    m_y = yPercent;
    m_posUnit = UIUnit::Percent;
}

void UIElement::setSizePercent(float wPercent, float hPercent) {
    m_width = wPercent;
    m_height = hPercent;
    m_sizeUnit = UIUnit::Percent;
}

float UIElement::getLocalWidth() const {
    if (m_sizeUnit == UIUnit::Percent && m_parent) {
        return m_parent->getLocalWidth() * (m_width / 100.0f);
    }
    return m_width;
}

float UIElement::getLocalHeight() const {
    if (m_sizeUnit == UIUnit::Percent && m_parent) {
        return m_parent->getLocalHeight() * (m_height / 100.0f);
    }
    return m_height;
}

float UIElement::getLocalX() const {
    if (m_posUnit == UIUnit::Percent && m_parent) {
        return m_parent->getLocalWidth() * (m_x / 100.0f);
    }
    return m_x;
}

float UIElement::getLocalY() const {
    if (m_posUnit == UIUnit::Percent && m_parent) {
        return m_parent->getLocalHeight() * (m_y / 100.0f);
    }
    return m_y;
}

float UIElement::getGlobalX() const {
    return getLocalX() + (m_parent ? m_parent->getGlobalX() : 0.0f);
}

float UIElement::getGlobalY() const {
    return getLocalY() + (m_parent ? m_parent->getGlobalY() : 0.0f);
}

bool UIElement::contains(double x, double y) const {
    float gx = getGlobalX();
    float gy = getGlobalY();
    float gw = getLocalWidth();
    float gh = getLocalHeight();

    return (x >= gx && x <= gx + gw &&
    y >= gy && y <= gy + gh);
}

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
