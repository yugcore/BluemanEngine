#include "ViewportRenderer.h"
#include <iostream>
#include <cmath>

namespace EngineEditor {

ViewportRenderer& ViewportRenderer::Get() {
    static ViewportRenderer instance;
    return instance;
}

ViewportRenderer::ViewportRenderer() {
}

ViewportRenderer::~ViewportRenderer() {
    Shutdown();
}

void ViewportRenderer::Init() {
    Resize(1280, 720);
}

void ViewportRenderer::Shutdown() {
    DeleteFramebuffer();
}

void ViewportRenderer::DeleteFramebuffer() {
    if (m_FBO) {
        glDeleteFramebuffers(1, &m_FBO);
        m_FBO = 0;
    }
    if (m_ColorTexture) {
        glDeleteTextures(1, &m_ColorTexture);
        m_ColorTexture = 0;
    }
    if (m_DepthRBO) {
        glDeleteRenderbuffers(1, &m_DepthRBO);
        m_DepthRBO = 0;
    }
}

void ViewportRenderer::CreateFramebuffer(uint32_t width, uint32_t height) {
    DeleteFramebuffer();

    m_Width = width;
    m_Height = height;

    if (m_Width == 0 || m_Height == 0) return;

    // Create FBO
    glGenFramebuffers(1, &m_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    // Color Texture Attachment
    glGenTextures(1, &m_ColorTexture);
    glBindTexture(GL_TEXTURE_2D, m_ColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ColorTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewportRenderer::Resize(uint32_t width, uint32_t height) {
    if (width == m_Width && height == m_Height) return;
    if (width == 0 || height == 0) return;
    CreateFramebuffer(width, height);
}

void ViewportRenderer::RenderScene(float deltaTime) {
    if (m_FBO == 0 || m_Width == 0 || m_Height == 0) return;

    m_RotationAngle += deltaTime * 45.0f;
    if (m_RotationAngle > 360.0f) m_RotationAngle -= 360.0f;

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glViewport(0, 0, m_Width, m_Height);

    // Animate background gradient clear color to show live scene rendering
    float pulse = (std::sin(m_RotationAngle * 0.05f) + 1.0f) * 0.5f;
    float r = 0.10f + pulse * 0.04f;
    float g = 0.12f + pulse * 0.05f;
    float b = 0.16f + pulse * 0.06f;

    glClearColor(r, g, b, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace EngineEditor
