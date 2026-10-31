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
              ConsoleSystem& consoleSystem, LoggerSystem& logger,
              ResourceManager& resourceManager) // ★ 加入 ResourceManager
    : UIContainer(x, y, width, height),
    m_font(font),
    m_eventBus(eventBus),
    m_consoleSystem(consoleSystem),
    m_logger(logger),
    m_resourceManager(resourceManager),
    // ★ 透過 ResourceManager 載入或取得背景 Shader
    m_bgShader(m_resourceManager.loadShader(
        "ConsoleBackground",
        "./assets/shaders/console_bg.vert",
        "./assets/shaders/console_bg.frag"
    )) {

        // 建立 OpenGL 4.6 Core Profile 繪製無 VBO 三角形時必需的空 VAO
        glGenVertexArrays(1, &m_dummyVAO);

        setVisible(false);

        // 依初始字體大小設定底部區域高度
        updateDynamicMetrics();

        float promptY = m_height - m_inputAreaHeight;
        m_inputPrompt = emplaceChild<UIText>(15.0f, promptY, m_font, U"> ", m_fontSize);
        m_inputPrompt->setColor(0.3f, 1.0f, 0.4f, 1.0f);

        m_inputDisplay = emplaceChild<UIText>(15.0f + static_cast<float>(m_fontSize) * 1.5f, promptY, m_font, U"", m_fontSize);
        m_inputDisplay->setColor(1.0f, 1.0f, 1.0f, 1.0f);

        recalculateLayout();
        bindEvents();
    }

    ~UIConsole() override {
        if (m_dummyVAO != 0) {
            glDeleteVertexArrays(1, &m_dummyVAO);
            m_dummyVAO = 0;
        }
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

        // 可見狀態下累加計時，提供給 Shader 作為泡泡飄動參數
        if (isVisible()) {
            m_elapsedTime += dt;
        }

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
    void render(const Shader& uiShader) override {
        if (!isVisible()) return;

        // 1. 啟用半透明混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 2. 切換為背景專用 Shader
        m_bgShader.use();
        GLuint bgProg = m_bgShader.getID();

        // ★ 關鍵修復：從已經持有正確正交投影的 uiShader 中讀出 projection 矩陣 (location 0)
        float orthoProj[16];
        glGetUniformfv(uiShader.getID(), 0, orthoProj);
        glProgramUniformMatrix4fv(bgProg, 0, 1, GL_FALSE, orthoProj); // location = 0: projection

        // 設定 Console 鋪滿畫面的 Model 矩陣 (location = 1)
        float model[16] = {
            m_width, 0.0f,     0.0f, 0.0f,
            0.0f,    m_height, 0.0f, 0.0f,
            0.0f,    0.0f,     1.0f, 0.0f,
            m_x,     m_y,      0.0f, 1.0f
        };
        glProgramUniformMatrix4fv(bgProg, 1, 1, GL_FALSE, model);     // location = 1: model

        // 設定時間與解析度
        glProgramUniform1f(bgProg, 2, m_elapsedTime);                 // location = 2: uTime
        glProgramUniform2f(bgProg, 3, m_width, m_height);             // location = 3: uResolution

        // 繪製背景矩形（注意：主迴圈已綁定 m_quadMesh->getVAO()，直接畫 6 個頂點即可）
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // 3. 畫完背景後，切換回原本的 UI Shader 繪製 Console 的文字與 Prompt
        uiShader.use();
        UIContainer::render(uiShader);
    }

private:
    void updateDynamicMetrics() {
        m_lineHeight = static_cast<float>(m_fontSize) * 1.35f;
        // 動態底部間距：隨著字體放大，拉開底邊保護緩衝，確保字型 Descent 完全落在視窗內
        m_bottomPadding = static_cast<float>(m_fontSize) * 0.35f + 8.0f;
        m_inputAreaHeight = static_cast<float>(m_fontSize) + m_bottomPadding;
    }

    void changeFontSize(int delta) {
        int newSize = std::clamp(m_fontSize + delta, 12, 48);
        if (newSize != m_fontSize) {
            m_fontSize = newSize;
            updateDynamicMetrics();

            m_inputPrompt->setFontSize(m_fontSize);
            m_inputDisplay->setFontSize(m_fontSize);

            recalculateLayout();
            refreshLogs();
        }
    }

    void recalculateLayout() {
        updateDynamicMetrics();

        // 貼齊視窗底部的抬高座標 (保留 bottomPadding)
        float promptY = m_height - m_inputAreaHeight;
        float promptWidth = static_cast<float>(m_fontSize) * 1.5f;

        m_inputPrompt->setPosition(15.0f, promptY);
        m_inputDisplay->setPosition(15.0f + promptWidth, promptY);

        // 可用日誌高度扣除動態輸入列與上方邊距
        float availableHeight = m_height - m_inputAreaHeight - 15.0f;
        m_maxVisibleLines = std::max(1, static_cast<int>(availableHeight / m_lineHeight));

        while (static_cast<int>(m_logLabels.size()) < m_maxVisibleLines) {
            auto* label = emplaceChild<UIText>(15.0f, 0.0f, m_font, U"", m_fontSize);
            label->setColor(0.88f, 0.88f, 0.88f, 1.0f);
            m_logLabels.push_back(label);
        }

        // 重新由下往上排版
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

            // 1. 放行 Console 開關鍵 (Grave Accent / Tilde)
            if (e.key == GLFW_KEY_GRAVE_ACCENT) {
                return;
            }

            // 2. ★ 關鍵修正：放行所有「Ctrl + Shift + 任意鍵」的超級全局組合鍵
            // 讓全局 UI 組件（如 UIWindow 或其他 Manager）能夠直接接收到！
            bool isSuperGlobalCombo = (e.mods & GLFW_MOD_CONTROL) && (e.mods & GLFW_MOD_SHIFT);
            if (isSuperGlobalCombo) {
                return; // 不標記 isHandled，放行給系統與其他 UI
            }

            // 3. ESC 直接關閉 Console
            if (e.key == GLFW_KEY_ESCAPE && e.action == GLFW_PRESS) {
                this->setVisible(false);
                e.isHandled = true;
                return;
            }

            // 攔截其餘一般輸入操作
            e.isHandled = true;

            if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
                bool ctrlPressed = (e.mods & GLFW_MOD_CONTROL);

                // 單純 Ctrl + '+' / '=' 放大
                if (ctrlPressed && (e.key == GLFW_KEY_EQUAL || e.key == GLFW_KEY_KP_ADD)) {
                    changeFontSize(2);
                    return;
                }
                // 單純 Ctrl + '-' 縮小
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
    ResourceManager& m_resourceManager; // ★ 引用 ResourceManager

    Shader& m_bgShader;                 // ★ 背景 Shader 引用
    GLuint m_dummyVAO{0};               // ★ 空 VAO
    float m_elapsedTime{0.0f};          // ★ 計時累積量

    int m_fontSize{18};
    float m_lineHeight{24.0f};
    float m_bottomPadding{14.0f};
    float m_inputAreaHeight{35.0f};
    int m_maxVisibleLines{15};
    int m_scrollOffset{0};

    std::u32string m_currentInput;
    UIText* m_inputPrompt{nullptr};
    UIText* m_inputDisplay{nullptr};
    std::vector<UIText*> m_logLabels;
};

#endif // GIRIGARA_UI_UICONSOLE_H
