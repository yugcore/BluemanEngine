#include "Overlay.h"
#include "core/EditorState.h"
#include "engine/scene/SceneGraph.h"
#include "third_party/IconsFontAwesome6.h"
#include <cmath>
#include <algorithm>

namespace EngineEditor::Panels {

static bool WorldToScreen(const float pos[3], const float view[16], const float proj[16], ImVec2 cursorPos, ImVec2 viewportAvail, ImVec2& outScreen) {
    float x = pos[0], y = pos[1], z = pos[2];

    // Clip space coordinates: clipPos = pos * view * proj
    float clipX = x * view[0] + y * view[4] + z * view[8] + view[12];
    float clipY = x * view[1] + y * view[5] + z * view[9] + view[13];
    float clipZ = x * view[2] + y * view[6] + z * view[10] + view[14];
    float clipW = x * view[3] + y * view[7] + z * view[11] + view[15];

    float ndcX = clipX * proj[0] + clipY * proj[4] + clipZ * proj[8] + proj[12];
    float ndcY = clipX * proj[1] + clipY * proj[5] + clipZ * proj[9] + proj[13];
    float ndcW = clipX * proj[3] + clipY * proj[7] + clipZ * proj[11] + proj[15];

    if (ndcW <= 0.001f) return false;

    float ndc2X = ndcX / ndcW;
    float ndc2Y = ndcY / ndcW;

    outScreen.x = cursorPos.x + (ndc2X * 0.5f + 0.5f) * viewportAvail.x;
    outScreen.y = cursorPos.y + (1.0f - (ndc2Y * 0.5f + 0.5f)) * viewportAvail.y;

    return true;
}

void RenderViewport3DOverlays(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, int showFlags) {
    if (!drawList || viewportAvail.x <= 0.0f || viewportAvail.y <= 0.0f) return;

    float viewMat[16];
    float projMat[16];

    const auto& camera = EditorState::Get().camera;
    camera.GetViewMatrix(viewMat);
    float aspect = (viewportAvail.y > 0.0f) ? (viewportAvail.x / viewportAvail.y) : 1.777f;
    camera.GetProjectionMatrix(aspect, projMat);

    bool showGrid = (showFlags & 1) != 0;
    bool showLights = (showFlags & 2) != 0;

    // 1. Render 3D Solid Landscape / Heightfield Terrain Mesh & Overlay Grid
    if (showGrid) {
        const int landscapeGridRadius = 20; // 40x40 plane grid
        const float landscapeGridSpacing = 2.5f; // Spans 100m x 100m ground plane

        ImU32 axisColorX    = IM_COL32(235, 65, 65, 230);  // X Axis (Red)
        ImU32 axisColorZ    = IM_COL32(65, 135, 245, 230); // Z Axis (Blue)
        ImU32 gridLineColor = IM_COL32(160, 175, 190, 140); // Soft slate grid lines on white plane
        ImU32 ridgeLineColor= IM_COL32(110, 130, 150, 180); // Major grid accent lines

        // Gentle flat plane heightfield (subtle ground elevation)
        auto getTerrainHeight = [](float x, float z) -> float {
            float dist = std::sqrt(x * x + z * z);
            if (dist < 5.0f) return 0.0f; // Central spawn plane is completely flat 0.0m
            return std::sin(x * 0.10f) * std::cos(z * 0.10f) * 0.25f; // Very subtle micro-elevation
        };

        // Directional Sun Light Vector for Surface Shading
        const SceneNode* sunNode = SceneGraph::Get().FindNode("DirectionalSunLight");
        float sunDir[3] = { -0.5f, -0.8f, -0.3f };
        if (sunNode) {
            float pitchRad = sunNode->rotation[0] * 3.14159265f / 180.0f;
            float yawRad   = sunNode->rotation[1] * 3.14159265f / 180.0f;
            sunDir[0] = std::cos(pitchRad) * std::sin(yawRad);
            sunDir[1] = -std::sin(pitchRad);
            sunDir[2] = -std::cos(pitchRad) * std::cos(yawRad);
        }
        float sunLength = std::sqrt(sunDir[0]*sunDir[0] + sunDir[1]*sunDir[1] + sunDir[2]*sunDir[2]);
        if (sunLength > 0.001f) { sunDir[0] /= sunLength; sunDir[1] /= sunLength; sunDir[2] /= sunLength; }

        // Structure for depth-sorted Landscape quads (Painters Algorithm)
        struct LandscapeQuad {
            float distSq;
            float p1[3], p2[3], p3[3], p4[3];
            ImU32 fillColor;
            bool isAxisX, isAxisZ, isMajorGrid;
        };

        const auto& camPos = camera.GetPosition();
        float camX = camPos.x, camY = camPos.y, camZ = camPos.z;

        std::vector<LandscapeQuad> quads;
        quads.reserve((size_t)(landscapeGridRadius * 2) * (landscapeGridRadius * 2));

        for (int i = -landscapeGridRadius; i < landscapeGridRadius; ++i) {
            float x1 = (float)i * landscapeGridSpacing;
            float x2 = (float)(i + 1) * landscapeGridSpacing;

            for (int j = -landscapeGridRadius; j < landscapeGridRadius; ++j) {
                float z1 = (float)j * landscapeGridSpacing;
                float z2 = (float)(j + 1) * landscapeGridSpacing;

                float y11 = getTerrainHeight(x1, z1);
                float y21 = getTerrainHeight(x2, z1);
                float y22 = getTerrainHeight(x2, z2);
                float y12 = getTerrainHeight(x1, z2);

                LandscapeQuad quad;
                quad.p1[0] = x1;  quad.p1[1] = y11; quad.p1[2] = z1; // Top-Left
                quad.p2[0] = x2;  quad.p2[1] = y21; quad.p2[2] = z1; // Top-Right
                quad.p3[0] = x2;  quad.p3[1] = y22; quad.p3[2] = z2; // Bottom-Right
                quad.p4[0] = x1;  quad.p4[1] = y12; quad.p4[2] = z2; // Bottom-Left

                // Quad center & distance from camera for depth sorting
                float centerX = (x1 + x2) * 0.5f;
                float centerY = (y11 + y21 + y22 + y12) * 0.25f;
                float centerZ = (z1 + z2) * 0.5f;
                float dx = centerX - camX, dy = centerY - camY, dz = centerZ - camZ;
                quad.distSq = dx * dx + dy * dy + dz * dz;

                // Calculate Surface Normal for Diffuse Lighting
                float edge1[3] = { x2 - x1, y21 - y11, 0.0f };
                float edge2[3] = { 0.0f, y12 - y11, z2 - z1 };
                float norm[3] = {
                    edge1[1] * edge2[2] - edge1[2] * edge2[1],
                    edge1[2] * edge2[0] - edge1[0] * edge2[2],
                    edge1[0] * edge2[1] - edge1[1] * edge2[0]
                };
                float nLen = std::sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);
                if (nLen > 0.0001f) { norm[0] /= nLen; norm[1] /= nLen; norm[2] /= nLen; }
                else { norm[0] = 0.0f; norm[1] = 1.0f; norm[2] = 0.0f; }

                // Diffuse Lighting Dot Product
                float dot = -(norm[0]*sunDir[0] + norm[1]*sunDir[1] + norm[2]*sunDir[2]);
                float lightFactor = std::clamp(dot, 0.75f, 1.00f);

                // Clean White Plane Surface Shading
                float rBase = 0.91f, gBase = 0.93f, bBase = 0.96f; // Off-White / Bright Neutral Plane

                uint8_t r = (uint8_t)(std::clamp(rBase * lightFactor, 0.0f, 1.0f) * 255.0f);
                uint8_t g = (uint8_t)(std::clamp(gBase * lightFactor, 0.0f, 1.0f) * 255.0f);
                uint8_t b = (uint8_t)(std::clamp(bBase * lightFactor, 0.0f, 1.0f) * 255.0f);
                quad.fillColor = IM_COL32(r, g, b, 245); // Opaque Solid White Ground Surface

                quad.isAxisX = (j == 0);
                quad.isAxisZ = (i == 0);
                quad.isMajorGrid = (i % 5 == 0 || j % 5 == 0);

                quads.push_back(quad);
            }
        }

        // Sort quads back-to-front (Painters Algorithm for clean depth rendering)
        std::sort(quads.begin(), quads.end(), [](const LandscapeQuad& a, const LandscapeQuad& b) {
            return a.distSq > b.distSq;
        });

        // Render Solid Landscape Surface Quads & Overlaid Grid Lines
        for (const auto& q : quads) {
            ImVec2 s1, s2, s3, s4;
            bool v1 = WorldToScreen(q.p1, viewMat, projMat, cursorPos, viewportAvail, s1);
            bool v2 = WorldToScreen(q.p2, viewMat, projMat, cursorPos, viewportAvail, s2);
            bool v3 = WorldToScreen(q.p3, viewMat, projMat, cursorPos, viewportAvail, s3);
            bool v4 = WorldToScreen(q.p4, viewMat, projMat, cursorPos, viewportAvail, s4);

            if (v1 && v2 && v3 && v4) {
                // Draw Overlaid Grid Wireframe Edges only (transparent background for ZeGFX 3D GPU render)
                ImU32 lineCol = q.isMajorGrid ? ridgeLineColor : gridLineColor;
                float lineThick = q.isMajorGrid ? 1.5f : 1.0f;

                if (q.isAxisZ) { drawList->AddLine(s1, s4, axisColorZ, 2.5f); }
                else { drawList->AddLine(s1, s4, lineCol, lineThick); }

                if (q.isAxisX) { drawList->AddLine(s1, s2, axisColorX, 2.5f); }
                else { drawList->AddLine(s1, s2, lineCol, lineThick); }
            }
        }
    }

