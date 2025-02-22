// Include the C++ wrapper instead of the raw header(s)
#include <webgpu/webgpu.hpp>

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>
#include <filesystem>
#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

#include "Application.h"

#include <iostream>
#include <cassert>
#include <fstream>
#include <vector>

#include "ResourceManager.h"

#include <glm/ext.hpp>

#include "imgui_internal.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_wgpu.h"


constexpr float PI = 3.14159265358979323846f;
using namespace wgpu;
using glm::vec3;

constexpr float deg_to_rad(float deg) {
	return deg * PI / 180.0f;
}

// We define a function that hides implementation-specific variants of device polling:
void wgpuPollEvents([[maybe_unused]] Device device, [[maybe_unused]] bool yieldToWebBrowser) {
#if defined(WEBGPU_BACKEND_DAWN)
	device.tick();
#elif defined(WEBGPU_BACKEND_WGPU)
	wgpuDevicePoll(device, false, nullptr);
#elif defined(WEBGPU_BACKEND_EMSCRIPTEN)
	if (yieldToWebBrowser) {
		emscripten_sleep(100);
	}
#endif
}

uint32_t ceilToNextMultiple(uint32_t value, uint32_t step) {
	uint32_t divide_and_ceil = value / step + (value % step == 0 ? 0 : 1);
	return step * divide_and_ceil;
}


bool Application::Initialize() {

	if (!InitializeWindowAndDevice()) return false;
	if (!InitializeSurface()) return false;
	if (!InitializeRenderPipeline()) return false;
	if (!InitializeDepthBuffer()) return false;
	if (!InitializeGeometry()) return false;
	if (!InitializeUniformBuffer()) return false;
	if (!InitializeTextures()) return false;
	if (!InitializeBindGroups()) return false;
	if (!InitializeGUI()) return false;
	wgpuPollEvents(device, false);

	return true;
}

void Application::Terminate() {
	TerminateGUI();
	TerminateTextures();
	TerminateDepthBuffer();
	TerminateBindGroups();

	TerminateUniformBuffer();
	TerminateRenderPipeline();
	TerminateSurface();
	TerminateWindowAndDevice();
}

void Application::MainLoop() {
	glfwPollEvents();

	UpdateDragInertia();

	MyUniforms t{};
	t.time = static_cast<float>(glfwGetTime());
	queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, time), &t.time, sizeof(MyUniforms::time));

	// Get the next target texture view
	TextureView targetView = GetNextSurfaceTextureView();

	// Create a command encoder for the draw call
	CommandEncoderDescriptor encoderDesc = {};
	encoderDesc.label = "My command encoder";
	CommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, &encoderDesc);

	// Create the render pass that clears the screen with our color
	RenderPassDescriptor renderPassDesc = {};

	// The attachment part of the render pass descriptor describes the target texture of the pass
	RenderPassColorAttachment renderPassColorAttachment = {};
	renderPassColorAttachment.view = targetView;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = LoadOp::Clear;
	renderPassColorAttachment.storeOp = StoreOp::Store;
	renderPassColorAttachment.clearValue = WGPUColor{ 0.02, 0.02, 0.02, 1.0 };
#ifndef WEBGPU_BACKEND_WGPU
	renderPassColorAttachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
#endif // NOT WEBGPU_BACKEND_WGPU

	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;

	RenderPassDepthStencilAttachment depthStencilAttachment;
	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
	depthStencilAttachment.view = depthTextureView;
	depthStencilAttachment.depthClearValue = 1.0f;
	// Operation settings comparable to the color attachment
	depthStencilAttachment.depthLoadOp = LoadOp::Clear;
	depthStencilAttachment.depthStoreOp = StoreOp::Store;
	// we could turn off writing to the depth buffer globally here
	depthStencilAttachment.depthReadOnly = false;

	// Stencil setup, mandatory but unused
	depthStencilAttachment.stencilClearValue = 0;
	depthStencilAttachment.stencilLoadOp = LoadOp::Clear;
	depthStencilAttachment.stencilStoreOp = StoreOp::Store;
	depthStencilAttachment.stencilReadOnly = true;

	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
	renderPassDesc.timestampWrites = nullptr;

	// Create the render pass and end it immediately (we only clear the screen but do not draw anything)
	RenderPassEncoder renderPass = encoder.beginRenderPass(renderPassDesc);

	// Select which render pipeline to use
	renderPass.setPipeline(pipeline);
	renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
	//renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0, indexBuffer.getSize());
	// Draw 1 instance of a 3-vertices shape


	renderPass.setBindGroup(0, bindGroup, 0, nullptr);
	renderPass.draw(vertexCount, 1, 0, 0);

	/*
	dynamicOffset = 1 * uniformStride;
	renderPass.setBindGroup(0, bindGroup, 1, &dynamicOffset);
	renderPass.drawIndexed(indexCount, 1, 0, 0, 0);
	*/

	UpdateGUI(renderPass);

	renderPass.end();
	renderPass.release();
	// Finally encode and submit the render pass
	CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.label = "Command buffer";
	CommandBuffer command = encoder.finish(cmdBufferDescriptor);

	queue.submit(1, &command);

	command.release();
	encoder.release();
	targetView.release();


