#include "AssetTile.h"
#include "theme/Colors.h"
#include <imgui_internal.h>
#include <cmath>
#include <vector>
#include <cstdint>

namespace EngineEditor::Widgets {

static ImU32 GetMaterialBaseColor(const std::string& name) {
    if (name.find("Gold") != std::string::npos) return IM_COL32(245, 190, 60, 255);
    if (name.find("Chrome") != std::string::npos) return IM_COL32(220, 225, 235, 255);
    if (name.find("Copper") != std::string::npos) return IM_COL32(215, 115, 75, 255);
    if (name.find("Steel") != std::string::npos || name.find("Nickel") != std::string::npos) return IM_COL32(160, 165, 175, 255);
    if (name.find("Brick") != std::string::npos || name.find("Clay") != std::string::npos) return IM_COL32(170, 65, 50, 255);
    if (name.find("Cobble") != std::string::npos || name.find("Pebble") != std::string::npos) return IM_COL32(120, 100, 85, 255);
    if (name.find("Slate") != std::string::npos || name.find("Basalt") != std::string::npos) return IM_COL32(70, 80, 95, 255);
    if (name.find("Glass") != std::string::npos) return IM_COL32(100, 200, 240, 200);
    if (name.find("Grass") != std::string::npos) return IM_COL32(60, 150, 60, 255);
    if (name.find("Gravel") != std::string::npos) return IM_COL32(140, 130, 120, 255);
    if (name.find("Concrete") != std::string::npos) return IM_COL32(130, 135, 140, 255);
    
    // Hash string for deterministic distinct material hue
    size_t hash = std::hash<std::string>{}(name);
    uint8_t r = (hash & 0xFF) % 180 + 50;
    uint8_t g = ((hash >> 8) & 0xFF) % 180 + 50;
    uint8_t b = ((hash >> 16) & 0xFF) % 180 + 50;
    return IM_COL32(r, g, b, 255);
}

bool RenderAssetTile(const char* id, const char* name, AssetItemType itemType, const char* typeName, const ImVec4& typeColor, bool isSelected, float width, float height, bool* outDoubleClicked) {
    bool clicked = false;
    if (outDoubleClicked) *outDoubleClicked = false;
    const auto& pal = Theme::GetPalette();

    ImVec4 cardBg = isSelected ? ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.22f) : pal.bgElevated;
    ImVec4 cardBorder = isSelected ? pal.accent : pal.borderSubtle;

    ImGui::PushID(id);
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, cardBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, isSelected ? 1.5f : 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

