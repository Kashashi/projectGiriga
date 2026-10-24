#include <UI/UIWindow.h>
#include <GLFW/glfw3.h>

// 方式 1：字串構造
UIWindow::UIWindow(float x, float y, float width, float height,
                   const FontLoader& font, const std::u32string& title,
                   unsigned int titleFontSize)
: UIElement(x, y, width, height) {
    m_titleText = std::make_unique<UIText>(x + 10.0f, y, font, title, titleFontSize);
    initCloseButton();
    updateChildrenPositions();
}

// 方式 2：直接移入構造好的 UIText
UIWindow::UIWindow(float x, float y, float width, float height,
                   std::unique_ptr<UIText> titleText)
: UIElement(x, y, width, height), m_titleText(std::move(titleText)) {
    initCloseButton();
    updateChildrenPositions();
}

void UIWindow::initCloseButton() {
    float btnSize = 20.0f;
    float btnMargin = (m_titleBarHeight - btnSize) * 0.5f;
    m_closeButton = std::make_unique<UIButton>(
        m_x + m_width - btnSize - btnMargin,
        m_y + btnMargin,
        btnSize, btnSize
    );
    m_closeButton->setColors({
        {0.8f, 0.2f, 0.2f, 1.0f},
        {1.0f, 0.3f, 0.3f, 1.0f},
        {0.6f, 0.1f, 0.1f, 1.0f}
    });
    m_closeButton->setOnClick([this]() {
        setVisible(false);
    });
}

void UIWindow::setTitle(std::unique_ptr<UIText> titleText) {
    m_titleText = std::move(titleText);
    updateChildrenPositions();
}

void UIWindow::setPosition(float x, float y) {
    m_x = x;
    m_y = y;
    updateChildrenPositions();
}

void UIWindow::updateChildrenPositions() {
    if (m_titleText) {
        float textY = m_y + (m_titleBarHeight - m_titleText->getHeight()) * 0.5f;
        m_titleText->setPosition(m_x + 10.0f, textY);
    }

    if (m_closeButton) {
        float btnSize = m_closeButton->getWidth();
        float btnMargin = (m_titleBarHeight - btnSize) * 0.5f;
        m_closeButton->setPosition(m_x + m_width - btnSize - btnMargin, m_y + btnMargin);
    }

    for (const auto& child : m_contentElements) {
        if (child.element) {
            child.element->setPosition(m_x + child.relX, m_y + child.relY);
        }
    }
}

bool UIWindow::isInsideTitleBar(double x, double y) const {
    return (x >= m_x && x <= m_x + m_width &&
    y >= m_y && y <= m_y + m_titleBarHeight);
}

// -------------------------------------------------------------
// 實作 UIElement 的虛擬介面
// -------------------------------------------------------------

void UIWindow::render(const Shader& shader) {
    if (!m_visible) return;

    constexpr GLint LOC_MODEL    = 1;
    constexpr GLint LOC_UI_COLOR = 2;
    constexpr GLint LOC_IS_TEXT  = 3;

    glProgramUniform1i(shader.getID(), LOC_IS_TEXT, 0);

    // 1. 繪製視窗本體 (Body)
    glProgramUniform4fv(shader.getID(), LOC_UI_COLOR, 1, m_bodyColor);
    float bodyModel[16] = {
        m_width, 0.0f,     0.0f, 0.0f,
        0.0f,    m_height, 0.0f, 0.0f,
        0.0f,    0.0f,     1.0f, 0.0f,
        m_x,     m_y,      0.0f, 1.0f
    };
    glProgramUniformMatrix4fv(shader.getID(), LOC_MODEL, 1, GL_FALSE, bodyModel);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 2. 繪製標題列 (Title Bar)
    glProgramUniform4fv(shader.getID(), LOC_UI_COLOR, 1, m_titleBarColor);
    float titleModel[16] = {
        m_width, 0.0f,             0.0f, 0.0f,
        0.0f,    m_titleBarHeight, 0.0f, 0.0f,
        0.0f,    0.0f,             1.0f, 0.0f,
        m_x,     m_y,              0.0f, 1.0f
    };
    glProgramUniformMatrix4fv(shader.getID(), LOC_MODEL, 1, GL_FALSE, titleModel);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 3. 繪製標題文字與關閉按鈕
    if (m_titleText) {
        m_titleText->render(shader);
    }
    if (m_closeButton) {
        m_closeButton->render(shader);
    }

    // 4. 繪製內部內容容器
    m_container.render(shader);
}

bool UIWindow::handleMouseMove(double x, double y) {
    if (!m_visible) return false;

    // 拖曳中
    if (m_isDragging) {
        setPosition(static_cast<float>(x - m_dragOffsetX),
                    static_cast<float>(y - m_dragOffsetY));
        return true;
    }

    // 優先詢問關閉按鈕
    if (m_closeButton && m_closeButton->handleMouseMove(x, y)) {
        return true;
    }

    // 詢問內部元件
    if (m_container.handleMouseMove(x, y)) {
        return true;
    }

    return contains(x, y);
}

bool UIWindow::handleMouseButton(int button, int action, int mods, double x, double y) {
    if (!m_visible) return false;

    // 1. 關閉按鈕優先判定
    if (m_closeButton && m_closeButton->handleMouseButton(button, action, mods, x, y)) {
        return true;
    }

    // 2. 處理滑鼠按下
    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
        // 點在標題列 -> 開始拖曳
        if (isInsideTitleBar(x, y)) {
            m_isDragging = true;
            m_dragOffsetX = x - m_x;
            m_dragOffsetY = y - m_y;
            return true;
        }

        // 點在內部內容
        if (m_container.handleMouseButton(button, action, mods, x, y)) {
            return true;
        }

        // 點在視窗本體空白處
        if (contains(x, y)) {
            return true;
        }
    }
    // 3. 處理滑鼠放開
    else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
        if (m_isDragging) {
            m_isDragging = false;
            return true;
        }
        if (m_container.handleMouseButton(button, action, mods, x, y)) {
            return true;
        }
    }

    return false;
}

bool UIWindow::handleKey(int key, int /*scancode*/, int action, int /*mods*/) {
    if (!m_visible) return false;

    // 聚焦、可見、且按下 ESC 時觸發關閉
    if (m_isFocused && isCloseOnEscEnabled() && key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        setVisible(false);
        return true;
    }

    return m_container.handleKey(key, 0, action, 0);
}

