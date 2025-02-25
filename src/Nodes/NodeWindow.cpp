#include "NodeWindow.h"

#include <glfw3webgpu.h>

#include "GLFW/glfw3.h"
#include "Application.h"

using namespace wgpu;

void NodeWindow::Begin()
{
    Node::Begin();
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
    if (that != nullptr) that->OnResize();
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
    
    InitializeRenderer();
}

void NodeWindow::Update(float dt)
{
    Node::Update(dt);
    glfwMakeContextCurrent(window);
    glfwPollEvents();

    Camera cam;
    renderer.Render(this, cam);
    
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

void NodeWindow::InitializeRenderer()
{
    auto size = GetWindowSize();
    auto& app = Application::Get();
    renderer.Initialize(app.GetDevice(), surface, (int)size.x, (int)size.y, surfaceFormat, WGPUTextureFormat_Depth24Plus);
}

void NodeWindow::ClearWindow(const glm::vec4& color)
{
    auto& app = Application::Get();
    
    CommandEncoderDescriptor encoderDesc = {};
    encoderDesc.label = "Window Command Encoder";
    CommandEncoder encoder = app.GetDevice().createCommandEncoder(encoderDesc);

    auto v = renderer.GetCurrentTextureView();
    
    RenderPassColorAttachment colorAttachment = {};
    colorAttachment.view = v;
    colorAttachment.resolveTarget = nullptr;
    colorAttachment.loadOp = LoadOp::Clear;
    colorAttachment.storeOp = StoreOp::Store;
    colorAttachment.clearValue = WGPUColor{ clearColor.x, clearColor.y,  clearColor.z, clearColor.a };

#ifndef WEBGPU_BACKEND_WGPU
    renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU
    
    RenderPassDescriptor renderPassDesc = {};
    renderPassDesc.nextInChain = nullptr;
    renderPassDesc.colorAttachments = &colorAttachment;
    renderPassDesc.colorAttachmentCount = 1;
    renderPassDesc.timestampWrites = nullptr;
    renderPassDesc.depthStencilAttachment = nullptr;
    
    RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);
    renderPass.end();
    renderPass.release();

    auto c = encoder.finish();
    app.GetQueue().submit(1, &c);
    encoder.release();
    c.release();
    v.release();
    
    surface.present();
}


void NodeWindow::TerminateSurface()
{
    surface.unconfigure();
    surface.release();
}


void NodeWindow::OnResize()
{
    auto size = GetWindowSize();
    renderer.RequestResize((uint32_t)size.x, (uint32_t)size.y);
}

Vector2 NodeWindow::GetWindowSize() const
{
    int x, y;
    glfwGetWindowSize(window, &x, &y);
    return {x, y};
}