    // 2. Render Directional Sun Light Visual Icon & Light Ray Direction Vector
    if (showLights) {
        const SceneNode* sunNodeVis = SceneGraph::Get().FindNode("DirectionalSunLight");
        float sunPos[3] = { 8.0f, 15.0f, -8.0f };
        float sunDir[3] = { -0.5f, -0.8f, -0.3f };
        if (sunNodeVis) {
            sunPos[0] = sunNodeVis->location[0];
            sunPos[1] = sunNodeVis->location[1];
            sunPos[2] = sunNodeVis->location[2];
            float pitchRad = sunNodeVis->rotation[0] * 3.14159265f / 180.0f;
            float yawRad   = sunNodeVis->rotation[1] * 3.14159265f / 180.0f;
            sunDir[0] = std::cos(pitchRad) * std::sin(yawRad);
            sunDir[1] = -std::sin(pitchRad);
            sunDir[2] = -std::cos(pitchRad) * std::cos(yawRad);
        }
        float sunEnd[3] = { sunPos[0] + sunDir[0] * 6.0f, sunPos[1] + sunDir[1] * 6.0f, sunPos[2] + sunDir[2] * 6.0f };

        ImVec2 sunScr, endScr;
        if (WorldToScreen(sunPos, viewMat, projMat, cursorPos, viewportAvail, sunScr)) {
            // Draw Sun Disk Icon
            drawList->AddCircleFilled(sunScr, 14.0f, IM_COL32(255, 215, 60, 240));
            drawList->AddCircle(sunScr, 18.0f, IM_COL32(255, 235, 130, 200), 0, 2.0f);

            // Draw Sunlight Direction Vector Ray
            if (WorldToScreen(sunEnd, viewMat, projMat, cursorPos, viewportAvail, endScr)) {
                drawList->AddLine(sunScr, endScr, IM_COL32(255, 230, 100, 220), 2.5f);
                drawList->AddCircleFilled(endScr, 4.0f, IM_COL32(255, 230, 100, 255));
            }
        }
    }

