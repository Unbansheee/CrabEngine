#pragma once
#include "Node.h"
#include "Renderer/Renderer.h"

class NodeWindow : public Node
{
public:
    NodeWindow() : Node() {};
    NodeWindow(const std::string& name) : Node(name) {}
    
    void Begin() override;
    void Update(float dt) override;
    bool WantsToClose() const { return bCloseRequested; }
    ~NodeWindow() override;

    wgpu::TextureFormat GetSurfaceFormat() const { return surfaceFormat; }

protected:
    void InitializeRenderer();
    void ClearWindow(const glm::vec4& color);
    void TerminateSurface();
    
    virtual void OnResize();
    virtual void OnKey(int key, int scancode, int action, int mods)
    {}
    virtual void OnScroll(double xoffset, double yoffset)
    {}
    virtual void OnMouseMove(double xpos, double ypos)
    {}
    virtual void OnMouseButton(int button, int action, int mods)
    {}

    Vector2 GetWindowSize() const;
protected:
    Renderer renderer;
    glm::vec4 clearColor = glm::vec4(1.f, 0.1f, 0.1f, 1.0f);
    struct GLFWwindow* window = nullptr;
    wgpu::Surface surface = nullptr;
    wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
    bool bCloseRequested = false;

};
