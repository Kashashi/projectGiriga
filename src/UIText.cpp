#include <UI/UIText.h>

namespace {
    constexpr GLint LOC_MODEL    = 1;
    constexpr GLint LOC_UI_COLOR = 2;
    constexpr GLint LOC_IS_TEXT  = 3;
}

UIText::UIText(float x, float y, const FontLoader& font, const std::u32string& text, unsigned int fontSize)
: UIElement(x, y, 0.0f, 0.0f), m_font(font), m_text(text), m_targetFontSize(fontSize) {
    rebake();
}

void UIText::setText(const std::u32string& text) {
    if (m_text != text) {
        m_text = text;
        rebake();
    }
}

void UIText::setFontSize(unsigned int fontSize) {
    if (m_targetFontSize != fontSize) {
        m_targetFontSize = fontSize;
        updateMetrics();
    }
}

void UIText::setColor(float r, float g, float b, float a) {
    m_color[0] = r;
    m_color[1] = g;
    m_color[2] = b;
    m_color[3] = a;
}

void UIText::updateMetrics() {
    if (!m_texture) {
        m_width = 0.0f;
        m_height = 0.0f;
        m_scale = 1.0f;
        return;
    }

    // 取得 FontLoader 的原生烘焙基準字號（例如 48）
    unsigned int baseSize = m_font.getFontSize();
    if (m_targetFontSize > 0 && baseSize > 0) {
        m_scale = static_cast<float>(m_targetFontSize) / static_cast<float>(baseSize);
    } else {
        m_scale = 1.0f;
    }

    // 讓 UIElement 的 AABB 包圍盒尺寸同步乘上縮放比
    m_width  = static_cast<float>(m_texture->getWidth()) * m_scale;
    m_height = static_cast<float>(m_texture->getHeight()) * m_scale;
}

void UIText::rebake() {
    if (m_text.empty()) {
        m_texture.reset();
        m_width = 0.0f;
        m_height = 0.0f;
        return;
    }

    // 重新烘焙成單通道紋理
    m_texture = std::make_unique<TextTexture>(m_font, m_text);
    updateMetrics();
}

void UIText::render(const Shader& shader) {
    if (!m_visible || !m_texture || m_width <= 0.0f || m_height <= 0.0f) return;

    glProgramUniform1i(shader.getID(), LOC_IS_TEXT, 1);
    glProgramUniform4fv(shader.getID(), LOC_UI_COLOR, 1, m_color);

    // 套用包含縮放後的 Model 矩陣
    float model[16] = {
        m_width, 0.0f,     0.0f, 0.0f,
        0.0f,    m_height, 0.0f, 0.0f,
        0.0f,    0.0f,     1.0f, 0.0f,
        m_x,     m_y,      0.0f, 1.0f
    };
    glProgramUniformMatrix4fv(shader.getID(), LOC_MODEL, 1, GL_FALSE, model);

    m_texture->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool UIText::handleMouseMove(double /*x*/, double /*y*/) {
    return false; // 不攔截滑鼠懸停，讓事件穿透到下方的按鈕
}

bool UIText::handleMouseButton(int /*button*/, int /*action*/, int /*mods*/, double /*x*/, double /*y*/) {
    return false; // 不攔截滑鼠點擊，讓下方的 UIButton 接收事件
}