#ifndef __EMSCRIPTEN__
	surface.present();
#endif

	wgpuPollEvents(device, true);
}

bool Application::IsRunning() {
	return !glfwWindowShouldClose(window);
}

void Application::OnResize() {
	TerminateDepthBuffer();

	InitializeSurface();
	InitializeDepthBuffer();

	UpdateProjectionMatrix();
}

bool Application::InitializeRenderPipeline() {

	std::cout << "Creating shader module..." << std::endl;
	ShaderModule shaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/shader.wgsl", device);
	std::cout << "Shader module: " << shaderModule << std::endl;

	// Check for errors
	if (shaderModule == nullptr) {
		std::cerr << "Could not load shader!" << std::endl;
		return false;
	}

	std::vector<VertexAttribute> vertexAttributes(4);
	vertexAttributes[0].shaderLocation = 0;
	vertexAttributes[0].format = VertexFormat::Float32x3;
	vertexAttributes[0].offset = 0;

	vertexAttributes[1].shaderLocation = 1;
	vertexAttributes[1].format = VertexFormat::Float32x3;
	vertexAttributes[1].offset = offsetof(VertexData, normal);

	vertexAttributes[2].shaderLocation = 2;
	vertexAttributes[2].format = VertexFormat::Float32x3;
	vertexAttributes[2].offset = offsetof(VertexData, color);

	vertexAttributes[3].shaderLocation = 3;
	vertexAttributes[3].format = VertexFormat::Float32x2;
	vertexAttributes[3].offset = offsetof(VertexData, uv);

	VertexBufferLayout vertexBufferLayout;
	vertexBufferLayout.attributeCount = vertexAttributes.size();
	vertexBufferLayout.attributes = vertexAttributes.data();
	vertexBufferLayout.arrayStride = sizeof(VertexData);
	vertexBufferLayout.stepMode = VertexStepMode::Vertex;

	RenderPipelineDescriptor pipelineDesc = {};
	pipelineDesc.vertex.bufferCount = 1;
	pipelineDesc.vertex.buffers = &vertexBufferLayout;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;
	pipelineDesc.vertex.module = shaderModule;
	pipelineDesc.primitive.topology = PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = IndexFormat::Undefined;
	pipelineDesc.primitive.frontFace = FrontFace::CCW;
	pipelineDesc.primitive.cullMode = CullMode::None;
	pipelineDesc.multisample.count = 1;
	pipelineDesc.multisample.mask = ~0u;
	pipelineDesc.multisample.alphaToCoverageEnabled = false;
	pipelineDesc.layout = layout;

	DepthStencilState depthStencilState = Default;
	depthStencilState.depthCompare = CompareFunction::Less;
	depthStencilState.depthWriteEnabled = true;
	depthTextureFormat = TextureFormat::Depth24Plus;
	depthStencilState.format = depthTextureFormat;
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

	pipelineDesc.depthStencil = &depthStencilState;


	FragmentState fragmentState = {};
	fragmentState.module = shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	pipelineDesc.fragment = &fragmentState;
	pipelineDesc.depthStencil = &depthStencilState;

	BlendState blendState;
	blendState.color.srcFactor = BlendFactor::SrcAlpha;
	blendState.color.dstFactor = BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = BlendOperation::Add;
	blendState.alpha.srcFactor = BlendFactor::Zero;
	blendState.alpha.dstFactor = BlendFactor::One;
	blendState.alpha.operation = BlendOperation::Add;

	ColorTargetState colorTarget;
	colorTarget.format = surfaceFormat;
	colorTarget.blend = &blendState;
	colorTarget.writeMask = ColorWriteMask::All; // We could write to only some of the color channels.

	// We have only one target because our render pass has only one output color
	// attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTarget;
	pipelineDesc.fragment = &fragmentState;

	// Samples per pixel
	pipelineDesc.multisample.count = 1;

	// Default value for the mask, meaning "all bits on"
	pipelineDesc.multisample.mask = ~0u;

	// Default value as well (irrelevant for count = 1 anyways)
	pipelineDesc.multisample.alphaToCoverageEnabled = false;

	// Define binding layout (don't forget to = Default)
	std::vector<BindGroupLayoutEntry> bindingLayoutEntries(3, Default);

	BindGroupLayoutEntry& bindingLayout = bindingLayoutEntries[0];
	bindingLayout.binding = 0;
	bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
	bindingLayout.buffer.type = BufferBindingType::Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(MyUniforms);

	BindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[1];
	textureBindingLayout.binding = 1;
	textureBindingLayout.visibility = ShaderStage::Fragment;
	textureBindingLayout.texture.sampleType = TextureSampleType::Float;
	textureBindingLayout.texture.viewDimension = TextureViewDimension::_2D;

	BindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[2];
	samplerBindingLayout.binding = 2;
	samplerBindingLayout.visibility = ShaderStage::Fragment;
	samplerBindingLayout.sampler.type = SamplerBindingType::Filtering;

	// Create a bind group layout
	BindGroupLayoutDescriptor bindGroupLayoutDesc{};
	bindGroupLayoutDesc.entryCount = bindingLayoutEntries.size();
	bindGroupLayoutDesc.entries = bindingLayoutEntries.data();
	bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

	// Create the pipeline layout
	PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
	layout = device.createPipelineLayout(layoutDesc);

	pipelineDesc.layout = layout;

	pipeline = device.createRenderPipeline(pipelineDesc);

	// We no longer need to access the shader module
	shaderModule.release();

	return pipeline != nullptr;
}

