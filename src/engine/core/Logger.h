#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>

namespace EngineEditor {

static constexpr size_t kMaxLogMessages = 10000;

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

private:
    void AddMessage(LogSeverity severity, const std::string& msg);
    void SeedStartupLogs();

    std::vector<LogMessage> m_Messages;
};

} // namespace EngineEditor

#endif // LOGGER_H
