// Include the C++ wrapper instead of the raw header(s)
module application;
import mesh_vertex;
import wgpu;
import <GLFW/glfw3.h>;
//#include <webgpu/webgpu.hpp>
//#include <GLFW/glfw3.h>
//#include <filesystem>
#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__


//#include <iostream>


/*
using namespace wgpu;
using glm::vec3;

namespace ImGui {
	bool DragDirection(const char* label, vec4& direction) {
		glm::vec2 angles = glm::degrees(glm::polar(vec3(direction)));
		bool changed = ImGui::DragFloat2(label, glm::value_ptr(angles));
		direction = vec4(glm::euclidean(glm::radians(angles)), direction.w);
		return changed;
	}
} // namespace ImGui

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
	//if (!InitializeRenderPipeline()) return false;
	if (!InitializeDepthBuffer()) return false;
	if (!InitializeUniformBuffer()) return false;
	if (!InitializeTextures()) return false;
	if (!InitializeLightingUniforms()) return false;
	//if (!InitializeBindGroups()) return false;
	if (!InitializeGUI()) return false;
	if (!InitializeNodes()) return false;

	previousFrameTime = glfwGetTime();
	
	return true;
}

void Application::Terminate() {
	TerminateGUI();
	TerminateTextures();
	TerminateDepthBuffer();
	//TerminateBindGroups();

	TerminateUniformBuffer();
	//TerminateRenderPipeline();
	TerminateSurface();
	TerminateWindowAndDevice();
}

void Application::MainLoop() {
	glfwMakeContextCurrent(window);
	glfwPollEvents();

	UpdateNodes();
	UpdateDragInertia();
	TextureView targetView = GetNextSurfaceTextureView();
	std::vector<CommandBuffer> commands;
	*/
	/*
	MyUniforms t{};
	t.time = static_cast<float>(glfwGetTime());
	queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, time), &t.time, sizeof(MyUniforms::time));

	double now = glfwGetTime();
	deltaTime = static_cast<float>(now - previousFrameTime);
	previousFrameTime = now;
	

	UpdateLightingUniforms();

	// Get the next target texture view
	

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
	*/
	//std::vector<DrawCommand> drawCommands;
	//GatherDrawCommands(drawCommands);

	/*
	uint32_t dynamicOffset = 0;
	int idx = 0;
	for (auto& drawCommand : drawCommands) {
		dynamicOffset = idx * perObjectUniformStride;

		renderPass.setPipeline(pipeline);
		renderPass.setVertexBuffer(0, drawCommand.vertexBuffer, 0, WGPU_WHOLE_SIZE);
		renderPass.setBindGroup(0, bindGroup, 1, &dynamicOffset);
		queue.writeBuffer(perObjectUniformBuffer, dynamicOffset + offsetof(PerObjectUniforms, modelMatrix), &drawCommand.modelMatrix, sizeof(mat4x4));
		if (drawCommand.indexCount > 0) {
			renderPass.setIndexBuffer(drawCommand.indexBuffer, IndexFormat::Uint16, 0, WGPU_WHOLE_SIZE);
			renderPass.drawIndexed(drawCommand.indexCount, 1, 0, 0, 0);
		}
		else {
			renderPass.draw(drawCommand.vertexCount, 1, 0, 0);
		}

		idx++;
	}
	*/

	/*
	// Select which render pipeline to use
	renderPass.setPipeline(pipeline);
	renderPass.setVertexBuffer(0, vertexBuffer, 0, vertexBuffer.getSize());
	//renderPass.setIndexBuffer(indexBuffer, IndexFormat::Uint16, 0, indexBuffer.getSize());
	// Draw 1 instance of a 3-vertices shape

	renderPass.setBindGroup(0, bindGroup, 0, nullptr);
	renderPass.draw(vertexCount, 1, 0, 0);
	*/
	/*
	dynamicOffset = 1 * uniformStride;
	renderPass.setBindGroup(0, bindGroup, 1, &dynamicOffset);
	renderPass.drawIndexed(indexCount, 1, 0, 0, 0);
	*/

	/*
	renderPass.end();
	renderPass.release();

	// Finally encode and submit the render pass
	CommandBufferDescriptor cmdBufferDescriptor = {};
	cmdBufferDescriptor.label = "Scene Draw Command Buffer";
	commands.emplace_back(encoder.finish(cmdBufferDescriptor));
	*/