    // 3. Render 3D Geometry Wireframes & Basic Shapes for Scene Graph Nodes
    auto TransformPoint = [](const float local[3], const SceneNode& node, float outWorld[3]) {
        float pitch = node.rotation[0] * 3.14159265f / 180.0f;
        float yaw   = node.rotation[1] * 3.14159265f / 180.0f;
        float roll  = node.rotation[2] * 3.14159265f / 180.0f;

        float cx = std::cos(pitch), sx = std::sin(pitch);
        float cy = std::cos(yaw),   sy = std::sin(yaw);
        float cz = std::cos(roll),  sz = std::sin(roll);

        float r00 = cy * cz + sy * sx * sz;
        float r01 = cx * sz;
        float r02 = -sy * cz + cy * sx * sz;

        float r10 = -cy * sz + sy * sx * cz;
        float r11 = cx * cz;
        float r12 = sy * sz + cy * sx * cz;

        float r20 = sy * cx;
        float r21 = -sx;
        float r22 = cy * cx;

        float sx_l = local[0] * node.scale[0];
        float sy_l = local[1] * node.scale[1];
        float sz_l = local[2] * node.scale[2];

        outWorld[0] = (r00 * sx_l + r01 * sy_l + r02 * sz_l) + node.location[0];
        outWorld[1] = (r10 * sx_l + r11 * sy_l + r12 * sz_l) + node.location[1];
        outWorld[2] = (r20 * sx_l + r21 * sy_l + r22 * sz_l) + node.location[2];
    };

