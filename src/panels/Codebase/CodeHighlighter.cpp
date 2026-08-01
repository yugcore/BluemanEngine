#include "CodeHighlighter.h"
#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace EngineEditor {

LanguageType CodeHighlighter::GetLanguageFromExtension(const std::string& filename) {
    if (filename.length() >= 3 && filename.substr(filename.length() - 3) == ".zl") return LanguageType::Zelyn;
    if (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".zyn") return LanguageType::Zelyn;
    if (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".cpp") return LanguageType::Cpp;
    if (filename.length() >= 2 && filename.substr(filename.length() - 2) == ".h") return LanguageType::Header;
    if (filename.length() >= 4 && filename.substr(filename.length() - 4) == ".hpp") return LanguageType::Header;
    
    // Check if filename contains Zelyn
    if (filename.find("game_logic") != std::string::npos || filename.find("player") != std::string::npos) {
        return LanguageType::Zelyn;
    }
    return LanguageType::Cpp;
}

const char* CodeHighlighter::GetLanguageDisplayName(LanguageType lang) {
    switch (lang) {
        case LanguageType::Zelyn: return "Zelyn 1.0";
        case LanguageType::Cpp: return "C++";
        case LanguageType::Header: return "C++ Header";
        default: return "Plain Text";
    }
}

const char* CodeHighlighter::GetLanguageBadgeText(LanguageType lang) {
    switch (lang) {
        case LanguageType::Zelyn: return "";
        case LanguageType::Cpp: return "";
        case LanguageType::Header: return "";
        default: return "";
    }
}

ImVec4 CodeHighlighter::GetLanguageBadgeColor(LanguageType lang) {
    // Neutral monochrome color for all UI chrome badges (tab bar & file tree)
    return ImVec4(0.72f, 0.74f, 0.78f, 1.0f);
}

static bool IsIdentifierChar(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::vector<CodeToken> CodeHighlighter::TokenizeLine(const std::string& line, LanguageType lang) {
    std::vector<CodeToken> tokens;
    if (line.empty()) return tokens;

    // Sets for keywords & types
    static const std::unordered_set<std::string> zelynKeywords = {
        "func", "var", "let", "const", "return", "if", "else", "while", "for", "in",
        "class", "struct", "import", "async", "await", "match", "yield", "pub", "priv",
        "spawn", "destroy", "true", "false", "null"
    };

    static const std::unordered_set<std::string> zelynTypes = {
        "int", "float", "double", "string", "bool", "void", "Entity", "Vec3", "Vec2",
        "Color", "Shader", "Texture", "Transform", "Ray", "Scene", "System"
    };

    static const std::unordered_set<std::string> cppKeywords = {
        "int", "float", "double", "char", "void", "bool", "return", "if", "else",
        "for", "while", "do", "switch", "case", "break", "continue", "struct", "class",
        "namespace", "using", "public", "private", "protected", "static", "constexpr",
        "const", "noexcept", "auto", "nullptr", "sizeof", "true", "false"
    };

    static const std::unordered_set<std::string> cppTypes = {
        "Engine", "Renderer", "std", "uint32_t", "int32_t", "size_t", "std::string",
        "ImVec2", "ImVec4", "ImGuiWindowFlags", "AssetFolder"
    };

    const bool isZelyn = (lang == LanguageType::Zelyn);
    size_t i = 0;
    size_t n = line.length();

    while (i < n) {
        // Comments
        if (i + 1 < n && line[i] == '/' && line[i + 1] == '/') {
            tokens.push_back({ line.substr(i), isZelyn ? ZELYN_COMMENT : CPP_COMMENT });
            break;
        }

        // Preprocessor / Annotations
        if (line[i] == '#' || (isZelyn && line[i] == '@')) {
            size_t start = i;
            while (i < n && (IsIdentifierChar(line[i]) || line[i] == '#' || line[i] == '@')) i++;
            tokens.push_back({ line.substr(start, i - start), isZelyn ? ZELYN_PREPROC : CPP_PREPROC, false, true });
            continue;
        }

        // Strings
        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            size_t start = i++;
            while (i < n && line[i] != quote) {
                if (line[i] == '\\' && i + 1 < n) i += 2;
                else i++;
            }
            if (i < n) i++; // Include closing quote
            tokens.push_back({ line.substr(start, i - start), isZelyn ? ZELYN_STRING : CPP_STRING });
            continue;
        }

        // Numbers
        if (std::isdigit(static_cast<unsigned char>(line[i]))) {
            size_t start = i;
            while (i < n && (std::isdigit(static_cast<unsigned char>(line[i])) || line[i] == '.' || line[i] == 'f' || line[i] == 'u')) i++;
            tokens.push_back({ line.substr(start, i - start), isZelyn ? ZELYN_NUMERIC : CPP_NUMERIC });
            continue;
        }

        // Identifiers & Keywords
        if (IsIdentifierChar(line[i])) {
            size_t start = i;
            while (i < n && IsIdentifierChar(line[i])) i++;
            std::string word = line.substr(start, i - start);

            // Check if followed by '(' -> function call
            size_t lookAhead = i;
            while (lookAhead < n && std::isspace(static_cast<unsigned char>(line[lookAhead]))) lookAhead++;
            bool isFunc = (lookAhead < n && line[lookAhead] == '(');

            ImVec4 color = isZelyn ? ZELYN_PLAIN : CPP_PLAIN;
            bool isKw = false;

            if (isZelyn) {
                if (zelynKeywords.count(word)) { color = ZELYN_KEYWORD; isKw = true; }
                else if (zelynTypes.count(word)) { color = ZELYN_TYPE; }
                else if (isFunc) { color = ZELYN_FUNC; }
            } else {
                if (cppKeywords.count(word)) { color = CPP_KEYWORD; isKw = true; }
                else if (cppTypes.count(word)) { color = CPP_TYPE; }
                else if (isFunc) { color = CPP_FUNC; }
            }

            tokens.push_back({ word, color, isFunc, isKw });
            continue;
        }

        // Operators & Whitespace & Punctuation
        size_t start = i;
        while (i < n && !IsIdentifierChar(line[i]) && line[i] != '"' && line[i] != '\'' &&
               line[i] != '#' && (line[i] != '/' || (i + 1 < n && line[i + 1] != '/'))) {
            i++;
        }
        tokens.push_back({ line.substr(start, i - start), isZelyn ? ZELYN_TYPE : ImVec4(0.7f, 0.7f, 0.7f, 1.0f) });
    }

    return tokens;
}

