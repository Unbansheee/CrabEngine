module;
#include "nfd.hpp"
#include "nativefiledialog-extended/src/include/nfd_glfw3.h"
#include "Jolt/Jolt.h"
#include "Jolt/Core/IssueReporting.h"

module Engine.Application;
import Engine.GFX.MeshVertex;
import Engine.WGPU;
import Engine.Physics.Jolt;
import Engine.Assert;
import "GLFW/glfw3.h";
import "cstdarg";
import Engine.Filesystem;
import Engine.Resource.ResourceManager;

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__

using namespace std::string_view_literals;
// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS

// Callback for asserts, connect this to your own assert handler if you have one
static bool AssertFailedImpl(const char *inExpression, const char *inMessage, const char *inFile, JPH::uint inLine)
{
	// Print to the TTY
	std::cout << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr? inMessage : "") << std::endl;
	// Breakpoint
	return true;
};
#endif // JPH_ENABLE_ASSERTS

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

	auto rootFS = std::make_unique<vfspp::NativeFileSystem>(std::filesystem::current_path().string());
	rootFS->Initialize();

	Filesystem::AddFileSystemDirectory("/engine", ENGINE_RESOURCE_DIR);
	Filesystem::AddFileSystemDirectory("/dotnet", (std::filesystem::current_path() /= "Dotnet").generic_string());


	scriptEngine.reset(new ScriptEngine());
	scriptEngine->Init();
	scriptEngine->LoadModule(Filesystem::StringToWString(Filesystem::AbsolutePath("/dotnet/CrabApplication.dll")), L"CrabApplication");


	std::cout << "Requesting adapter..." << std::endl;
	wgpu::RequestAdapterOptions adapterOpts = {};
	//surface = glfwGetWGPUSurface(wgpuInstance, window);
	//adapterOpts.compatibleSurface = surface;
	adapterOpts.featureLevel = WGPUFeatureLevel_Core;

	wgpu::Adapter adapter = wgpuInstance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	std::vector<WGPUFeatureName> requiredFeatures;
	requiredFeatures.push_back((WGPUFeatureName)wgpu::NativeFeature::TextureAdapterSpecificFormatFeatures);
	requiredFeatures.push_back((WGPUFeatureName)wgpu::NativeFeature::VertexWritableStorage);
	requiredFeatures.push_back((WGPUFeatureName)wgpu::NativeFeature::PushConstants);

	std::cout << "Requesting device..." << std::endl;
	wgpu::Limits requiredLimits = GetRequiredLimits(adapter);
	wgpu::NativeLimits nativeLimits = wgpu::Default;
	nativeLimits.maxPushConstantSize = 256;
	requiredLimits.nextInChain = &nativeLimits.chain;

	wgpu::DeviceDescriptor deviceDesc = {};
	deviceDesc.label = {"My Device", wgpu::STRLEN};
	deviceDesc.requiredFeatureCount = requiredFeatures.size();
	deviceDesc.requiredFeatures = requiredFeatures.data();
	deviceDesc.requiredLimits = &requiredLimits;
	deviceDesc.defaultQueue.nextInChain = nullptr;
	deviceDesc.defaultQueue.label = {"The default queue", wgpu::STRLEN};

	wgpu::DeviceLostCallbackInfo deviceLostCallbackInfo;
	deviceLostCallbackInfo.callback = [](WGPUDevice const * device, WGPUDeviceLostReason reason, WGPUStringView message, void*, void*)  {
		std::cout << "Device lost: reason " << reason;
		if (message.data) std::cout << " (" << message.data << ")";
		std::cout << std::endl;
	};

	deviceDesc.deviceLostCallbackInfo = deviceLostCallbackInfo;
	deviceDesc.uncapturedErrorCallbackInfo.callback = [](WGPUDevice const * device, WGPUErrorType type, WGPUStringView message, void* userdata1, void* userdata2) {
		Assert::Check(false, "Uncaptured device error: type" + std::to_string((type)), message.data);
	}
	;
	wgpuDevice = adapter.requestDevice(deviceDesc);
	Assert::Check(wgpuDevice, "wgpuDevice != nullptr", "Failed to initialize WGPU Device");

	std::cout << "Got device: " << wgpuDevice << std::endl;

	NFD::Init();

	JPH::RegisterDefaultAllocator();
	JPH::Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl;)
	JPH::Factory::sInstance = new JPH::Factory();
	JPH::RegisterJoltTypes();
	tempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
	jobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, maxConcurrentJobs - 1);
	sceneTree.SetRoot(Node::NewNode());

}


Application::~Application()
{
	sceneTree.Clear();
	// Unregisters all types with the factory and cleans up the default material
	JPH::UnregisterJoltTypes();
	// Destroy the factory
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	NFD::Quit();
	glfwTerminate();
}

Application & Application::Get() {
	static Application s;
	return s;
}

void Application::Begin()
{
	auto files = Filesystem::GetAllFiles();
	for (auto& [dir, file] : files) {
		ResourceManager::Load(dir);
	}

	sceneTree.Begin();
}

bool Application::ShouldClose() const
{
	return bShouldClose;
}

void Application::Update()
{
	dt = deltaTime.Tick((float)glfwGetTime());
	sceneTree.Update(dt);
}

void Application::Close()
{
	bShouldClose = true;
}

wgpu::Limits Application::GetRequiredLimits(wgpu::Adapter adapter)
{
	wgpu::Limits supportedLimits;
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

	wgpu::Limits requiredLimits = wgpu::Default;
	requiredLimits.maxVertexAttributes = 6;
	requiredLimits.maxVertexBuffers = 1;
	requiredLimits.maxBufferSize = 10000000 * sizeof(MeshVertex);
	requiredLimits.maxVertexBufferArrayStride = sizeof(MeshVertex);
	requiredLimits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment;
	requiredLimits.minStorageBufferOffsetAlignment = supportedLimits.minStorageBufferOffsetAlignment;
	requiredLimits.maxTextureDimension2D = supportedLimits.maxTextureDimension2D;
	requiredLimits.maxInterStageShaderVariables = 64;
	requiredLimits.maxBindGroups = 4;
	requiredLimits.maxUniformBuffersPerShaderStage = 8;
	requiredLimits.maxUniformBufferBindingSize = 65536;
	requiredLimits.maxDynamicUniformBuffersPerPipelineLayout = 1;
	requiredLimits.maxTextureDimension1D = std::max(largestWidth, largestHeight);
	requiredLimits.maxTextureDimension2D = std::max(largestWidth, largestHeight);
	requiredLimits.maxTextureArrayLayers = 1;
	requiredLimits.maxSampledTexturesPerShaderStage = 4;
	requiredLimits.maxSamplersPerShaderStage = 4;
	requiredLimits.maxBindingsPerBindGroup = 5;
	requiredLimits.maxStorageTexturesPerShaderStage = 2;

	requiredLimits.maxComputeWorkgroupsPerDimension = 65535;
	requiredLimits.maxComputeInvocationsPerWorkgroup = 32*32;
	requiredLimits.maxComputeWorkgroupSizeX = 32;
	requiredLimits.maxComputeWorkgroupSizeY = 32;
	requiredLimits.maxComputeWorkgroupSizeZ = 32;

	return requiredLimits;
}