    const std::string& selectedNodeName = EditorState::Get().selectedNodeName;

    auto renderNodeShape = [&](auto& self, const std::vector<SceneNode>& nodes) -> void {
        for (const auto& node : nodes) {
            bool isSelected = (selectedNodeName == node.name);
            ImU32 shapeColor = isSelected ? IM_COL32(255, 215, 0, 240) : IM_COL32(70, 175, 240, 200);
            ImU32 wireColor  = isSelected ? IM_COL32(255, 235, 100, 255) : IM_COL32(140, 200, 255, 220);
            float lineThick  = isSelected ? 2.5f : 1.5f;

            std::string nameLower = node.name;
            std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            bool isPrimitiveMesh = node.meshPath.rfind("primitives/", 0) == 0 ||
                                   node.meshPath == "DefaultCube" || node.meshPath == "cube" || node.meshPath == "Engine/DefaultCube" ||
                                   node.meshPath == "DefaultPlane" || node.meshPath == "plane" || node.meshPath == "Engine/DefaultPlane" ||
                                   node.meshPath == "DefaultSphere" || node.meshPath == "sphere" || node.meshPath == "Engine/DefaultSphere" ||
                                   node.meshPath == "DefaultCylinder" || node.meshPath == "cylinder" || node.meshPath == "Engine/DefaultCylinder" ||
                                   node.meshPath == "DefaultCone" || node.meshPath == "cone" || node.meshPath == "Engine/DefaultCone";

            bool isCustomMesh = !node.meshPath.empty() && !isPrimitiveMesh &&
                                (node.meshPath.find(".zmesh") != std::string::npos || 
                                 node.meshPath.find(".gltf") != std::string::npos || 
                                 node.meshPath.find(".glb") != std::string::npos || 
                                 node.meshPath.find(".obj") != std::string::npos);

            // Shape A: Cube / Box (only for primitive cubes or non-custom primitive actors)
            if (!isCustomMesh && (nameLower.find("cube") != std::string::npos || (node.type == SceneNodeType::Actor && nameLower.find("sphere") == std::string::npos && nameLower.find("cylinder") == std::string::npos && nameLower.find("plane") == std::string::npos && nameLower.find("cone") == std::string::npos))) {
                static float localVerts[8][3] = {
                    {-0.5f, -0.5f, -0.5f}, { 0.5f, -0.5f, -0.5f}, { 0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},
                    {-0.5f, -0.5f,  0.5f}, { 0.5f, -0.5f,  0.5f}, { 0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}
                };
                ImVec2 scr[8];
                bool valid[8];
                for (int v = 0; v < 8; ++v) {
                    float w[3];
                    TransformPoint(localVerts[v], node, w);
                    valid[v] = WorldToScreen(w, viewMat, projMat, cursorPos, viewportAvail, scr[v]);
                }
                // Edges
                static int edges[12][2] = {
                    {0,1},{1,2},{2,3},{3,0},
                    {4,5},{5,6},{6,7},{7,4},
                    {0,4},{1,5},{2,6},{3,7}
                };
                for (int e = 0; e < 12; ++e) {
                    if (valid[edges[e][0]] && valid[edges[e][1]]) {
                        drawList->AddLine(scr[edges[e][0]], scr[edges[e][1]], wireColor, lineThick);
                    }
                }
            }
            // Shape B: Sphere
            else if (nameLower.find("sphere") != std::string::npos) {
                const int numSegs = 16;
                ImVec2 circleXY[numSegs], circleXZ[numSegs];
                bool valXY[numSegs], valXZ[numSegs];

                for (int i = 0; i < numSegs; ++i) {
                    float a = (float)i * 6.2831853f / (float)numSegs;
                    float pXY[3] = { std::cos(a) * 0.5f, std::sin(a) * 0.5f, 0.0f };
                    float pXZ[3] = { std::cos(a) * 0.5f, 0.0f, std::sin(a) * 0.5f };
                    float wXY[3], wXZ[3];
                    TransformPoint(pXY, node, wXY);
                    TransformPoint(pXZ, node, wXZ);
                    valXY[i] = WorldToScreen(wXY, viewMat, projMat, cursorPos, viewportAvail, circleXY[i]);
                    valXZ[i] = WorldToScreen(wXZ, viewMat, projMat, cursorPos, viewportAvail, circleXZ[i]);
                }

                for (int i = 0; i < numSegs; ++i) {
                    int next = (i + 1) % numSegs;
                    if (valXY[i] && valXY[next]) drawList->AddLine(circleXY[i], circleXY[next], wireColor, lineThick);
                    if (valXZ[i] && valXZ[next]) drawList->AddLine(circleXZ[i], circleXZ[next], wireColor, lineThick);
                }
            }
            // Shape C: Cylinder
            else if (nameLower.find("cylinder") != std::string::npos) {
                const int numSegs = 12;
                ImVec2 topC[numSegs], botC[numSegs];
                bool vTop[numSegs], vBot[numSegs];

                for (int i = 0; i < numSegs; ++i) {
                    float a = (float)i * 6.2831853f / (float)numSegs;
                    float pT[3] = { std::cos(a) * 0.5f,  0.5f, std::sin(a) * 0.5f };
                    float pB[3] = { std::cos(a) * 0.5f, -0.5f, std::sin(a) * 0.5f };
                    float wT[3], wB[3];
                    TransformPoint(pT, node, wT);
                    TransformPoint(pB, node, wB);
                    vTop[i] = WorldToScreen(wT, viewMat, projMat, cursorPos, viewportAvail, topC[i]);
                    vBot[i] = WorldToScreen(wB, viewMat, projMat, cursorPos, viewportAvail, botC[i]);
                }

                for (int i = 0; i < numSegs; ++i) {
                    int next = (i + 1) % numSegs;
                    if (vTop[i] && vTop[next]) drawList->AddLine(topC[i], topC[next], wireColor, lineThick);
                    if (vBot[i] && vBot[next]) drawList->AddLine(botC[i], botC[next], wireColor, lineThick);
                    if (vTop[i] && vBot[i] && (i % 3 == 0)) drawList->AddLine(topC[i], botC[i], wireColor, lineThick);
                }
            }
            // Shape D: Plane
            else if (nameLower.find("plane") != std::string::npos) {
                float localP[4][3] = {
                    {-0.5f, 0.0f, -0.5f}, {0.5f, 0.0f, -0.5f}, {0.5f, 0.0f, 0.5f}, {-0.5f, 0.0f, 0.5f}
                };
                ImVec2 sP[4];
                bool vP[4];
                for (int i = 0; i < 4; ++i) {
                    float w[3];
                    TransformPoint(localP[i], node, w);
                    vP[i] = WorldToScreen(w, viewMat, projMat, cursorPos, viewportAvail, sP[i]);
                }
                for (int i = 0; i < 4; ++i) {
                    int next = (i + 1) % 4;
                    if (vP[i] && vP[next]) drawList->AddLine(sP[i], sP[next], wireColor, lineThick);
                }
            }
            // Shape E: Cone
            else if (nameLower.find("cone") != std::string::npos) {
                const int numSegs = 12;
                ImVec2 botC[numSegs], apexS;
                bool vBot[numSegs], vApex;
                float pApex[3] = { 0.0f, 0.5f, 0.0f };
                float wApex[3];
                TransformPoint(pApex, node, wApex);
                vApex = WorldToScreen(wApex, viewMat, projMat, cursorPos, viewportAvail, apexS);

                for (int i = 0; i < numSegs; ++i) {
                    float a = (float)i * 6.2831853f / (float)numSegs;
                    float pB[3] = { std::cos(a) * 0.5f, -0.5f, std::sin(a) * 0.5f };
                    float wB[3];
                    TransformPoint(pB, node, wB);
                    vBot[i] = WorldToScreen(wB, viewMat, projMat, cursorPos, viewportAvail, botC[i]);
                }

                for (int i = 0; i < numSegs; ++i) {
                    int next = (i + 1) % numSegs;
                    if (vBot[i] && vBot[next]) drawList->AddLine(botC[i], botC[next], wireColor, lineThick);
                    if (vApex && vBot[i] && (i % 3 == 0)) drawList->AddLine(apexS, botC[i], wireColor, lineThick);
                }
            }

            // Draw Node Selection Label in 3D Space
            if (isSelected) {
                float center[3] = { 0.0f, 0.0f, 0.0f };
                float wCenter[3];
                TransformPoint(center, node, wCenter);
                ImVec2 sCenter;
                if (WorldToScreen(wCenter, viewMat, projMat, cursorPos, viewportAvail, sCenter)) {
                    drawList->AddCircleFilled(sCenter, 5.0f, IM_COL32(255, 220, 0, 255));
                    char tagBuf[128];
                    snprintf(tagBuf, sizeof(tagBuf), "%s [%s]", node.name.c_str(), SceneGraph::GetTypeName(node.type));
                    drawList->AddText(ImVec2(sCenter.x + 8.0f, sCenter.y - 8.0f), IM_COL32(255, 235, 100, 255), tagBuf);
                }
            }

            if (!node.children.empty()) {
                self(self, node.children);
            }
        }
    };

