#ifndef VIEWPORT_RENDERER_H
#define VIEWPORT_RENDERER_H

#include <stdint.h>
#include <glad/glad.h>

namespace EngineEditor {

class ViewportRenderer {
public:
    static ViewportRenderer& Get();

    ViewportRenderer();
    ~ViewportRenderer();

    void Init();
    void Shutdown();
    void Resize(uint32_t width, uint32_t height);
    void RenderScene(float deltaTime);

    uint64_t GetTextureID() const { return static_cast<uint64_t>(m_ColorTexture); }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

private:
    void CreateFramebuffer(uint32_t width, uint32_t height);
    void DeleteFramebuffer();

    GLuint m_FBO = 0;
    GLuint m_ColorTexture = 0;
    GLuint m_DepthRBO = 0;
    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    float m_RotationAngle = 0.0f;
};

} // namespace EngineEditor

#endif // VIEWPORT_RENDERER_H
