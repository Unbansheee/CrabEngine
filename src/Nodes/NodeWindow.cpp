
module;
#include <iostream>
#include <glfw3webgpu/glfw3webgpu.h>
//#include <glm/ext/matrix_clip_space.hpp>
//#include <webgpu/webgpu.hpp>

#include "GLFW/glfw3.h"

module Engine.Node.Window;
import Engine.Reflection.ClassDB;
import Engine.Node;
import Engine.Application;
import Engine.GFX.View;




using namespace wgpu;



void NodeWindow::EnterTree()
{
    if (GetTree()->IsInEditor()) return;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(1280, 960, Name.c_str(), nullptr, nullptr);
    std::cout << "Window: " << window << std::endl;
    if (!window) {
        std::cerr << "Could not create window!" << std::endl;
        return;
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    auto& app = Application::Get();
    auto instance = app.GetInstance();
    surface = wgpu::raii::Surface(glfwGetWGPUSurface(instance, window));
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.compatibleSurface = *surface;
    wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
#ifdef WEBGPU_BACKEND_WGPU
    SurfaceCapabilities capabilities;
    surface->getCapabilities(adapter, &capabilities);
    surfaceFormat = capabilities.formats[0];
#else
    surfaceFormat = TextureFormat::BGRA8Unorm;
#endif
    
    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int, int){
    auto that = reinterpret_cast<NodeWindow*>(glfwGetWindowUserPointer(window));
    if (that != nullptr) that->RequestResize();
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
        auto that = reinterpret_cast<NodeWindow*>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->OnMouseButton(button, action, mods);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
        auto that = reinterpret_cast<NodeWindow*>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->OnMouseMove(xpos, ypos);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
        auto that = reinterpret_cast<NodeWindow*>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->OnScroll(xoffset, yoffset);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto that = reinterpret_cast<NodeWindow*>(glfwGetWindowUserPointer(window));
        if (that != nullptr) that->OnKey(key, scancode, action, mods);
    });

    auto size = GetWindowSize();
    CreateSwapChain(size.x, size.y);
    CreateDepthTexture(size.x, size.y);
    CreateIDPassTextures(size.x, size.y);
    InitializeRenderer();
}

void NodeWindow::Update(float dt)
{
    Node::Update(dt);
    if (GetTree()->IsInEditor()) return;

    glfwMakeContextCurrent(window);
    glfwPollEvents();
    
    auto DepthView = GetDepthTextureView();
    auto SurfaceView = GetCurrentTextureView();
    
    if (bShouldRenderNodesToSurface)
    {
        View viewData;
        if (ActiveCamera)
        {
            viewData.Position = ActiveCamera->GetGlobalPosition();
            viewData.ViewMatrix = ActiveCamera->GetViewMatrix();
            viewData.ProjectionMatrix = glm::perspectiveRH(ActiveCamera->FOV, GetAspectRatio(), ActiveCamera->NearClippingPlane, ActiveCamera->FarClippingPlane);
        }
        if (SurfaceView) {
            renderer.RenderNodeTree(this, viewData, *SurfaceView, *DepthView, PickingPassTexture);
        }
    }
    renderer.Flush();
    
    surface->present();
    GetNextSurfaceTextureView();

    ExecuteResize();
    
    if (glfwWindowShouldClose(window))
    {
        RemoveFromParent();
        bCloseRequested = true;
    }
}


void NodeWindow::ExitTree()
{
    TerminateSurface();
    glfwDestroyWindow(window);
    window = nullptr;
}


float NodeWindow::GetAspectRatio() const
{
    return GetWindowSize().x / GetWindowSize().y;
}

