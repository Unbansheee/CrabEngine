#include "NodeWindow.h"

#include <glfw3webgpu.h>

#include "GLFW/glfw3.h"
#include "Application.h"

using namespace wgpu;

void NodeWindow::Begin()
{
    Node::Begin();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(640, 480, Name.c_str(), nullptr, nullptr);
    std::cout << "Window: " << window << std::endl;
    if (!window) {
        std::cerr << "Could not create window!" << std::endl;
        return;
    }
    glfwMakeContextCurrent(window);

    glfwSetWindowUserPointer(window, this);
    auto& app = Application::Get();
    auto instance = app.GetInstance();
    surface = glfwGetWGPUSurface(instance, window);
    wgpu::RequestAdapterOptions adapterOpts = {};
    adapterOpts.compatibleSurface = surface;
    wgpu::Adapter adapter = instance.requestAdapter(adapterOpts);
#ifdef WEBGPU_BACKEND_WGPU
    surfaceFormat = surface.getPreferredFormat(adapter);
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
    InitializeRenderer();
}

void NodeWindow::Update(float dt)
{
    Node::Update(dt);
    glfwMakeContextCurrent(window);
    glfwPollEvents();
    
    auto DepthView = GetDepthTextureView();
    auto SurfaceView = GetCurrentTextureView();
    
    Camera cam;
    cam.ProjectionMatrix = glm::perspective(45 * PI / 180, GetAspectRatio(), 0.01f, 1000.0f);
    cam.ViewMatrix = glm::lookAt(glm::vec3(4.0f, 0.f, 0.0f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    cam.Position = Vector3(4, 0, 0);
    
    if (bShouldRenderNodesToSurface)
    {
        renderer.RenderNodeTree(this, cam, SurfaceView, DepthView);
    }
    renderer.Flush();
    
    surface.present();

    DepthView.release();
    SurfaceView.release();
    m_currentSurfaceView.release();
    m_currentSurfaceView = nullptr;
    
    ExecuteResize();
    
    if (glfwWindowShouldClose(window))
    {
        RemoveFromParent();
        bCloseRequested = true;
    }
}


NodeWindow::~NodeWindow()
{
    glfwDestroyWindow(window);
}

float NodeWindow::GetAspectRatio() const
{
    return GetWindowSize().x / GetWindowSize().y;
}

void NodeWindow::InitializeRenderer()
{
    auto& app = Application::Get();
    renderer.Initialize(app.GetDevice());
}


void NodeWindow::TerminateSurface()
{
    surface.unconfigure();
    surface.release();
}


void NodeWindow::RequestResize()
{
    auto size = GetWindowSize();
    resizeRequest.active = true;
    resizeRequest.width = size.x;
    resizeRequest.height = size.y;
}

wgpu::TextureView NodeWindow::GetCurrentTextureView()
{
    if (m_currentSurfaceView == nullptr)
    {
        m_currentSurfaceView = GetNextSurfaceTextureView();
    }
    return m_currentSurfaceView;
}

wgpu::TextureView NodeWindow::GetNextSurfaceTextureView() const
{
    // Get the surface texture
    wgpuSurfaceGetCurrentTexture(surface, &m_surfaceTexture);
    
    if (m_surfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) {
        return nullptr;
    }
    wgpu::Texture texture = m_surfaceTexture.texture;
    
    // Create a view for this surface texture
    wgpu::TextureViewDescriptor viewDescriptor;
    viewDescriptor.label = "Surface texture view";
    viewDescriptor.format = texture.getFormat();
    viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
    viewDescriptor.baseMipLevel = 0;
    viewDescriptor.mipLevelCount = 1;
    viewDescriptor.baseArrayLayer = 0;
    viewDescriptor.arrayLayerCount = 1;
    viewDescriptor.aspect = wgpu::TextureAspect::All;
    wgpu::TextureView targetView = texture.createView(viewDescriptor);
#ifndef WEBGPU_BACKEND_WGPU
    // We no longer need the texture, only its view
    // (NB: with wgpu-native, surface textures must not be manually released)
    wgpuTextureRelease(surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU

    return targetView;
}

wgpu::TextureView NodeWindow::GetDepthTextureView() const
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
    WGPUTextureView depth_view = wgpuTextureCreateView(m_depthTexture, &depthTextureViewDesc);
    return depth_view;
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
    if (m_currentSurfaceView) m_currentSurfaceView.release();
    m_currentSurfaceView = nullptr;
    wgpu::SurfaceConfiguration desc;
    desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
    desc.format = surfaceFormat;
    desc.width = width;
    desc.height = height;
    desc.presentMode = wgpu::PresentMode::Fifo;
    desc.viewFormats = nullptr;
    desc.viewFormatCount = 0;
    desc.device = Application::Get().GetDevice();
    surface.configure(desc);
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
        m_depthTexture.destroy();
        m_depthTexture.release();
        CreateSwapChain(resizeRequest.width, resizeRequest.height);
        CreateDepthTexture(resizeRequest.width, resizeRequest.height);

        resizeRequest = {};
    }
}