/*
	WGPURenderPassColorAttachment color_attachments = {};
	color_attachments.loadOp = WGPULoadOp_Load;
	color_attachments.storeOp = WGPUStoreOp_Store;
	color_attachments.clearValue = { 0, 0, 0, 0 };
	color_attachments.view = targetView;
	
	WGPURenderPassDescriptor render_pass_desc = {};
	render_pass_desc.colorAttachmentCount = 1;
	render_pass_desc.colorAttachments = &color_attachments;
	render_pass_desc.depthStencilAttachment = nullptr;
	
	WGPUCommandEncoderDescriptor enc_desc = {};
	CommandEncoder gui_encoder = wgpuDeviceCreateCommandEncoder(device, &enc_desc);
	
	RenderPassEncoder pass = gui_encoder.beginRenderPass(render_pass_desc);
	UpdateGUI(pass);
	pass.end();

	CommandBufferDescriptor cmd_buffer_desc = {};
	cmd_buffer_desc.label = "ImGUI Draw Command Buffer";
	commands.emplace_back(gui_encoder.finish(cmd_buffer_desc));

	queue.submit(commands.size(), commands.data());

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

#ifndef __EMSCRIPTEN__
	surface.present();
#endif


	for (auto& commandBuffer : commands) {
		commandBuffer.release();
	}
	//renderpass is released earlier
	pass.release();
	//encoder.release();
	gui_encoder.release();
	targetView.release();
	

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
*/

/*
bool Application::InitializeRenderPipeline() {

	std::cout << "Creating shader module..." << std::endl;
	ShaderModule shaderModule = ResourceManager::loadShaderModule(RESOURCE_DIR "/shader.wgsl", device);
	std::cout << "Shader module: " << shaderModule << std::endl;

	// Check for errors
	if (shaderModule == nullptr) {
		std::cerr << "Could not load shader!" << std::endl;
		return false;
	}
	
	VertexBufferLayout vertexBufferLayout = Vertex::CreateVertexBufferLayout<MeshVertex>();

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
	std::vector<BindGroupLayoutEntry> bindingLayoutEntries(6, Default);

	BindGroupLayoutEntry& bindingLayout = bindingLayoutEntries[0];
	bindingLayout.binding = 0;
	bindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
	bindingLayout.buffer.type = BufferBindingType::Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(MyUniforms);

	BindGroupLayoutEntry& perObjectBindingLayout = bindingLayoutEntries[1];
	perObjectBindingLayout.binding = 1;
	perObjectBindingLayout.visibility = ShaderStage::Vertex | ShaderStage::Fragment;
	perObjectBindingLayout.buffer.type = BufferBindingType::Uniform;
	perObjectBindingLayout.buffer.minBindingSize = sizeof(PerObjectUniforms);
	perObjectBindingLayout.buffer.hasDynamicOffset = true;

	BindGroupLayoutEntry& textureBindingLayout = bindingLayoutEntries[2];
	textureBindingLayout.binding = 2;
	textureBindingLayout.visibility = ShaderStage::Fragment;
	textureBindingLayout.texture.sampleType = TextureSampleType::Float;
	textureBindingLayout.texture.viewDimension = TextureViewDimension::_2D;

	BindGroupLayoutEntry& normalTextureBindingLayout = bindingLayoutEntries[3];
	normalTextureBindingLayout.binding = 3;
	normalTextureBindingLayout.visibility = ShaderStage::Fragment;
	normalTextureBindingLayout.texture.sampleType = TextureSampleType::Float;
	normalTextureBindingLayout.texture.viewDimension = TextureViewDimension::_2D;

	BindGroupLayoutEntry& samplerBindingLayout = bindingLayoutEntries[4];
	samplerBindingLayout.binding = 4;
	samplerBindingLayout.visibility = ShaderStage::Fragment;
	samplerBindingLayout.sampler.type = SamplerBindingType::Filtering;

	BindGroupLayoutEntry& lightingBindingLayout = bindingLayoutEntries[5];
	lightingBindingLayout.binding = 5;
	lightingBindingLayout.visibility = ShaderStage::Fragment;
	lightingBindingLayout.buffer.type = BufferBindingType::Uniform;
	lightingBindingLayout.buffer.minBindingSize = sizeof(LightingUniforms);

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
*/

