#ifndef GIRIGARA_UI_UICONSOLE_H
#define GIRIGARA_UI_UICONSOLE_H

#include <UI/UIContainer.h>
#include <UI/UIText.h>
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
    UIConsole(float x, float y, float width, float height, FontLoader& font, EventBus& eventBus)
    : UIContainer(x, y, width, height), m_font(font), m_eventBus(eventBus) {

        setVisible(false);

        // 建立輸入提示字元與當前輸入列
        m_inputPrompt = emplaceChild<UIText>(15.0f, m_height - m_inputAreaHeight + 5.0f, m_font, U"> ", m_fontSize);
        m_inputPrompt->setColor(0.3f, 1.0f, 0.4f, 1.0f);

        m_inputDisplay = emplaceChild<UIText>(35.0f, m_height - m_inputAreaHeight + 5.0f, m_font, U"", m_fontSize);
        m_inputDisplay->setColor(1.0f, 1.0f, 1.0f, 1.0f);

        recalculateLayout();
        bindEvents();
    }

    // 視窗大小改變時呼叫
    void onResize(float newWidth, float newHeight) {
        m_width = newWidth;
        m_height = newHeight;
        setSize(newWidth, newHeight); // 同步更新 UIContainer/UIElement 基礎屬性
        recalculateLayout();
        refreshLogs();
    }

    void update(float dt) override {
        UIContainer::update(dt);

        if (ConsoleSystem::getInstance().isDirty()) {
            refreshLogs();
            ConsoleSystem::getInstance().clearDirty();
        }
    }

