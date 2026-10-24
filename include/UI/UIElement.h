#ifndef GIRIGARA_UI_UI_ELEMENT_H
#define GIRIGARA_UI_UI_ELEMENT_H

#include <Shader/Shader.h>
#include <Script/LuaRef.h>
#include <UI/UIRegistry.h>
#include <string>
#include <cstdint>

class UIElement {
public:
    virtual ~UIElement() {
        // 物件銷毀時自動脫離全域註冊表
        UIRegistry::getInstance().unregisterElement(m_uuid, m_name);
    }

    UIElement(const UIElement&) = delete;
    UIElement& operator=(const UIElement&) = delete;
    UIElement(UIElement&&) noexcept = default;
    UIElement& operator=(UIElement&&) noexcept = default;

    // UUID 與 Name 存取介面
    uint64_t getUUID() const { return m_uuid; }
    const std::string& getName() const { return m_name; }

    void setName(const std::string& name) {
        // 先解除舊名字，再綁定新名字
        UIRegistry::getInstance().unregisterElement(0, m_name);
        m_name = name;
        UIRegistry::getInstance().registerElement(m_uuid, m_name, this);
    }

    virtual void render(const Shader& shader) = 0;
    virtual bool handleMouseMove(double x, double y) = 0;
    virtual bool handleMouseButton(int button, int action, int mods, double x, double y) = 0;

    virtual bool handleKey(int key, int scancode, int action, int mods) {
        (void)key; (void)scancode; (void)action; (void)mods;
        return false;
    }

    virtual void update(float dt);
    void setScriptUpdate(LuaRef callback);

    void setVisible(bool visible) {
        m_visible = visible;
        if (!visible) m_isFocused = false;
    }
    bool isVisible() const { return m_visible; }

    void setPosition(float x, float y) { m_x = x; m_y = y; }
    void setSize(float width, float height) { m_width = width; m_height = height; }

    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }

    virtual void setFocused(bool focused) { m_isFocused = focused; }
    bool isFocused() const { return m_isFocused; }

    virtual bool contains(double x, double y) const {
        return (x >= m_x && x <= m_x + m_width &&
        y >= m_y && y <= m_y + m_height);
    }

protected:
    UIElement()
    : m_uuid(UIRegistry::getInstance().generateUUID()) {
        UIRegistry::getInstance().registerElement(m_uuid, "", this);
    }

    UIElement(float x, float y, float width, float height, std::string name = "")
    : m_x(x), m_y(y), m_width(width), m_height(height),
    m_uuid(UIRegistry::getInstance().generateUUID()),
    m_name(std::move(name)) {
        UIRegistry::getInstance().registerElement(m_uuid, m_name, this);
    }

    virtual void onUpdate(float dt);

    uint64_t m_uuid = 0;
    std::string m_name;

    float m_x = 0.0f;
    float m_y = 0.0f;
    float m_width = 0.0f;
    float m_height = 0.0f;
    bool m_visible = true;
    bool m_isFocused = false;

    LuaRef m_scriptUpdate;
};

#endif // GIRIGARA_UI_UI_ELEMENT_H
