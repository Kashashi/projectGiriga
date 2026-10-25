#ifndef GIRIGARA_UI_UICONSOLE_H
#define GIRIGARA_UI_UICONSOLE_H

#include <UI/UIContainer.h>
#include <UI/UIText.h>
#include <System/LoggerSystem.h>
#include <System/ConsoleSystem.h>
#include <Font/FontLoader.h>
#include <Event/EventBus.h>
#include <Event/Event.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>
#include <algorithm>

namespace EncodingUtils {
    inline std::u32string utf8ToUtf32(const std::string& str) {
        std::u32string out;
        size_t i = 0;
        while (i < str.size()) {
            unsigned char c = str[i];
            char32_t cp = 0;
            size_t bytes = 0;

            if (c < 0x80) {
                cp = c;
                bytes = 1;
            } else if ((c & 0xE0) == 0xC0) {
                cp = c & 0x1F;
                bytes = 2;
            } else if ((c & 0xF0) == 0xE0) {
                cp = c & 0x0F;
                bytes = 3;
            } else if ((c & 0xF8) == 0xF0) {
                cp = c & 0x07;
                bytes = 4;
            } else {
                i++;
                continue;
            }

            if (i + bytes > str.size()) break;

            for (size_t j = 1; j < bytes; ++j) {
                cp = (cp << 6) | (str[i + j] & 0x3F);
            }
            out.push_back(cp);
            i += bytes;
        }
        return out;
    }