void NodeWindow::SetMouseVisible(bool visible) {
    glfwSetInputMode(window, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

bool NodeWindow::IsMouseVisible() const {
    int mode = glfwGetInputMode(window, GLFW_CURSOR);
    return mode != GLFW_CURSOR_DISABLED;
}

void NodeWindow::SetMouseCursor(int shape) {
    static std::unordered_map<int, GLFWcursor*> cursorCache;

    if (cursorCache.find(shape) == cursorCache.end())
    {
        GLFWcursor* cursor = glfwCreateStandardCursor(shape);
        cursorCache[shape] = cursor;
    }

    glfwSetCursor(window, cursorCache[shape]);
}

void NodeWindow::SetMousePosition(double x, double y) {
    glfwSetCursorPos(window, x, y);
}

Vector2 NodeWindow::GetMousePosition() const {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    return { xpos, ypos };
}

void NodeWindow::InitializeRenderer()
{
    auto& app = Application::Get();
    renderer.Initialize(app.GetDevice());
}


void NodeWindow::TerminateSurface()
{
    if (m_surfaceTexture.texture) wgpuTextureRelease(m_surfaceTexture.texture);
    surface->unconfigure();
    surface = {};
}


void NodeWindow::RequestResize()
{
    auto size = GetWindowSize();
    if (size.x > 0 && size.y > 0)
    {
        resizeRequest.active = true;
        resizeRequest.width = size.x;
        resizeRequest.height = size.y;
    }

}

void NodeWindow::OnKey(int key, int scancode, int action, int mods) {
    InputEvent event;
    event.type = InputEvent::Type::Key;
    event.key = { key, scancode, action, mods };
    PropagateInputToChildren(this, event);
}

void NodeWindow::OnScroll(double xoffset, double yoffset) {
    InputEvent event;
    event.type = InputEvent::Type::Scroll;
    event.scroll = { xoffset, yoffset };
    PropagateInputToChildren(this, event);
}

void NodeWindow::OnMouseMove(double xpos, double ypos) {
    InputEvent event;
    event.type = InputEvent::Type::MouseMove;
    event.mouseMove = { xpos, ypos };
    PropagateInputToChildren(this, event);
}

void NodeWindow::OnMouseButton(int button, int action, int mods) {
    InputEvent event;
    event.type = InputEvent::Type::MouseButton;
    event.mouseButton = { button, action, mods };
    PropagateInputToChildren(this, event);
}

InputResult NodeWindow::PropagateInputToChildren(Node* parent, const InputEvent& event) {
    for (auto it = parent->Children.rbegin(); it != parent->Children.rend(); ++it)
    {
        Node* child = it->get(); // or use your ObjectRef<Node> if applicable
        if (child)
        {
            if (auto script = child->GetScriptInstance()) {
                InputResult result = *script->Call<InputResult>(L"HandleInput", static_cast<InputEventInterop>(event));
                if (result == InputResult::Handled)
                    return InputResult::Handled;
            }
            else {
                InputResult result = child->HandleInput(event);
                if (result == InputResult::Handled)
                    return InputResult::Handled;
            }

            if (PropagateInputToChildren(child, event) == InputResult::Handled)
                return InputResult::Handled;
        }
    }
    return InputResult::Ignored;
}

wgpu::raii::TextureView NodeWindow::GetCurrentTextureView() const
{
    wgpu::Texture texture = m_surfaceTexture.texture;
    if (!texture) return {};

    // Create a view for this surface texture
    wgpu::TextureViewDescriptor viewDescriptor;
    viewDescriptor.label = {"Surface texture view", wgpu::STRLEN};
    viewDescriptor.format = texture.getFormat();
    viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = wgpu::TextureAspect::All;
    wgpu::raii::TextureView targetView = texture.createView(viewDescriptor);

#ifndef WEBGPU_BACKEND_WGPU
    // We no longer need the texture, only its view
    // (NB: with wgpu-native, surface textures must not be manually released)
    wgpuTextureRelease(m_surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU

    return targetView;
}

wgpu::raii::TextureView NodeWindow::GetNextSurfaceTextureView() const
{
    // Get the surface texture
    if (m_surfaceTexture.texture) {
        wgpuTextureRelease(m_surfaceTexture.texture);
    }

    wgpuSurfaceGetCurrentTexture(*surface, &m_surfaceTexture);
    
    if (m_surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessOptimal && m_surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::SuccessSuboptimal) {
        return {};
    }
    return GetCurrentTextureView();
}

wgpu::raii::TextureView NodeWindow::GetDepthTextureView() const
{
    // Depth attachment
    wgpu::TextureViewDescriptor depthTextureViewDesc;
    depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
    depthTextureViewDesc.baseArrayLayer = 0;
    depthTextureViewDesc.arrayLayerCount = 1;
    depthTextureViewDesc.baseMipLevel = 0;
    depthTextureViewDesc.mipLevelCount = 1;
    depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    depthTextureViewDesc.format = depthFormat;
    WGPUTextureView depth_view = wgpuTextureCreateView(*m_depthTexture, &depthTextureViewDesc);
    return wgpu::raii::TextureView{depth_view};
}

wgpu::SurfaceTexture NodeWindow::GetSurfaceTexture() const
{
    return m_surfaceTexture;
}

Vector2 NodeWindow::GetWindowSize() const
{
    int x, y;
    glfwGetWindowSize(window, &x, &y);
    return {x, y};
}

void NodeWindow::CreateSwapChain(uint32_t width, uint32_t height)
{
    if (m_surfaceTexture.texture)
    {
        wgpuTextureRelease(m_surfaceTexture.texture);
        m_surfaceTexture.texture = nullptr;
    }
    
    m_currentSurfaceView = {};
    
    wgpu::SurfaceConfiguration desc;
    desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc | wgpu::TextureUsage::TextureBinding;
    desc.format = surfaceFormat;
    desc.width = width;
    desc.height = height;
    desc.presentMode = wgpu::PresentMode::Immediate;
    desc.viewFormats = nullptr;
    desc.viewFormatCount = 0;
    desc.device = Application::Get().GetDevice();
    surface->configure(desc);
}

void NodeWindow::CreateDepthTexture(uint32_t width, uint32_t height)
{
    wgpu::TextureDescriptor desc;
    desc.size = {width, height, 1};
    desc.mipLevelCount = 1;
    desc.sampleCount = 1;
    desc.dimension = wgpu::TextureDimension::_2D;
    desc.format = depthFormat;
    desc.usage = wgpu::TextureUsage::RenderAttachment;
    desc.viewFormatCount = 1;
    desc.viewFormats = (WGPUTextureFormat*)&depthFormat;
       
    m_depthTexture = Application::Get().GetDevice().createTexture(desc);
}

void NodeWindow::ExecuteResize()
{
    if (resizeRequest.active)
    {
        CreateSwapChain(resizeRequest.width, resizeRequest.height);
        CreateDepthTexture(resizeRequest.width, resizeRequest.height);

        resizeRequest = {};
    }
}

void NodeWindow::CreateIDPassTextures(uint32_t width, uint32_t height)
{
    if (!PickingPassTexture)
    {
        PickingPassTexture = std::make_shared<RuntimeTextureResource>();
    }
    wgpu::TextureDescriptor IDPassDesc = wgpu::Default;
    IDPassDesc.size.width = width;
    IDPassDesc.size.height = height;
    IDPassDesc.size.depthOrArrayLayers = 1;
    IDPassDesc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc;
    IDPassDesc.format = wgpu::TextureFormat::R32Uint;
    IDPassDesc.mipLevelCount = 1;
    IDPassDesc.sampleCount = 1;
    IDPassDesc.dimension = wgpu::TextureDimension::_2D;

    PickingPassTexture->CreateBlankTexture(IDPassDesc);
}