    ImGui::BeginChild("AssetCardTile", ImVec2(width, height), true, ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cardMin = ImGui::GetWindowPos();

    // 1. Square Thumbnail Box (top 66px)
    float thumbH = 66.0f;
    ImVec2 thumbMin = ImVec2(cardMin.x + 4.0f, cardMin.y + 4.0f);
    ImVec2 thumbMax = ImVec2(cardMin.x + width - 4.0f, cardMin.y + 4.0f + thumbH);

    // Thumbnail background
    dl->AddRectFilled(thumbMin, thumbMax, IM_COL32(20, 22, 26, 255), 2.0f);
    dl->AddRect(thumbMin, thumbMax, IM_COL32(40, 44, 52, 255), 2.0f);

    ImVec2 center = ImVec2((thumbMin.x + thumbMax.x) * 0.5f, (thumbMin.y + thumbMax.y) * 0.5f);

    // Render Thumbnail Preview Graphics according to Asset Category
    if (itemType == AssetItemType::Material) {
        // --- Real PBR Material Sphere Preview ---
        float radius = 20.0f;
        ImU32 matColor = GetMaterialBaseColor(name);
        
        // Base material sphere
        dl->AddCircleFilled(center, radius, matColor, 32);
        
        // High-specular specular light highlight
        dl->AddCircleFilled(ImVec2(center.x - 5.0f, center.y - 5.0f), radius * 0.40f, IM_COL32(255, 255, 255, 220), 16);
        
        // Ambient rim light shadow
        dl->AddCircle(center, radius, IM_COL32(10, 10, 15, 180), 32, 1.5f);
    }
    else if (itemType == AssetItemType::Mesh) {
        // --- Static Mesh Wireframe/Cube Graphic ---
        float sz = 14.0f;
        dl->AddRect(ImVec2(center.x - sz, center.y - sz), ImVec2(center.x + sz, center.y + sz), IM_COL32(49, 130, 206, 255), 0.0f, 0, 1.5f);
        dl->AddLine(ImVec2(center.x - sz, center.y - sz), ImVec2(center.x - 5.0f, center.y - 5.0f), IM_COL32(49, 130, 206, 200), 1.5f);
        dl->AddLine(ImVec2(center.x + sz, center.y - sz), ImVec2(center.x + 5.0f, center.y - 5.0f), IM_COL32(49, 130, 206, 200), 1.5f);
    }
    else if (itemType == AssetItemType::Texture) {
        // --- Texture Image Canvas Graphic ---
        float w = 16.0f, h = 12.0f;
        dl->AddRectFilled(ImVec2(center.x - w, center.y - h), ImVec2(center.x + w, center.y + h), IM_COL32(35, 38, 45, 255), 2.0f);
        dl->AddRect(ImVec2(center.x - w, center.y - h), ImVec2(center.x + w, center.y + h), IM_COL32(214, 158, 46, 255), 2.0f, 0, 1.5f);
        dl->AddCircleFilled(ImVec2(center.x - 7.0f, center.y - 3.0f), 3.0f, IM_COL32(214, 158, 46, 255));
    }
    else if (itemType == AssetItemType::Script) {
        // --- Source Code Bracket Graphic ---
        dl->AddText(ImVec2(center.x - 12.0f, center.y - 9.0f), IM_COL32(128, 90, 213, 255), "{ ZLN }");
    }
    else if (itemType == AssetItemType::Audio) {
        // --- Soundwave Waveform Graphic ---
        float bars[] = { 5.0f, 12.0f, 18.0f, 8.0f, 14.0f, 6.0f };
        for (int b = 0; b < 6; ++b) {
            float bx = center.x - 15.0f + b * 6.0f;
            dl->AddLine(ImVec2(bx, center.y - bars[b] * 0.5f), ImVec2(bx, center.y + bars[b] * 0.5f), IM_COL32(49, 151, 149, 255), 2.0f);
        }
    }
    else if (itemType == AssetItemType::Level) {
        // --- Level Map Grid Graphic ---
        dl->AddRect(ImVec2(center.x - 14.0f, center.y - 10.0f), ImVec2(center.x + 14.0f, center.y + 10.0f), IM_COL32(229, 62, 62, 255), 2.0f, 0, 1.5f);
        dl->AddLine(ImVec2(center.x - 14.0f, center.y), ImVec2(center.x + 14.0f, center.y), IM_COL32(229, 62, 62, 180), 1.0f);
    }
    else if (itemType == AssetItemType::VFX) {
        // --- Sparkle Particle Graphic ---
        dl->AddLine(ImVec2(center.x - 10.0f, center.y), ImVec2(center.x + 10.0f, center.y), IM_COL32(221, 107, 32, 255), 2.0f);
        dl->AddLine(ImVec2(center.x, center.y - 10.0f), ImVec2(center.x, center.y + 10.0f), IM_COL32(221, 107, 32, 255), 2.0f);
    }
    else {
        // --- Generic Category Badge ---
        dl->AddRectFilled(ImVec2(center.x - 16.0f, center.y - 10.0f), ImVec2(center.x + 16.0f, center.y + 10.0f), IM_COL32(40, 45, 55, 255), 3.0f);
        dl->AddText(ImVec2(center.x - 12.0f, center.y - 7.0f), IM_COL32(200, 205, 215, 255), typeName);
    }

    // Category Badge in Top-Right Corner of Thumbnail
    ImU32 badgeCol = ImGui::ColorConvertFloat4ToU32(typeColor);
    dl->AddRectFilled(ImVec2(thumbMax.x - 34.0f, thumbMin.y + 2.0f), ImVec2(thumbMax.x - 2.0f, thumbMin.y + 14.0f), badgeCol, 2.0f);
    
    // Short type badge string
    const char* badgeStr = (itemType == AssetItemType::Material) ? "MAT" :
                           (itemType == AssetItemType::Mesh) ? "MESH" :
                           (itemType == AssetItemType::Texture) ? "TEX" :
                           (itemType == AssetItemType::Script) ? "ZLN" :
                           (itemType == AssetItemType::Audio) ? "SND" :
                           (itemType == AssetItemType::Level) ? "MAP" :
                           (itemType == AssetItemType::VFX) ? "VFX" : "DAT";
    dl->AddText(ImVec2(thumbMax.x - 31.0f, thumbMin.y + 1.0f), IM_COL32(10, 10, 15, 255), badgeStr);

    // 2. Color-Coded Underline Stripe below Thumbnail
    float stripeY = thumbMax.y + 2.0f;
    dl->AddLine(
        ImVec2(thumbMin.x, stripeY),
        ImVec2(thumbMax.x, stripeY),
        badgeCol, 2.0f);

    // 3. Asset Name Label with Truncation (...)
    ImGui::SetCursorScreenPos(ImVec2(thumbMin.x, stripeY + 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? pal.textPrimary : pal.textSecondary);

    std::string displayName = name;
    if (displayName.length() > 13) {
        displayName = displayName.substr(0, 10) + "...";
    }
    ImGui::TextUnformatted(displayName.c_str());

    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Click & Double-Click Detection
    if (ImGui::IsItemClicked()) {
        clicked = true;
    }
    if (outDoubleClicked && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        *outDoubleClicked = true;
    }

    // 4. Hover Tooltip for Full Asset Name & Path
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::TextColored(pal.textPrimary, "Asset: %s", name);
        ImGui::TextColored(pal.textSecondary, "Type: %s", typeName);
        ImGui::Separator();
        ImGui::TextColored(pal.textDisabled, "Path: ZeGFX Workspace/Content/%s", name);
        ImGui::EndTooltip();
    }

    ImGui::EndGroup();
    ImGui::PopID();

    return clicked;
}

} // namespace EngineEditor::Widgets