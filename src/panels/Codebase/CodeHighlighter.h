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

    // Zelyn Color Palette
    static constexpr ImVec4 ZELYN_KEYWORD = ImVec4(0.78f, 0.57f, 0.92f, 1.0f); // #C792EA Electric Violet
    static constexpr ImVec4 ZELYN_TYPE    = ImVec4(0.54f, 0.87f, 1.0f, 1.0f); // #89DDFF Bright Cyan
    static constexpr ImVec4 ZELYN_FUNC    = ImVec4(1.00f, 0.80f, 0.42f, 1.0f); // #FFCB6B Warm Amber
    static constexpr ImVec4 ZELYN_STRING  = ImVec4(0.76f, 0.91f, 0.55f, 1.0f); // #C3E88D Soft Lime
    static constexpr ImVec4 ZELYN_COMMENT = ImVec4(0.48f, 0.52f, 0.56f, 1.0f); // Dim Desaturated Slate Gray
    static constexpr ImVec4 ZELYN_PREPROC = ImVec4(1.00f, 0.33f, 0.44f, 1.0f); // #FF5370 Coral Pink
    static constexpr ImVec4 ZELYN_NUMERIC = ImVec4(0.97f, 0.55f, 0.42f, 1.0f); // #F78C6C Vibrant Orange
    static constexpr ImVec4 ZELYN_PLAIN   = ImVec4(0.90f, 0.93f, 0.96f, 1.0f); // Soft Ice White

    // C++ Color Palette
    static constexpr ImVec4 CPP_KEYWORD = ImVec4(0.34f, 0.61f, 0.96f, 1.0f); // Soft Blue
    static constexpr ImVec4 CPP_TYPE    = ImVec4(0.31f, 0.80f, 0.77f, 1.0f); // Soft Teal
    static constexpr ImVec4 CPP_FUNC    = ImVec4(0.86f, 0.82f, 0.54f, 1.0f); // Soft Yellow
    static constexpr ImVec4 CPP_STRING  = ImVec4(0.88f, 0.62f, 0.44f, 1.0f); // Soft Orange
    static constexpr ImVec4 CPP_COMMENT = ImVec4(0.48f, 0.52f, 0.56f, 1.0f); // Dim Desaturated Slate Gray
    static constexpr ImVec4 CPP_PREPROC = ImVec4(0.75f, 0.52f, 0.90f, 1.0f); // Soft Purple
    static constexpr ImVec4 CPP_NUMERIC = ImVec4(0.70f, 0.85f, 0.60f, 1.0f); // Mint Green
    static constexpr ImVec4 CPP_PLAIN   = ImVec4(0.88f, 0.88f, 0.88f, 1.0f);
};

} // namespace EngineEditor

#endif // PANELS_CODEBASE_CODE_HIGHLIGHTER_H