/*
bool Application::InitializeUniformBuffer() {
	SupportedLimits supportedLimits;
	device.getLimits(&supportedLimits);
	Limits deviceLimits = supportedLimits.limits;
	uniformStride = ceilToNextMultiple((uint32_t)sizeof(MyUniforms), (uint32_t)deviceLimits.minUniformBufferOffsetAlignment);
	perObjectUniformStride = ceilToNextMultiple((uint32_t)sizeof(PerObjectUniforms), (uint32_t)deviceLimits.minUniformBufferOffsetAlignment);
	
	BufferDescriptor bufferDesc;
	bufferDesc.label = "Uniform Buffer";
	bufferDesc.size = uniformStride + sizeof(MyUniforms);
	bufferDesc.usage = BufferUsage::Uniform | BufferUsage::CopyDst;
	bufferDesc.mappedAtCreation = false;
	uniformBuffer = device.createBuffer(bufferDesc);

	bufferDesc.label = "Per Object Uniform Buffer";
	bufferDesc.size = perObjectUniformStride + sizeof(PerObjectUniforms) * 200;
	perObjectUniformBuffer = device.createBuffer(bufferDesc);

	uniforms.time = 1.0f;
	uniforms.color = { 0.0f, 1.0f, 0.4f, 1.0f };

	UpdateViewMatrix();
	UpdateProjectionMatrix();

	queue.writeBuffer(uniformBuffer, 0, &uniforms, sizeof(MyUniforms));
	return uniformBuffer != nullptr;
}

bool Application::InitializeBindGroups() {
	// Create a binding
	std::vector<BindGroupEntry> bindings(6);
	bindings[0].binding = 0;
	bindings[0].buffer = uniformBuffer;
	bindings[0].offset = 0;
	bindings[0].size = sizeof(MyUniforms);

	bindings[1].binding = 1;
	bindings[1].buffer = perObjectUniformBuffer;
	bindings[1].offset = 0;
	bindings[1].size = sizeof(PerObjectUniforms);

	bindings[2].binding = 2;
	bindings[2].textureView = baseColorTextureView;

	bindings[3].binding = 3;
	bindings[3].textureView = normalTextureView;

	bindings[4].binding = 4;
	bindings[4].sampler = sampler;

	bindings[5].binding = 5;
	bindings[5].buffer = lightingUniformBuffer;
	bindings[5].offset = 0;
	bindings[5].size = sizeof(LightingUniforms);

	// A bind group contains one or multiple bindings
	BindGroupDescriptor bindGroupDesc{};
	bindGroupDesc.layout = bindGroupLayout;
	bindGroupDesc.entryCount = bindings.size();
	bindGroupDesc.entries = bindings.data();
	bindGroup = device.createBindGroup(bindGroupDesc);

	return bindGroup != nullptr;
}

bool Application::InitializeTextures() {

	baseColorTexture = ResourceManager::loadTexture(RESOURCE_DIR "/cobblestone_floor_08_diff_2k.jpg", device, &baseColorTextureView);
	normalTexture = ResourceManager::loadTexture(RESOURCE_DIR "/cobblestone_floor_08_nor_gl_2k.png", device, &normalTextureView);
	if (!baseColorTexture || !normalTexture) {
		std::cerr << "Could not load textures!" << std::endl;
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

	return baseColorTexture != nullptr && normalTexture != nullptr && sampler != nullptr;
}

bool Application::InitializeWindowAndDevice() {
	WGPUInstanceDescriptor instanceDesc{};
	instance = createInstance(instanceDesc);
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
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void*) {
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
	depthTextureFormat = TextureFormat::Depth24Plus;
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

bool Application::InitializeLightingUniforms() {
	BufferDescriptor bufferDesc;
	bufferDesc.size = sizeof(LightingUniforms);
	bufferDesc.usage = BufferUsage::CopyDst | BufferUsage::Uniform;
	bufferDesc.mappedAtCreation = false;
	lightingUniformBuffer = device.createBuffer(bufferDesc);

	// Initial values
	lightingUniforms.directions[0] = { 0.5f, -0.9f, 0.1f, 0.0f };
	lightingUniforms.directions[1] = { 0.2f, 0.4f, 0.3f, 0.0f };
	lightingUniforms.colors[0] = { 1.0f, 0.9f, 0.6f, 1.0f };
	lightingUniforms.colors[1] = { 0.6f, 0.9f, 1.0f, 1.0f };
	lightingUniforms.hardness = 32.f;
	lightingUniforms.kd = 1.0f;
	lightingUniforms.ks = 0.5f;
	lightingUniforms.normalStrength = 1.f;
	lightingUniformsChanged = true;

	UpdateLightingUniforms();

	return lightingUniformBuffer != nullptr;
}

bool Application::InitializeNodes() {


	//GLTFSceneParser parser;
	//auto tree = parser.ParseGLTF(device, RESOURCE_DIR "/Level1.glb");
	//AddChild(std::move(tree));
	
	//return true;
	rootNode = std::make_unique<Node>("Root");
	return true;
}

void Application::TerminateTextures() {
	normalTextureView.release();
	baseColorTextureView.release();
	normalTexture.destroy();
	normalTexture.release();
	baseColorTexture.destroy();
	baseColorTexture.release();
	sampler.release();
}

void Application::TerminateGeometry() {

}

void Application::TerminateBindGroups() {
	bindGroup.release();
}

void Application::TerminateUniformBuffer() {
	uniformBuffer.destroy();
	uniformBuffer.release();
}
*/
/*
void Application::TerminateRenderPipeline() {
	pipeline.release();
	layout.release();
	bindGroupLayout.release();
}
*/
/*
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

void Application::TerminateLightingUniforms() {
	lightingUniformBuffer.destroy();
	lightingUniformBuffer.release();
}

void Application::UpdateProjectionMatrix() {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float ratio = width / (float)height;
	uniforms.projectionMatrix = glm::perspective(45 * PI / 180, ratio, 0.01f, 1000.0f);
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

	uniforms.cameraWorldPosition = position;
	queue.writeBuffer(uniformBuffer, offsetof(MyUniforms, cameraWorldPosition), &uniforms.cameraWorldPosition, sizeof(uniforms.cameraWorldPosition));

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

void Application::UpdateLightingUniforms() {
	if (lightingUniformsChanged) {
		queue.writeBuffer(lightingUniformBuffer, 0, &lightingUniforms, sizeof(LightingUniforms));
		lightingUniformsChanged = false;
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
	info.DepthStencilFormat = TextureFormat::Undefined;
	info.NumFramesInFlight = 3;
	info.ViewportPresentMode = PresentMode::Fifo;
	info.CreateViewportWindowFn = [](ImGuiViewport* viewport) {
		auto that = reinterpret_cast<Application*>(ImGui::GetIO().UserData);
		auto* window = static_cast<GLFWwindow*>(viewport->PlatformHandle);

		int x, y;
		glfwGetWindowPos(window, &x, &y);
		std::cout << "Creating window for viewport " << viewport->ID << " at " << x << ", " << y << std::endl;

		return glfwGetWGPUSurface(that->instance, window);
	};


	ImGui_ImplWGPU_Init(&info);

	return true;
}


void Application::UpdateGUI(wgpu::RenderPassEncoder renderPass) {
	auto& io = ImGui::GetIO();

	// Start the Dear ImGui frame
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();


	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	dockspace_flags |= ImGuiDockNodeFlags_PassthruCentralNode;

	
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), dockspace_flags);
	// In Application::updateGui

	bool changed = false;
	ImGui::Begin("Lighting");
	changed = ImGui::ColorEdit3("Color #0", glm::value_ptr(lightingUniforms.colors[0])) || changed;
	changed = ImGui::DragDirection("Direction #0", lightingUniforms.directions[0]) || changed;
	changed = ImGui::ColorEdit3("Color #1", glm::value_ptr(lightingUniforms.colors[1])) || changed;
	changed = ImGui::DragDirection("Direction #1", lightingUniforms.directions[1]) || changed;
	changed = ImGui::SliderFloat("Hardness", &lightingUniforms.hardness, 1.0f, 100.f) || changed;
	changed = ImGui::SliderFloat("K Diffuse", &lightingUniforms.kd, 0.0f, 1.0f) || changed;
	changed = ImGui::SliderFloat("K Specular", &lightingUniforms.ks, 0.0f, 1.0f) || changed;
	changed = ImGui::SliderFloat("Normal Strength", &lightingUniforms.normalStrength, 0.0f, 1.0f) || changed;
	ImGui::End();
	lightingUniformsChanged = changed;

	ImGui::Begin("Scene");
	ImGuiDrawNode(rootNode.get());
	ImGui::End();

	ImGui::Begin("Statistics");
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
	cameraState.zoom = glm::clamp(cameraState.zoom, -20.0f, 20.0f);
	UpdateViewMatrix();
}

void Application::onKey([[maybe_unused]] int key, [[maybe_unused]] int scancode, [[maybe_unused]] int action, [[maybe_unused]] int mods) {
}

void Application::ImGuiDrawNode(Node *node) {

	bool bHasChildren = node->Children.size() > 0;

	if (bHasChildren) {
		if (ImGui::TreeNode((node->Name + "##xx").c_str())) {
			for (auto& child : node->Children) {
				ImGuiDrawNode(child.get());
			}
			ImGui::TreePop();
		}
	}
	else {
		if (ImGui::TreeNodeEx((node->Name + "##xx").c_str(), ImGuiTreeNodeFlags_Leaf)) {
			ImGui::TreePop();
		}
	}
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
	requiredLimits.limits.maxVertexAttributes = 6;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBufferSize = 10000000 * sizeof(MeshVertex);
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(MeshVertex);
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.maxTextureDimension2D = supportedLimits.limits.maxTextureDimension2D;
	requiredLimits.limits.maxInterStageShaderComponents = 17;
	requiredLimits.limits.maxBindGroups = 2;
	requiredLimits.limits.maxUniformBuffersPerShaderStage = 3;
	requiredLimits.limits.maxUniformBufferBindingSize = 65536;
	requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
	requiredLimits.limits.maxTextureDimension1D = std::max(largestWidth, largestHeight);
	requiredLimits.limits.maxTextureDimension2D = std::max(largestWidth, largestHeight);
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 4;
	requiredLimits.limits.maxSamplersPerShaderStage = 4;
	requiredLimits.limits.maxBindingsPerBindGroup = 5;


	return requiredLimits;
}
*/