    renderNodeShape(renderNodeShape, SceneGraph::Get().GetRootNodes());

    // 4. Render 3D Orientation Triad / View Cube Gizmo in Top-Right Corner
    {
        float triadSize = 45.0f;
        ImVec2 triadCenter(cursorPos.x + viewportAvail.x - 65.0f, cursorPos.y + 75.0f);
        drawList->AddCircleFilled(triadCenter, 32.0f, IM_COL32(20, 26, 36, 180));
        drawList->AddCircle(triadCenter, 32.0f, IM_COL32(50, 60, 80, 220), 0, 1.5f);

        // Project X, Y, Z axis vectors from camera view matrix
        Vec3f xAxis(viewMat[0], viewMat[4], viewMat[8]);
        Vec3f yAxis(viewMat[1], viewMat[5], viewMat[9]);
        Vec3f zAxis(viewMat[2], viewMat[6], viewMat[10]);

        ImVec2 sX(triadCenter.x + xAxis.x * triadSize, triadCenter.y - xAxis.y * triadSize);
        ImVec2 sY(triadCenter.x + yAxis.x * triadSize, triadCenter.y - yAxis.y * triadSize);
        ImVec2 sZ(triadCenter.x + zAxis.x * triadSize, triadCenter.y - zAxis.y * triadSize);

        // Draw triad axes
        drawList->AddLine(triadCenter, sX, IM_COL32(235, 65, 65, 255), 2.5f);
        drawList->AddText(ImVec2(sX.x + 2.0f, sX.y - 6.0f), IM_COL32(255, 100, 100, 255), "X");

        drawList->AddLine(triadCenter, sY, IM_COL32(65, 220, 90, 255), 2.5f);
        drawList->AddText(ImVec2(sY.x + 2.0f, sY.y - 6.0f), IM_COL32(100, 255, 120, 255), "Y");

        drawList->AddLine(triadCenter, sZ, IM_COL32(65, 135, 245, 255), 2.5f);
        drawList->AddText(ImVec2(sZ.x + 2.0f, sZ.y - 6.0f), IM_COL32(100, 170, 255, 255), "Z");
    }

    // 5. Render Isolation Mode Active Banner
    if (EditorState::Get().isIsolationMode) {
        float bannerW = 320.0f;
        float bannerH = 28.0f;
        ImVec2 bannerMin(cursorPos.x + (viewportAvail.x - bannerW) * 0.5f, cursorPos.y + 40.0f);
        ImVec2 bannerMax(bannerMin.x + bannerW, bannerMin.y + bannerH);

        drawList->AddRectFilled(bannerMin, bannerMax, IM_COL32(215, 140, 20, 235), 4.0f);
        drawList->AddRect(bannerMin, bannerMax, IM_COL32(255, 200, 60, 255), 4.0f, 0, 1.5f);
        drawList->AddText(ImVec2(bannerMin.x + 18.0f, bannerMin.y + 6.0f), IM_COL32(255, 255, 255, 255), "[ISOLATION MODE ACTIVE - Alt+H to Exit]");
    }

    // 6. Render 3D Measurement Ruler Tool Overlay
    if (EditorState::Get().isMeasurementToolActive) {
        // Measurement rendering handled by ViewportMeasurement
    }
}

} // namespace EngineEditor::Panels