private:
    void changeFontSize(int delta) {
        int newSize = std::clamp(m_fontSize + delta, 12, 48);
        if (newSize != m_fontSize) {
            m_fontSize = newSize;
            m_lineHeight = static_cast<float>(m_fontSize) * 1.35f;

            // 更新輸入列字型大小
            m_inputPrompt->setFontSize(m_fontSize);
            m_inputDisplay->setFontSize(m_fontSize);

            recalculateLayout();
            refreshLogs();
        }
    }

    // 依目前的高度與字體大小，重新計算行數與元件佈局
    void recalculateLayout() {
        // 更新輸入列位置 (固定貼齊底部)
        float promptY = m_height - m_inputAreaHeight + 5.0f;
        m_inputPrompt->setPosition(15.0f, promptY);
        m_inputDisplay->setPosition(15.0f + static_cast<float>(m_fontSize), promptY);

        // 計算主日誌區域可顯示的總行數
        float availableHeight = m_height - m_inputAreaHeight - 10.0f; // 扣除輸入列與頂部邊距
        m_maxVisibleLines = std::max(1, static_cast<int>(availableHeight / m_lineHeight));

        // 動態擴充或修整日誌 Label 物件池
        while (static_cast<int>(m_logLabels.size()) < m_maxVisibleLines) {
            auto* label = emplaceChild<UIText>(15.0f, 0.0f, m_font, U"", m_fontSize);
            label->setColor(0.88f, 0.88f, 0.88f, 1.0f);
            m_logLabels.push_back(label);
        }

        // 重新定位每一行：由最底下（緊貼輸入框上方）一路往上排到頂部
        for (int i = 0; i < static_cast<int>(m_logLabels.size()); ++i) {
            if (i < m_maxVisibleLines) {
                m_logLabels[i]->setVisible(true);
                m_logLabels[i]->setFontSize(m_fontSize);

                // i = 0 為最靠近輸入框的最新行，i 越大越往上
                float lineY = (m_height - m_inputAreaHeight - 10.0f) - (static_cast<float>(i + 1) * m_lineHeight);
                m_logLabels[i]->setPosition(15.0f, lineY);
            } else {
                // 超過可視範圍的多餘行數隱藏（不刪除）
                m_logLabels[i]->setVisible(false);
            }
        }
    }

    void bindEvents() {
        // ★ 自動監聽視窗縮放事件，始終填滿視窗並貼齊最底端
        m_eventBus.subscribe<WindowResizeEvent>([this](WindowResizeEvent& e) {
            this->onResize(static_cast<float>(e.width), static_cast<float>(e.height));
        });

        // 1. 攔截按鍵事件 (字體縮放、Enter、Backspace、ESC)
        m_eventBus.subscribePriority<KeyEvent>([this](KeyEvent& e) {
            if (!this->isVisible()) return;

            // 放行波浪鍵切換
            if (e.key == GLFW_KEY_GRAVE_ACCENT) {
                return;
            }

            // ESC 關閉主控台
            if (e.key == GLFW_KEY_ESCAPE && e.action == GLFW_PRESS) {
                this->setVisible(false);
                e.isHandled = true;
                return;
            }

            // 攔截其他鍵盤操作
            e.isHandled = true;

            if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
                bool ctrlPressed = (e.mods & GLFW_MOD_CONTROL);

                // Ctrl + '+' / '=' 放大文字
                if (ctrlPressed && (e.key == GLFW_KEY_EQUAL || e.key == GLFW_KEY_KP_ADD)) {
                    changeFontSize(2);
                    return;
                }
                // Ctrl + '-' 縮小文字
                if (ctrlPressed && (e.key == GLFW_KEY_MINUS || e.key == GLFW_KEY_KP_SUBTRACT)) {
                    changeFontSize(-2);
                    return;
                }

                // PageUp / PageDown 快速翻頁滾動
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
                        ConsoleSystem::getInstance().executeCommand(cmd);

                        m_currentInput.clear();
                        m_inputDisplay->setText(U"");
                        m_scrollOffset = 0; // 發布新指令後回到最新日誌
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

        // 2. 攔截滑鼠滾輪 (放大/縮小字體 或 捲動歷史日誌)
        m_eventBus.subscribePriority<MouseScrolledEvent>([this](MouseScrolledEvent& e) {
            if (!this->isVisible()) return;

            e.isHandled = true;

            // 透過 GLFW 查詢當前 Control 鍵是否按下
            GLFWwindow* curWin = glfwGetCurrentContext();
            bool ctrlPressed = false;
            if (curWin) {
                ctrlPressed = (glfwGetKey(curWin, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                glfwGetKey(curWin, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
            }

            if (ctrlPressed) {
                // Ctrl + 滾輪：縮放字體大小
                if (e.yoffset > 0) {
                    changeFontSize(2);
                } else if (e.yoffset < 0) {
                    changeFontSize(-2);
                }
            } else {
                // 普通滾輪：上下滾動歷史紀錄 (yoffset > 0 往上看舊日誌)
                scrollLogs(static_cast<int>(e.yoffset * 3)); // 乘上速度因子
            }
        });

        // 3. 攔截字元輸入
        m_eventBus.subscribePriority<CharInputEvent>([this](CharInputEvent& e) {
            if (!this->isVisible()) return;
            if (e.codepoint == '`' || e.codepoint == '~') return;

            e.isHandled = true;
            m_currentInput += static_cast<char32_t>(e.codepoint);
            m_inputDisplay->setText(m_currentInput);
        });

        // 4. 攔截滑鼠點擊與移動，避免穿透
        m_eventBus.subscribePriority<MouseButtonEvent>([this](MouseButtonEvent& e) {
            if (this->isVisible()) e.isHandled = true;
        });

            m_eventBus.subscribePriority<MouseMovedEvent>([this](MouseMovedEvent& e) {
                if (this->isVisible()) e.isHandled = true;
            });
    }

    void scrollLogs(int delta) {
        const auto& logs = ConsoleSystem::getInstance().getLogs();
        int totalLogs = static_cast<int>(logs.size());

        // 最大可向上滾動距離
        int maxOffset = std::max(0, totalLogs - m_maxVisibleLines);

        m_scrollOffset = std::clamp(m_scrollOffset + delta, 0, maxOffset);
        refreshLogs();
    }

    void refreshLogs() {
        const auto& logs = ConsoleSystem::getInstance().getLogs();
        int totalLogs = static_cast<int>(logs.size());

        // 從最底端（最新 - m_scrollOffset）開始向上繪製
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

    int m_fontSize{18};
    float m_lineHeight{24.0f};
    float m_inputAreaHeight{35.0f};
    int m_maxVisibleLines{15};
    int m_scrollOffset{0}; // 0 代表最底端的最新紀錄，> 0 代表向上翻閱舊日誌

    std::u32string m_currentInput;
    UIText* m_inputPrompt{nullptr};
    UIText* m_inputDisplay{nullptr};
    std::vector<UIText*> m_logLabels;
};

#endif // GIRIGARA_UI_UICONSOLE_H