bool Application::InitializeGeometry() {

	// x0, y0, x1, y1, ...
	std::vector<VertexData> vertexData = {	};

	bool success = ResourceManager::loadGeometryFromObj(RESOURCE_DIR "/fourareen.obj", vertexData);
	// Check for errors
	if (!success) {
		std::cerr << "Could not load geometry!" << std::endl;
		return false;
	}


	// We will declare vertexCount as a member of the Application class
	vertexCount = static_cast<uint32_t>(vertexData.size());
	//indexCount = static_cast<uint32_t>(indexData.size());

	BufferDescriptor bufferDesc;
	bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Vertex; // Vertex usage here!
	bufferDesc.mappedAtCreation = false;

	bufferDesc.label = "Vertex Data";
	bufferDesc.size = vertexData.size() * sizeof(VertexData);
	vertexBuffer = device.createBuffer(bufferDesc);
	queue.writeBuffer(vertexBuffer, 0, vertexData.data(), bufferDesc.size);

	/*
	bufferDesc.label = "Index Buffer";
	bufferDesc.size = indexData.size() * sizeof(uint16_t);
	bufferDesc.size = (bufferDesc.size + 3) & ~3;
	bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Index;
	indexBuffer = device.createBuffer(bufferDesc);
	queue.writeBuffer(indexBuffer, 0, indexData.data(), bufferDesc.size);
	*/

	return vertexBuffer != nullptr;
}

bool Application::InitializeUniformBuffer() {
	SupportedLimits supportedLimits;
	device.getLimits(&supportedLimits);
	Limits deviceLimits = supportedLimits.limits;
	uniformStride = ceilToNextMultiple((uint32_t)sizeof(MyUniforms), (uint32_t)deviceLimits.minUniformBufferOffsetAlignment);

	BufferDescriptor bufferDesc;
	bufferDesc.label = "Uniform Buffer";
	bufferDesc.size = uniformStride + sizeof(MyUniforms);
	bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
	bufferDesc.mappedAtCreation = false;
	uniformBuffer = device.createBuffer(bufferDesc);

	uniforms.time = 1.0f;
	uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };

	uniforms.modelMatrix = mat4x4(1.0);
	UpdateViewMatrix();
	UpdateProjectionMatrix();

	queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(MyUniforms));
	return uniformBuffer != nullptr;
}