Application::Application()
{
	WGPUInstanceDescriptor instanceDesc{};
	wgpuInstance = wgpu::createInstance(instanceDesc);
	if (!wgpuInstance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
	}

	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW!" << std::endl;
	}

	std::cout << "Requesting adapter..." << std::endl;
	wgpu::RequestAdapterOptions adapterOpts = {};
	//surface = glfwGetWGPUSurface(wgpuInstance, window);
	//adapterOpts.compatibleSurface = surface;
	wgpu::Adapter adapter = wgpuInstance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	std::cout << "Requesting device..." << std::endl;
	wgpu::RequiredLimits requiredLimits = GetRequiredLimits(adapter);
	wgpu::DeviceDescriptor deviceDesc = {};
	deviceDesc.label = "My Device";
	deviceDesc.requiredFeatureCount = 0;
	deviceDesc.requiredLimits = &requiredLimits;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = "The default queue";
	deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void*) {
		std::cout << "Device lost: reason " << reason;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	};
	wgpuDevice = adapter.requestDevice(deviceDesc);
	std::cout << "Got device: " << wgpuDevice << std::endl;

	errorCallbackHandle = wgpuDevice.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cout << "Uncaptured device error: type " << type;
		if (message) std::cout << " (" << message << ")";
		std::cout << std::endl;
	});
	
	sceneTree.SetRoot(Node::NewNode());
}

