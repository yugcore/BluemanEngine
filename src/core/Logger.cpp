#include "Logger.h"
#include "theme/Colors.h"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace EngineEditor {

Logger& Logger::Get() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    Info("Blueman Engine Editor Log Stream Initialized.");
}

static std::string GetCurrentTimestamp() {
    std::time_t t = std::time(nullptr);
    std::tm tm;
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::setfill('0')
       << std::setw(2) << tm.tm_hour << ":"
       << std::setw(2) << tm.tm_min << ":"
       << std::setw(2) << tm.tm_sec;
    return ss.str();
}

void Logger::AddMessage(LogSeverity severity, const std::string& msg) {
    // Enforce bounded capacity — drop oldest 25% when full
    if (m_Messages.size() >= kMaxLogMessages) {
        size_t dropCount = kMaxLogMessages / 4;
        m_Messages.erase(m_Messages.begin(), m_Messages.begin() + dropCount);
    }
    m_Messages.push_back({ GetCurrentTimestamp(), severity, msg });
}

void Logger::Info(const std::string& msg) {
    AddMessage(LogSeverity::Info, msg);
}

void Logger::Warning(const std::string& msg) {
    AddMessage(LogSeverity::Warning, msg);
}

void Logger::Error(const std::string& msg) {
    AddMessage(LogSeverity::Error, msg);
}

void Logger::Clear() {
    m_Messages.clear();
}

ImVec4 Logger::GetSeverityColor(LogSeverity severity) {
    const auto& pal = Theme::GetPalette();
    switch (severity) {
        case LogSeverity::Info:    return pal.textPrimary;
        case LogSeverity::Warning: return pal.statusWarning;
        case LogSeverity::Error:   return pal.statusError;
        default:                   return pal.textSecondary;
    }
}

} // namespace EngineEditor