bool Application::InitializeBindGroups() {
	// Create a binding
	std::vector<BindGroupEntry> bindings(3);
	bindings[0].binding = 0;
	bindings[0].buffer = uniformBuffer;
	bindings[0].offset = 0;
	bindings[0].size = sizeof(MyUniforms);

	bindings[1].binding = 1;
	bindings[1].textureView = textureView;

	bindings[2].binding = 2;
	bindings[2].sampler = sampler;

	// A bind group contains one or multiple bindings
	BindGroupDescriptor bindGroupDesc{};
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = bindings.size();
	bindGroupDesc.entries = bindings.data();
	bindGroup = device.createBindGroup(bindGroupDesc);

	return bindGroup != nullptr;
}

bool Application::InitializeTextures() {

	texture = ResourceManager::loadTexture(RESOURCE_DIR "/fourareen2K_albedo.jpg", device, &textureView);
	if (!texture) {
		std::cerr << "Could not load texture!" << std::endl;
		return false;
	}

	SamplerDescriptor samplerDesc;
	samplerDesc.addressModeU = AddressMode::Repeat;
	samplerDesc.addressModeV = AddressMode::Repeat;
	samplerDesc.addressModeW = AddressMode::Repeat;
	samplerDesc.magFilter = FilterMode::Linear;
	samplerDesc.minFilter = FilterMode::Linear;
	samplerDesc.mipmapFilter = MipmapFilterMode::Nearest;
	samplerDesc.lodMinClamp = 0.0f;
	samplerDesc.lodMaxClamp = 8.0f;
	samplerDesc.compare = CompareFunction::Undefined;
	samplerDesc.maxAnisotropy = 1;
	sampler = device.createSampler(samplerDesc);

	return texture != nullptr && sampler != nullptr;
}

bool Application::InitializeWindowAndDevice() {

	instance = createInstance(InstanceDescriptor{});
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
		return false;
	}


	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW!" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
	if (!window) {
		std::cerr << "Could not create window!" << std::endl;
		return false;
	}


	std::cout << "Requesting adapter..." << std::endl;
	surface = glfwGetWGPUSurface(instance, window);
	RequestAdapterOptions adapterOpts = {};
	adapterOpts.compatibleSurface = surface;
	Adapter adapter = instance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	std::cout << "Requesting device..." << std::endl;
	RequiredLimits requiredLimits = GetRequiredLimits(adapter);
	DeviceDescriptor deviceDesc = {};
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = &requiredLimits;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void* /* pUserData */) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	device = adapter.requestDevice(deviceDesc);
	std::cout << "Got device: " << device << std::endl;

	m_errorCallbackHandle = device.setUncapturedErrorCallback([](ErrorType type, char const* message) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	});

	queue = device.getQueue();
#ifdef WEBGPU_BACKEND_WGPU
		surfaceFormat = surface.getPreferredFormat(adapter);
#else
		surfaceFormat = TextureFormat::BGRA8Unorm;
#endif

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int, int){
		auto that = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		if (that != nullptr) that->OnResize();
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		auto that = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		if (that != nullptr) that->onMouseButton(button, action, mods);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
		auto that = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		if (that != nullptr) that->onMouseMove(xpos, ypos);
	});

	glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
		auto that = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		if (that != nullptr) that->onScroll(xoffset, yoffset);
	});

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		auto that = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
		if (that != nullptr) that->onKey(key, scancode, action, mods);
	});


	// Release the adapter only after it has been fully utilized
	adapter.release();
	return device != nullptr;
}


bool Application::InitializeSurface() {
	// Get the current size of the window's framebuffer:
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	std::cout << "Configuring Surface" << std::endl;
	SurfaceConfiguration config;
	config.width = static_cast<uint32_t>(width);
	config.height = static_cast<uint32_t>(height);
	config.usage = TextureUsage::RenderAttachment;
	config.format = surfaceFormat;
	config.presentMode = PresentMode::Fifo;
	config.viewFormatCount = 0;
	config.viewFormats = nullptr;
	config.device = device;
	surface.configure(config);
	std::cout << "Surface: " << surface << std::endl;
	return surface != nullptr;

}