    inline std::string utf32ToUtf8(const std::u32string& str) {
        std::string out;
        for (char32_t cp : str) {
            if (cp <= 0x7F) {
                out.push_back(static_cast<char>(cp));
            } else if (cp <= 0x7FF) {
                out.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else if (cp <= 0xFFFF) {
                out.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else if (cp <= 0x10FFFF) {
                out.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
        }
        return out;
    }
}

class UIConsole : public UIContainer {
public:
    UIConsole(float x, float y, float width, float height,
              FontLoader& font, EventBus& eventBus,
              ConsoleSystem& consoleSystem, LoggerSystem& logger)
    : UIContainer(x, y, width, height),
    m_font(font),
    m_eventBus(eventBus),
    m_consoleSystem(consoleSystem),
    m_logger(logger) {

        setVisible(false);

        m_inputPrompt = emplaceChild<UIText>(15.0f, m_height - m_inputAreaHeight + 5.0f, m_font, U"> ", m_fontSize);
        m_inputPrompt->setColor(0.3f, 1.0f, 0.4f, 1.0f);

        m_inputDisplay = emplaceChild<UIText>(35.0f, m_height - m_inputAreaHeight + 5.0f, m_font, U"", m_fontSize);
        m_inputDisplay->setColor(1.0f, 1.0f, 1.0f, 1.0f);

        recalculateLayout();
        bindEvents();
    }

    void onResize(float newWidth, float newHeight) {
        m_width = newWidth;
        m_height = newHeight;
        setSize(newWidth, newHeight);
        recalculateLayout();
        refreshLogs();
    }

    void update(float dt) override {
        UIContainer::update(dt);

        if (m_logger.isDirty()) {
            refreshLogs();
            m_logger.clearDirty();
        }
    }

    bool handleMouseMove(double x, double y) override {
        if (!isVisible()) return false;
        UIContainer::handleMouseMove(x, y);
        return true; // Console 開啟時，攔截滑鼠移動
    }

    bool handleMouseButton(int button, int action, int mods, double x, double y) override {
        if (!isVisible()) return false;
        UIContainer::handleMouseButton(button, action, mods, x, y);
        return true; // Console 開啟時，攔截滑鼠點擊
    }

private:
    void changeFontSize(int delta) {
        int newSize = std::clamp(m_fontSize + delta, 12, 48);
        if (newSize != m_fontSize) {
            m_fontSize = newSize;
            m_lineHeight = static_cast<float>(m_fontSize) * 1.35f;

            m_inputPrompt->setFontSize(m_fontSize);
            m_inputDisplay->setFontSize(m_fontSize);

            recalculateLayout();
            refreshLogs();
        }
    }

    void recalculateLayout() {
        float promptY = m_height - m_inputAreaHeight + 5.0f;
        m_inputPrompt->setPosition(15.0f, promptY);
        m_inputDisplay->setPosition(15.0f + static_cast<float>(m_fontSize), promptY);

        float availableHeight = m_height - m_inputAreaHeight - 10.0f;
        m_maxVisibleLines = std::max(1, static_cast<int>(availableHeight / m_lineHeight));

        while (static_cast<int>(m_logLabels.size()) < m_maxVisibleLines) {
            auto* label = emplaceChild<UIText>(15.0f, 0.0f, m_font, U"", m_fontSize);
            label->setColor(0.88f, 0.88f, 0.88f, 1.0f);
            m_logLabels.push_back(label);
        }

        for (int i = 0; i < static_cast<int>(m_logLabels.size()); ++i) {
            if (i < m_maxVisibleLines) {
                m_logLabels[i]->setVisible(true);
                m_logLabels[i]->setFontSize(m_fontSize);

                float lineY = (m_height - m_inputAreaHeight - 10.0f) - (static_cast<float>(i + 1) * m_lineHeight);
                m_logLabels[i]->setPosition(15.0f, lineY);
            } else {
                m_logLabels[i]->setVisible(false);
            }
        }
    }

    void bindEvents() {
        m_eventBus.subscribe<WindowResizeEvent>([this](WindowResizeEvent& e) {
            this->onResize(static_cast<float>(e.width), static_cast<float>(e.height));
        });

        m_eventBus.subscribePriority<KeyEvent>([this](KeyEvent& e) {
            if (!this->isVisible()) return;

            if (e.key == GLFW_KEY_GRAVE_ACCENT) {
                return;
            }

            if (e.key == GLFW_KEY_ESCAPE && e.action == GLFW_PRESS) {
                this->setVisible(false);
                e.isHandled = true;
                return;
            }

            e.isHandled = true;

            if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
                bool ctrlPressed = (e.mods & GLFW_MOD_CONTROL);

                if (ctrlPressed && (e.key == GLFW_KEY_EQUAL || e.key == GLFW_KEY_KP_ADD)) {
                    changeFontSize(2);
                    return;
                }
                if (ctrlPressed && (e.key == GLFW_KEY_MINUS || e.key == GLFW_KEY_KP_SUBTRACT)) {
                    changeFontSize(-2);
                    return;
                }

                if (e.key == GLFW_KEY_PAGE_UP) {
                    scrollLogs(m_maxVisibleLines / 2);
                    return;
                }
                if (e.key == GLFW_KEY_PAGE_DOWN) {
                    scrollLogs(-(m_maxVisibleLines / 2));
                    return;
                }

                if (e.key == GLFW_KEY_ENTER) {
                    if (!m_currentInput.empty()) {
                        std::string cmd = EncodingUtils::utf32ToUtf8(m_currentInput);
                        m_consoleSystem.executeCommand(cmd);

                        m_currentInput.clear();
                        m_inputDisplay->setText(U"");
                        m_scrollOffset = 0;
                        refreshLogs();
                    }
                }
                else if (e.key == GLFW_KEY_BACKSPACE) {
                    if (!m_currentInput.empty()) {
                        m_currentInput.pop_back();
                        m_inputDisplay->setText(m_currentInput);
                    }
                }
            }
        });

        m_eventBus.subscribePriority<MouseScrolledEvent>([this](MouseScrolledEvent& e) {
            if (!this->isVisible()) return;

            e.isHandled = true;

            GLFWwindow* curWin = glfwGetCurrentContext();
            bool ctrlPressed = false;
            if (curWin) {
                ctrlPressed = (glfwGetKey(curWin, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(curWin, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
            }

            if (ctrlPressed) {
                if (e.yoffset > 0) {
                    changeFontSize(2);
                } else if (e.yoffset < 0) {
                    changeFontSize(-2);
                }
            } else {
                scrollLogs(static_cast<int>(e.yoffset * 3));
            }
        });

        m_eventBus.subscribePriority<CharInputEvent>([this](CharInputEvent& e) {
            if (!this->isVisible()) return;
            if (e.codepoint == '`' || e.codepoint == '~') return;

            e.isHandled = true;
            m_currentInput += static_cast<char32_t>(e.codepoint);
            m_inputDisplay->setText(m_currentInput);
        });
        // 務必註解或刪除這兩段，避免全域攔截滑鼠：
        // m_eventBus.subscribePriority<MouseButtonEvent>([this](MouseButtonEvent& e) {
        //     if (this->isVisible()) e.isHandled = true;
        // });
        //
        // m_eventBus.subscribePriority<MouseMovedEvent>([this](MouseMovedEvent& e) {
        //     if (this->isVisible()) e.isHandled = true;
        // });
    }

    void scrollLogs(int delta) {
        const auto& logs = m_logger.getLogs();
        int totalLogs = static_cast<int>(logs.size());
        int maxOffset = std::max(0, totalLogs - m_maxVisibleLines);

        m_scrollOffset = std::clamp(m_scrollOffset + delta, 0, maxOffset);
        refreshLogs();
    }

    void refreshLogs() {
        const auto& logs = m_logger.getLogs();
        int totalLogs = static_cast<int>(logs.size());

        for (int i = 0; i < m_maxVisibleLines; ++i) {
            if (i >= static_cast<int>(m_logLabels.size())) break;

            int targetIdx = totalLogs - 1 - m_scrollOffset - i;
            if (targetIdx >= 0 && targetIdx < totalLogs) {
                std::u32string u32line = EncodingUtils::utf8ToUtf32(logs[targetIdx]);
                m_logLabels[i]->setText(u32line);
            } else {
                m_logLabels[i]->setText(U"");
            }
        }
    }

    FontLoader& m_font;
    EventBus& m_eventBus;
    ConsoleSystem& m_consoleSystem;
    LoggerSystem& m_logger;

    int m_fontSize{18};
    float m_lineHeight{24.0f};
    float m_inputAreaHeight{35.0f};
    int m_maxVisibleLines{15};
    int m_scrollOffset{0};

    std::u32string m_currentInput;
    UIText* m_inputPrompt{nullptr};
    UIText* m_inputDisplay{nullptr};
    std::vector<UIText*> m_logLabels;
};

#endif // GIRIGARA_UI_UICONSOLE_H
