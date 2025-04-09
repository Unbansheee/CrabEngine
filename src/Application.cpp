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

#ifdef __EMSCRIPTEN__
#  include <emscripten.h>
#endif // __EMSCRIPTEN__


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

	std::cout << "Requesting adapter..." << std::endl;
	wgpu::RequestAdapterOptions adapterOpts = {};
	//surface = glfwGetWGPUSurface(wgpuInstance, window);
	//adapterOpts.compatibleSurface = surface;
	wgpu::Adapter adapter = wgpuInstance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	wgpu::AdapterProperties p;
	adapter.getProperties(&p);
	wgpuDevice.

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
		//std::cout << "Uncaptured device error: type " << type;
		//if (message) std::cout << " (" << message << ")";
		//std::cout << std::endl;
		Assert::Check(false, "Uncaptured device error: type" + std::to_string((type)), message);
	});
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
	// Unregisters all types with the factory and cleans up the default material
	JPH::UnregisterJoltTypes();
	// Destroy the factory
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;

	NFD::Quit();
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
	requiredLimits.limits.maxInterStageShaderComponents = 64;
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
	requiredLimits.limits.maxStorageTexturesPerShaderStage = 2;

	requiredLimits.limits.maxComputeWorkgroupsPerDimension = 65535;
	requiredLimits.limits.maxComputeInvocationsPerWorkgroup = 32*32;
	requiredLimits.limits.maxComputeWorkgroupSizeX = 32;
	requiredLimits.limits.maxComputeWorkgroupSizeY = 32;
	requiredLimits.limits.maxComputeWorkgroupSizeZ = 32;


	return requiredLimits;
}