bool Application::InitializeDepthBuffer() {
	// Get the current size of the window's framebuffer:
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);

	TextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = TextureDimension::_2D;
	depthTextureDesc.format = depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	depthTextureDesc.usage = TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
	depthTexture = device.createTexture(depthTextureDesc);

	TextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = TextureViewDimension::_2D;
	depthTextureViewDesc.format = depthTextureFormat;
	depthTextureView = depthTexture.createView(depthTextureViewDesc);

	return depthTextureView != nullptr;
}

void Application::TerminateTextures() {
	textureView.release();
	texture.destroy();
	texture.release();
	sampler.release();
}

void Application::TerminateGeometry() {
	vertexBuffer.destroy();
	vertexBuffer.release();
	vertexCount = 0;
}

void Application::TerminateBindGroups() {
	bindGroup.release();
}

void Application::TerminateUniformBuffer() {
	uniformBuffer.destroy();
	uniformBuffer.release();
}

void Application::TerminateRenderPipeline() {
	pipeline.release();
	layout.release();
	bindGroupLayout.release();
}

void Application::TerminateWindowAndDevice() {
	queue.release();
	device.release();
	surface.release();
	instance.release();
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::TerminateSurface() {
	surface.unconfigure();
	surface.release();
}


void Application::TerminateDepthBuffer() {
	depthTextureView.release();
	depthTexture.destroy();
	depthTexture.release();
}

void Application::UpdateProjectionMatrix() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float ratio = width / (float)height;
	uniforms.projectionMatrix = glm::perspective(45 * PI / 180, ratio, 0.01f, 100.0f);
	queue.writeBuffer(
		uniformBuffer,
		offsetof(MyUniforms, projectionMatrix),
		&uniforms.projectionMatrix,
		sizeof(MyUniforms::projectionMatrix)
	);
}

void Application::UpdateViewMatrix() {
	float cx = cos(cameraState.angles.x);
	float sx = sin(cameraState.angles.x);
	float cy = cos(cameraState.angles.y);
	float sy = sin(cameraState.angles.y);
	vec3 position = vec3(cx * cy, sx * cy, sy) * std::exp(-cameraState.zoom);
	uniforms.viewMatrix = glm::lookAt(position, vec3(0.0f), vec3(0, 0, 1));
	queue.writeBuffer(
		uniformBuffer,
		offsetof(MyUniforms, viewMatrix),
		&uniforms.viewMatrix,
		sizeof(MyUniforms::viewMatrix)
	);
}

void Application::UpdateDragInertia() {
	constexpr float eps = 1e-4f;
	// Apply inertia only when the user released the click.
	if (!dragState.active) {
		// Avoid updating the matrix when the velocity is no longer noticeable
		if (std::abs(dragState.velocity.x) < eps && std::abs(dragState.velocity.y) < eps) {
			return;
		}
		cameraState.angles += dragState.velocity;
		cameraState.angles.y = glm::clamp(cameraState.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);
		// Dampen the velocity so that it decreases exponentially and stops
		// after a few frames.
		dragState.velocity *= dragState.inertia;
		UpdateViewMatrix();
	}
}

bool Application::InitializeGUI() {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.UserData = this;
	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther(window, true);
	ImGui_ImplWGPU_InitInfo info;
	info.Device = device;
	info.RenderTargetFormat = surfaceFormat;
	info.DepthStencilFormat = depthTextureFormat;
	info.NumFramesInFlight = 3;
	/*
	info.ViewportPresentMode = PresentMode::Fifo;
	info.CreateViewportWindowFn = [](ImGuiViewport* viewport) {
		auto that = reinterpret_cast<Application*>(ImGui::GetIO().UserData);
		auto* window = static_cast<GLFWwindow*>(viewport->PlatformHandle);

		int x, y;
		glfwGetWindowPos(window, &x, &y);
		std::cout << "Creating window for viewport " << viewport->ID << " at " << x << ", " << y << std::endl;

		return glfwGetWGPUSurface(that->instance, window);
	};
	*/

	ImGui_ImplWGPU_Init(&info);

	return true;
}

void Application::UpdateGUI(wgpu::RenderPassEncoder renderPass) {
	auto& io = ImGui::GetIO();

	// Start the Dear ImGui frame
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Build our UI
	static float f = 0.0f;
	static int counter = 0;
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;


	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);
	ImGui::Begin("Hello, world!");                                // Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");                     // Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);            // Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);                  // Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color);       // Edit 3 floats representing a color

	if (ImGui::Button("Button"))                                  // Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	ImGui::End();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
}

