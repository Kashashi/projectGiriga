#include <UI/UIContainer.h>
#include <algorithm>

void UIContainer::addChild(std::unique_ptr<UIElement> child) {
    if (child) {
        child->setParent(this); // ★ 設置父節點
        m_children.push_back(std::move(child));
    }
}

void UIContainer::clear() {
    m_focusedElement = nullptr;
    m_children.clear();
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

void UIContainer::update(float dt) {
    UIElement::update(dt);
    if (!isVisible()) return;

    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->update(dt);
        }
    }
}

void UIContainer::render(const Shader& shader) {
    if (!isVisible()) return;

    // 正向渲染：先畫底層，再畫頂層
    for (auto& child : m_children) {
        if (child && child->isVisible()) {
            child->render(shader);
        }
    }
}

bool UIContainer::handleMouseMove(double x, double y) {
    if (!isVisible()) return false;

    // 1. 優先由子元件處理
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it) && (*it)->isVisible()) {
            if ((*it)->handleMouseMove(x, y)) {
                return true;
            }
        }
    }

    // 2. 若子元件沒處理：
    // 如果開啟了穿透模式，一律回傳 false 放行給底層場景
    if (m_passThrough) {
        return false;
    }

    // 若非穿透容器，則看滑鼠是否落在容器自身範圍內
    return contains(x, y);
}

bool UIContainer::handleMouseButton(int button, int action, int mods, double x, double y) {
    if (!isVisible()) return false;

    // 1. 優先由子元件處理 (例如點到 InGameConsole 裡的按鈕或文字框)
    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it) && (*it)->isVisible()) {
            if ((*it)->handleMouseButton(button, action, mods, x, y)) {
                setFocusedElement(it->get());
                return true; // 子元件消耗了事件
            }
        }
    }

    // 2. 若子元件均未處理：
    // 若為穿透容器 (如全域 UI 畫布)，絕不吃掉事件，讓點擊穿透到底下的場景
    if (m_passThrough) {
        return false;
    }

    // 若為非穿透容器 (如實體面板)，點在自己身上就攔截，不讓點擊穿透到身後的東西
    return contains(x, y);
}

bool UIContainer::handleKey(int key, int scancode, int action, int mods) {
    if (!isVisible()) return false;

    if (m_focusedElement && m_focusedElement->isVisible()) {
        if (m_focusedElement->handleKey(key, scancode, action, mods)) {
            return true;
        }
    }

    for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
        if ((*it) && (*it)->isVisible() && it->get() != m_focusedElement) {
            if ((*it)->handleKey(key, scancode, action, mods)) {
                return true;
            }
        }
    }

    // 鍵盤未被任何子節點處理，穿透模式下放行
    return !m_passThrough;
}


