#include "NodeImGUIContextWindow.h"

#include <glfw3webgpu.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_wgpu.h"
#include "ImGuizmo/ImGuizmo.h"

REGISTER_CLASS(NodeImGUIContextWindow)


void NodeImGUIContextWindow::EnterTree()
{
    NodeWindow::EnterTree();
    imguiContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(imguiContext);
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.UserData = this;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    ImGui_ImplGlfw_InitForOther(window, true);

    auto& app = Application::Get();
    
    ImGui_ImplWGPU_InitInfo info;
    info.Device = app.GetDevice();
    info.RenderTargetFormat = surfaceFormat;
    info.DepthStencilFormat = wgpu::TextureFormat::Undefined;
    info.NumFramesInFlight = 3;
    info.ViewportPresentMode = wgpu::PresentMode::Fifo;
    info.CreateViewportWindowFn = [](ImGuiViewport* viewport) {
        auto that = reinterpret_cast<NodeWindow*>(ImGui::GetIO().UserData);
        auto* window = static_cast<GLFWwindow*>(viewport->PlatformHandle);
        
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        std::cout << "Creating window for viewport " << viewport->ID << " at " << x << ", " << y << std::endl;

        return glfwGetWGPUSurface(Application::Get().GetInstance(), window);
    };
    ImGui_ImplWGPU_Init(&info);
}

void NodeImGUIContextWindow::Update(float dt)
{
    WGPURenderPassColorAttachment color_attachments = {};
    color_attachments.loadOp = WGPULoadOp_Load;
    color_attachments.storeOp = WGPUStoreOp_Store;
    color_attachments.clearValue = { 0, 0, 0, 0 };
    color_attachments.view = GetCurrentTextureView();
	
    WGPURenderPassDescriptor render_pass_desc = {};
    render_pass_desc.colorAttachmentCount = 1;
    render_pass_desc.colorAttachments = &color_attachments;
    render_pass_desc.depthStencilAttachment = nullptr;
    
    WGPUCommandEncoderDescriptor enc_desc = {};
    wgpu::CommandEncoder gui_encoder = Application::Get().GetDevice().createCommandEncoder(enc_desc);
    wgpu::RenderPassEncoder pass = gui_encoder.beginRenderPass(render_pass_desc);
    
    auto& io = ImGui::GetIO();
    ImGui_ImplWGPU_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();

    DrawGUI();

    // Depth first walk
    for (const auto& i : Children)
    {
        if (!i) continue;
        i->DrawGUIInternal();
    }
    
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), pass);

    pass.end();
    wgpu::CommandBufferDescriptor cmd_buffer_desc = {};
    cmd_buffer_desc.label = "ImGUI Draw Command Buffer";
    auto cmd = gui_encoder.finish(cmd_buffer_desc);

    renderer.AddCommand(cmd);

    pass.release();
    gui_encoder.release();

    NodeWindow::Update(dt);
    
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void NodeImGUIContextWindow::DrawGUI()
{
    NodeWindow::DrawGUI();
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockSpaceOverViewport(0, ImGui::FindViewportByPlatformHandle(window), dockspace_flags);
}