void CodeHighlighter::RenderHighlightedLine(const std::string& line, LanguageType lang) {
    auto tokens = TokenizeLine(line, lang);
    if (tokens.empty()) {
        ImGui::TextUnformatted("");
        return;
    }
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) ImGui::SameLine(0.0f, 0.0f);
        ImGui::TextColored(tokens[i].color, "%s", tokens[i].text.c_str());

        // Connected Hover Docs for functions/keywords
        if (ImGui::IsItemHovered()) {
            HoverDoc doc;
            if (GetHoverDoc(tokens[i].text, lang, doc)) {
                ImGui::BeginTooltip();
                ImGui::TextColored(GetLanguageBadgeColor(lang), "[%s Doc]", doc.language.c_str());
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", doc.symbol.c_str());
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "Signature: %s", doc.signature.c_str());
                ImGui::TextUnformatted(doc.description.c_str());
                ImGui::EndTooltip();
            }
        }
    }
}

bool CodeHighlighter::GetHoverDoc(const std::string& token, LanguageType lang, HoverDoc& outDoc) {
    if (token == "Initialize" || token == "Engine::Initialize") {
        outDoc = { "Engine::Initialize", "void Engine::Initialize()", "Initializes the ZeGFX DirectX 12 render backend, window contexts, and asset registry.", "C++" };
        return true;
    }
    if (token == "Run" || token == "Engine::Run") {
        outDoc = { "Engine::Run", "void Engine::Run()", "Executes the core application message loop and hardware ray tracing solve.", "C++" };
        return true;
    }
    if (token == "Shutdown" || token == "Engine::Shutdown") {
        outDoc = { "Engine::Shutdown", "void Engine::Shutdown()", "Releases DX12 swapchain buffers, pipeline state objects, and cleans up resources.", "C++" };
        return true;
    }
    if (token == "RenderFrame" || token == "Renderer::RenderFrame") {
        outDoc = { "Renderer::RenderFrame", "void Renderer::RenderFrame()", "Executes DXR Ray Tracing solve & Volumetric Lighting pipeline pass.", "C++" };
        return true;
    }
    if (token == "spawn_entity" || token == "spawn") {
        outDoc = { "spawn_entity", "func spawn_entity(type: string, pos: Vec3) -> Entity", "Instantiates a new engine Entity into the active spatial partition grid.", "Zelyn" };
        return true;
    }
    if (token == "Raytrace" || token == "raytrace") {
        outDoc = { "Raytrace", "func Raytrace(origin: Vec3, dir: Vec3, maxDist: float) -> HitResult", "Performs hardware DXR acceleration structure traversal query.", "Zelyn" };
        return true;
    }
    if (token == "print") {
        outDoc = { "print", "func print(msg: string) -> void", "Logs text message to the ZeGFX Output Log & Terminal.", "Zelyn" };
        return true;
    }
    if (token == "Vector3" || token == "Vec3") {
        outDoc = { "Vec3", "struct Vec3 { float x, y, z; }", "Built-in 3D vector primitive with SIMD acceleration.", "Zelyn" };
        return true;
    }
    if (token == "@attribute" || token == "@export") {
        outDoc = { "@attribute", "@attribute(name: string, category: string)", "Exposes script property live to the Blueman Details Inspector.", "Zelyn" };
        return true;
    }
    return false;
}

bool CodeHighlighter::GetDiagnosticError(const std::string& filename, int lineNumber, std::string& outError) {
    if (filename.find("game_logic") != std::string::npos && lineNumber == 6) {
        outError = "Syntax Error: Missing semicolon or closing parenthesis after Raytrace call.";
        return true;
    }
    if (filename.find("Main.cpp") != std::string::npos && lineNumber == 15) {
        outError = "Warning: Engine::Initialize should be called before configuring viewports.";
        return true;
    }
    return false;
}

} // namespace EngineEditor
