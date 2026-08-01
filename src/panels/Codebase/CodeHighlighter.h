#ifndef PANELS_CODEBASE_CODE_HIGHLIGHTER_H
#define PANELS_CODEBASE_CODE_HIGHLIGHTER_H

#include <imgui.h>
#include <string>
#include <vector>

namespace EngineEditor {

enum class LanguageType {
    Cpp,
    Header,
    Zelyn,
    Unknown
};

struct HoverDoc {
    std::string symbol;
    std::string signature;
    std::string description;
    std::string language;
};

struct CodeToken {
    std::string text;
    ImVec4 color;
    bool isFunction = false;
    bool isKeyword = false;
};

class CodeHighlighter {
public:
    static LanguageType GetLanguageFromExtension(const std::string& filename);
    static const char* GetLanguageDisplayName(LanguageType lang);
    static const char* GetLanguageBadgeText(LanguageType lang);
    static ImVec4 GetLanguageBadgeColor(LanguageType lang);

    static std::vector<CodeToken> TokenizeLine(const std::string& line, LanguageType lang);
    static void RenderHighlightedLine(const std::string& line, LanguageType lang);

    static bool GetHoverDoc(const std::string& token, LanguageType lang, HoverDoc& outDoc);
    static bool GetDiagnosticError(const std::string& filename, int lineNumber, std::string& outError);

    // Zelyn Color Palette (Monochrome Grayscale)
    static constexpr ImVec4 ZELYN_KEYWORD = ImVec4(1.00f, 1.00f, 1.00f, 1.0f); // Pure White
    static constexpr ImVec4 ZELYN_TYPE    = ImVec4(0.82f, 0.82f, 0.82f, 1.0f); // Light Silver
    static constexpr ImVec4 ZELYN_FUNC    = ImVec4(0.92f, 0.92f, 0.92f, 1.0f); // Soft Ice White
    static constexpr ImVec4 ZELYN_STRING  = ImVec4(0.72f, 0.72f, 0.72f, 1.0f); // Silver Gray
    static constexpr ImVec4 ZELYN_COMMENT = ImVec4(0.45f, 0.45f, 0.45f, 1.0f); // Dim Gray
    static constexpr ImVec4 ZELYN_PREPROC = ImVec4(0.85f, 0.85f, 0.85f, 1.0f); // Light Gray
    static constexpr ImVec4 ZELYN_NUMERIC = ImVec4(0.78f, 0.78f, 0.78f, 1.0f); // Gray
    static constexpr ImVec4 ZELYN_PLAIN   = ImVec4(0.95f, 0.95f, 0.95f, 1.0f); // Pure White

    // C++ Color Palette (Monochrome Grayscale)
    static constexpr ImVec4 CPP_KEYWORD = ImVec4(1.00f, 1.00f, 1.00f, 1.0f); // Pure White
    static constexpr ImVec4 CPP_TYPE    = ImVec4(0.82f, 0.82f, 0.82f, 1.0f); // Light Silver
    static constexpr ImVec4 CPP_FUNC    = ImVec4(0.92f, 0.92f, 0.92f, 1.0f); // Soft Ice White
    static constexpr ImVec4 CPP_STRING  = ImVec4(0.72f, 0.72f, 0.72f, 1.0f); // Silver Gray
    static constexpr ImVec4 CPP_COMMENT = ImVec4(0.45f, 0.45f, 0.45f, 1.0f); // Dim Gray
    static constexpr ImVec4 CPP_PREPROC = ImVec4(0.85f, 0.85f, 0.85f, 1.0f); // Light Gray
    static constexpr ImVec4 CPP_NUMERIC = ImVec4(0.78f, 0.78f, 0.78f, 1.0f); // Gray
    static constexpr ImVec4 CPP_PLAIN   = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
};

} // namespace EngineEditor

#endif // PANELS_CODEBASE_CODE_HIGHLIGHTER_H