Application::~Application()
{
	glfwTerminate();
}

void Application::Begin()
{
	sceneTree.Begin();
}

bool Application::ShouldClose() const
{
	return bShouldClose;
}

void Application::Update()
{
	float dt = deltaTime.Tick((float)glfwGetTime());
	sceneTree.Update(dt);
}

void Application::Close()
{
	bShouldClose = true; 
}

wgpu::RequiredLimits Application::GetRequiredLimits(wgpu::Adapter adapter)
{
	wgpu::SupportedLimits supportedLimits;
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

	wgpu::RequiredLimits requiredLimits = wgpu::Default;
	requiredLimits.limits.maxVertexAttributes = 6;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBufferSize = 10000000 * sizeof(MeshVertex);
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(MeshVertex);
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.maxTextureDimension2D = supportedLimits.limits.maxTextureDimension2D;
	requiredLimits.limits.maxInterStageShaderComponents = 17;
	requiredLimits.limits.maxBindGroups = 4;
	requiredLimits.limits.maxUniformBuffersPerShaderStage = 3;
	requiredLimits.limits.maxUniformBufferBindingSize = 65536;
	requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
	requiredLimits.limits.maxTextureDimension1D = std::max(largestWidth, largestHeight);
	requiredLimits.limits.maxTextureDimension2D = std::max(largestWidth, largestHeight);
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 4;
	requiredLimits.limits.maxSamplersPerShaderStage = 4;
	requiredLimits.limits.maxBindingsPerBindGroup = 5;


	return requiredLimits;
}