void Application::TerminateGUI() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplWGPU_Shutdown();
}

void Application::onMouseMove(double xpos, double ypos) {
	if (dragState.active) {
		glm::vec2 currentMouse = glm::vec2((float)xpos, (float)ypos);
		glm::vec2 delta = (currentMouse - dragState.startMouse) * dragState.sensitivity;
		cameraState.angles = dragState.startCameraState.angles + delta;
		// Clamp to avoid going too far when orbitting up/down
		cameraState.angles.y = glm::clamp(cameraState.angles.y, -PI / 2 + 1e-5f, PI / 2 - 1e-5f);
		UpdateViewMatrix();

		dragState.velocity = delta - dragState.previousDelta;
		dragState.previousDelta = delta;
	}
}

void Application::onMouseButton(int button, int action, [[maybe_unused]] int mods) {
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) return;

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		switch(action) {
			case GLFW_PRESS:
				dragState.active = true;
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			dragState.startMouse = glm::vec2((float)xpos, (float)ypos);
			dragState.startCameraState = cameraState;
			break;
			case GLFW_RELEASE:
				dragState.active = false;
			break;
			default: break;
		}
	}
}

void Application::onScroll([[maybe_unused]] double xoffset, double yoffset) {
	cameraState.zoom += dragState.scrollSensitivity * static_cast<float>(yoffset);
	cameraState.zoom = glm::clamp(cameraState.zoom, -2.0f, 2.0f);
	UpdateViewMatrix();
}

void Application::onKey([[maybe_unused]] int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods) {
}


TextureView Application::GetNextSurfaceTextureView() {
	// Get the surface texture
	SurfaceTexture surfaceTexture;
	surface.getCurrentTexture(&surfaceTexture);
	if (surfaceTexture.status != SurfaceGetCurrentTextureStatus::Success) {
		return nullptr;
	}
	Texture texture = surfaceTexture.texture;

	// Create a view for this surface texture
	TextureViewDescriptor viewDescriptor;
	viewDescriptor.label = "Surface texture view";
	viewDescriptor.format = texture.getFormat();
	viewDescriptor.dimension = TextureViewDimension::_2D;
	viewDescriptor.baseMipLevel = 0;
	viewDescriptor.mipLevelCount = 1;
	viewDescriptor.baseArrayLayer = 0;
	viewDescriptor.arrayLayerCount = 1;
	viewDescriptor.aspect = TextureAspect::All;
	TextureView targetView = texture.createView(viewDescriptor);

#ifndef WEBGPU_BACKEND_WGPU
	// We no longer need the texture, only its view
	// (NB: with wgpu-native, surface textures must not be manually released)
	wgpuTextureRelease(surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU

	return targetView;
}

RequiredLimits Application::GetRequiredLimits(wgpu::Adapter adapter) {
	SupportedLimits supportedLimits;
	adapter.getLimits(&supportedLimits);

	int monitorCount;
	GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
	int largestWidth = 0;
	int largestHeight = 0;
	for (int i = 0; i < monitorCount; i++) {
		int xpos;
		int ypos;
		int width;
		int height;
		glfwGetMonitorWorkarea(monitors[i], &xpos, &ypos, &width, &height);
		largestWidth += width;
		largestHeight += height;
	}

	RequiredLimits requiredLimits = Default;
	requiredLimits.limits.maxVertexAttributes = 4;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBufferSize = 1000000 * sizeof(VertexData);
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexData);
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.maxTextureDimension2D = supportedLimits.limits.maxTextureDimension2D;
	requiredLimits.limits.maxInterStageShaderComponents = 8;
	requiredLimits.limits.maxBindGroups = 2;
	requiredLimits.limits.maxUniformBuffersPerShaderStage = 2;
	requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);
	requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
	requiredLimits.limits.maxTextureDimension1D = std::max(largestWidth, largestHeight);
	requiredLimits.limits.maxTextureDimension2D = std::max(largestWidth, largestHeight);
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 4;
	requiredLimits.limits.maxSamplersPerShaderStage = 4;
	requiredLimits.limits.maxBindingsPerBindGroup = 4;


	return requiredLimits;
}
