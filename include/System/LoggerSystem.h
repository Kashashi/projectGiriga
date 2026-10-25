#ifndef GIRIGARA_SYSTEM_LOGGER_SYSTEM_H
#define GIRIGARA_SYSTEM_LOGGER_SYSTEM_H

#include <string>
#include <deque>
#include <iostream>
#include <sstream>

enum class LogLevel {
    Info,
    Warning,
    Error
};

class LoggerSystem {
public:
    explicit LoggerSystem(size_t maxHistoryLines = 65535)
    : m_maxHistoryLines(maxHistoryLines) {}

    ~LoggerSystem() = default;

    LoggerSystem(const LoggerSystem&) = delete;
    LoggerSystem& operator=(const LoggerSystem&) = delete;

    // 核心 Log 函式
    void log(const std::string& message) {
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

    void info(const std::string& message) {
        log("[INFO] " + message);
    }

    void warn(const std::string& message) {
        log("[WARN] " + message);
    }

    void error(const std::string& message) {
        log("[ERROR] " + message);
    }

    void clear() {
        m_logs.clear();
        m_dirty = true;
    }

    const std::deque<std::string>& getLogs() const { return m_logs; }
    bool isDirty() const { return m_dirty; }
    void clearDirty() { m_dirty = false; }

private:
    std::deque<std::string> m_logs;
    size_t m_maxHistoryLines;
    bool m_dirty{true};
};

#endif // GIRIGARA_SYSTEM_LOGGER_SYSTEM_H
