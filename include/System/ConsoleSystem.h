#ifndef GIRIGARA_SYSTEM_CONSOLE_SYSTEM_H
#define GIRIGARA_SYSTEM_CONSOLE_SYSTEM_H

#include <string>
#include <vector>
#include <System/LoggerSystem.h>

class ScriptEngine;

class ConsoleSystem {
public:
    explicit ConsoleSystem(LoggerSystem& logger)
    : m_logger(logger) {}

    void setScriptEngine(ScriptEngine* engine) {
        m_scriptEngine = engine;
    }

    void executeCommand(const std::string& cmd) {
        if (cmd.empty()) return;

        m_logger.log("> " + cmd);
        m_commandHistory.push_back(cmd);

        if (cmd == "clear") {
            m_logger.clear();
            return;
        }

        if (m_scriptEngine) {
            executeLua(cmd);
        } else {
            m_logger.error("ScriptEngine 未連結，無法執行指令");
        }
    }

    LoggerSystem& getLogger() { return m_logger; }
    const std::vector<std::string>& getHistory() const { return m_commandHistory; }

private:
    void executeLua(const std::string& code);

    LoggerSystem& m_logger;
    ScriptEngine* m_scriptEngine{nullptr};
    std::vector<std::string> m_commandHistory;
};

#endif // GIRIGARA_SYSTEM_CONSOLE_SYSTEM_H
