#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>
#include <imgui.h>

namespace EngineEditor {

enum class LogSeverity {
    Info,
    Warning,
    Error
};

struct LogMessage {
    std::string timestamp;
    LogSeverity severity;
    std::string message;
};

class Logger {
public:
    static Logger& Get();

    Logger();

    void Info(const std::string& msg);
    void Warning(const std::string& msg);
    void Error(const std::string& msg);
    void Clear();

    const std::vector<LogMessage>& GetMessages() const { return m_Messages; }

    static ImVec4 GetSeverityColor(LogSeverity severity);

private:
    void AddMessage(LogSeverity severity, const std::string& msg);
    void SeedStartupLogs();

    std::vector<LogMessage> m_Messages;
};

} // namespace EngineEditor

#endif // LOGGER_H
