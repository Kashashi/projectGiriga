#include <UI/UIContainer.h>

void UIContainer::addChild(std::unique_ptr<UIElement> child) {
    if (child) {
        m_children.push_back(std::move(child));
    }
}

void UIContainer::clear() {
    m_children.clear();
    m_focusedElement = nullptr;
}

void UIContainer::setFocusedElement(UIElement* element) {
    if (m_focusedElement == element) return;
    if (m_focusedElement) {
        m_focusedElement->setFocused(false);
    }
    m_focusedElement = element;
    if (m_focusedElement) {
        m_focusedElement->setFocused(true);
    }
}

void UIContainer::render(const Shader& shader) {
    if (!m_visible) return;
    for (const auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render(shader);
        }
    }
}

bool UIContainer::handleMouseMove(double x, double y) {
    if (!m_visible) return false;

    bool consumed = false;
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (*it && (*it)->isVisible()) {
            if ((*it)->handleMouseMove(x, y)) {
                consumed = true;
            }
        }
    }
    return consumed;
}

bool UIContainer::handleMouseButton(int button, int action, int mods, double x, double y) {
    if (!m_visible) return false;

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (*it && (*it)->isVisible()) {
            if ((*it)->handleMouseButton(button, action, mods, x, y)) {
                // 點擊命中子元素，賦予焦點
                setFocusedElement(it->get());
                return true;
            }
        }
    }

    // 點在容器空白處，清除目前焦點
    setFocusedElement(nullptr);
    return false;
}

bool UIContainer::handleKey(int key, int scancode, int action, int mods) {
    if (!m_visible) return false;

    // 1. 優先派發給具備焦點的子元素
    if (m_focusedElement && m_focusedElement->isVisible()) {
        if (m_focusedElement->handleKey(key, scancode, action, mods)) {
            return true;
        }
    }

    // 2. 焦點元素未處理時，反向廣播給所有可見子元素
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if (*it && (*it)->isVisible() && it->get() != m_focusedElement) {
            if ((*it)->handleKey(key, scancode, action, mods)) {
                return true;
            }
        }
    }

    return false;
}

void UIContainer::update(float dt) {
    if (!m_visible) return;

    // 先執行自身可能掛載的腳本更新
    onUpdate(dt);

    // 遞迴調用所有子節點的 update
    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->update(dt);
        }
    }
}
