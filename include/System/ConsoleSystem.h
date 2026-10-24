#ifndef GIRIGARA_SYSTEM_CONSOLE_SYSTEM_H
#define GIRIGARA_SYSTEM_CONSOLE_SYSTEM_H

#include <string>
#include <vector>
#include <deque>
#include <functional>
#include <iostream>
#include <sstream>

class ScriptEngine;

class ConsoleSystem {
public:
    static ConsoleSystem& getInstance() {
        static ConsoleSystem instance;
        return instance;
    }

    void setScriptEngine(ScriptEngine* engine) {
        m_scriptEngine = engine;
    }

    // 寫入一筆日誌 (包含換行處理)
    void log(const std::string& message) {
        // 同時輸出至系統終端
        std::cout << message << std::endl;

        std::istringstream stream(message);
        std::string line;
        while (std::getline(stream, line)) {
            m_logs.push_back(line);
            if (m_logs.size() > m_maxHistoryLines) {
                m_logs.pop_front();
            }
        }
        m_dirty = true;
    }

    // 執行輸入的命令 (交給 Lua 或內建指令)
    void executeCommand(const std::string& cmd) {
        if (cmd.empty()) return;

        log("> " + cmd);
        m_commandHistory.push_back(cmd);

        if (cmd == "clear") {
            clear();
            return;
        }

        if (m_scriptEngine) {
            // 直接由 Lua 引擎執行字串指令
            executeLua(cmd);
        } else {
            log("[Error] ScriptEngine 未連結，無法執行指令");
        }
    }

    void clear() {
        m_logs.clear();
        m_dirty = true;
    }

    const std::deque<std::string>& getLogs() const { return m_logs; }
    bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

private:
    ConsoleSystem() = default;
    void executeLua(const std::string& code);

    ScriptEngine* m_scriptEngine{nullptr};
    std::deque<std::string> m_logs;
    std::vector<std::string> m_commandHistory;
    size_t m_maxHistoryLines{65535}; // 儲存 65535 行
    bool m_dirty{true};
};

#endif // GIRIGARA_SYSTEM_CONSOLE_SYSTEM_H
