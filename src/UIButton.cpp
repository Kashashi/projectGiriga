#include <UI/UIButton.h>
#include <GLFW/glfw3.h>

namespace {
    constexpr GLint LOC_MODEL    = 1;
    constexpr GLint LOC_UI_COLOR = 2;
    constexpr GLint LOC_IS_TEXT  = 3;
}

UIButton::UIButton(float x, float y, float width, float height)
: UIElement(x, y, width, height) {}

void UIButton::setLabel(const FontLoader& font, const std::u32string& text,
                        float r, float g, float b, float a) {
    m_label = std::make_unique<UIText>(0.0f, 0.0f, font, text);
    m_label->setParent(this); // ★ 關鍵 1：標籤的 parent 設為按鈕本身
    m_label->setColor(r, g, b, a);
    updateLabelPosition();
                        }

                        void UIButton::updateLabelPosition() {
                            if (!m_label) return;
                            // ★ 關鍵 2：標籤在按鈕內部置中，直接計算相對 Local 座標，不要加 m_x, m_y！
                            float textX = (m_width - m_label->getWidth()) * 0.5f;
                            float textY = (m_height - m_label->getHeight()) * 0.5f;
                            m_label->setPosition(textX, textY);
                        }

                        void UIButton::render(const Shader& shader) {
                            if (!m_visible) return;

                            // 1. 繪製按鈕底色
                            glProgramUniform1i(shader.getID(), LOC_IS_TEXT, 0);

                            // ★ 關鍵 3：使用 getGlobalX() 與 getGlobalY()
                            float gw = getLocalWidth();
                            float gh = getLocalHeight();
                            float gx = getGlobalX();
                            float gy = getGlobalY();

                            float model[16] = {
                                gw,   0.0f, 0.0f, 0.0f,
                                0.0f, gh,   0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f,
                                gx,   gy,   0.0f, 1.0f
                            };
                            glProgramUniformMatrix4fv(shader.getID(), LOC_MODEL, 1, GL_FALSE, model);

                            const float* activeColor = m_colors.normal;
                            if (m_isPressed) {
                                activeColor = m_colors.pressed;
                            } else if (m_isHovered) {
                                activeColor = m_colors.hovered;
                            }
                            glProgramUniform4fv(shader.getID(), LOC_UI_COLOR, 1, activeColor);

                            glDrawArrays(GL_TRIANGLES, 0, 6);

                            // 2. 繪製按鈕標籤文字 (m_label 內部會自動加上 gx, gy)
                            if (m_label && m_label->isVisible()) {
                                m_label->render(shader);
                            }
                        }

                        bool UIButton::handleMouseMove(double x, double y) {
                            if (!m_visible) {
                                m_isHovered = false;
                                return false;
                            }

                            // UIElement::contains 內部已經改成使用 getGlobalX()/Y() 判定
                            m_isHovered = contains(x, y);
                            if (!m_isHovered) {
                                m_isPressed = false;
                            }

                            return m_isHovered;
                        }

                        bool UIButton::handleMouseButton(int button, int action, int /*mods*/, double x, double y) {
                            if (!m_visible) return false;

                            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                                if (action == GLFW_PRESS) {
                                    if (contains(x, y)) {
                                        m_isPressed = true;
                                        return true;
                                    }
                                } else if (action == GLFW_RELEASE) {
                                    if (m_isPressed && contains(x, y)) {
                                        m_isPressed = false;
                                        if (m_onClick) {
                                            m_onClick();
                                        }
                                        return true;
                                    }
                                    m_isPressed = false;
                                }
                            }
                            return false;
                        }
