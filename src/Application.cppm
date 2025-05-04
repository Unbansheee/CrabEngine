//
// Created by Vinnie on 20/02/2025.
//
module;

#pragma once

export module Engine.Application;
import Engine.Node;
import Engine.SceneTree;
import Engine.GLM;
import Engine.WGPU;
import Engine.StringID;
import Engine.Physics.Jolt;
import vfspp;
import Engine.Filesystem;
import Engine.ScriptEngine;

struct ImGuiContext;
using glm::mat4x4;
using glm::vec4;

// Tick Helper
struct DeltaTicker
{
public:
    float Tick(float newTime)
    {
        float delta = newTime - previousTime;
        previousTime = newTime;
        return delta;
    }

private:
    float previousTime = 0.0f;
};



export class Application
{
public:
    // Get or instantiate the application
    static Application& Get();

    // Load asset references and Begin the scenetree
    void Begin();
    // Tick the engine's update loop
    void Update();
    // Has a close been requested
    bool ShouldClose() const;
    // Request for the application to close
    void Close();

    wgpu::Instance GetInstance() const { return wgpuInstance; }
    wgpu::Device GetDevice() const { return wgpuDevice; }
    wgpu::Queue GetQueue() const { return wgpuDeviceGetQueue(wgpuDevice); }

    static wgpu::Limits GetRequiredLimits(wgpu::Adapter adapter);

    // Get the application's scenetree
    SceneTree& GetSceneTree() {return sceneTree;}

    // Get the time passed since the last update
    float DeltaTime() {return dt;};

    // Get the string database responsible for string_id hashing
    static sid::default_database& GetStringDB();

    // Jolt stuff
    JPH::TempAllocator* GetPhysicsAllocator() const;
    JPH::JobSystem* GetJobSystem() const;

    // Get the engine's filesystem. Prefer to use Filesystem:: from Engine.Filesystem instead
    vfspp::VirtualFileSystemPtr GetFilesystem();

    // Get the C# Script Engine instance
    ScriptEngine* GetScriptEngine() {return scriptEngine.get();};
protected:
    SceneTree sceneTree;
    DeltaTicker deltaTime;
    float dt;
    bool bShouldClose = false;

    // Scripting
    std::unique_ptr<ScriptEngine> scriptEngine;

    // Graphics
    wgpu::Instance wgpuInstance = nullptr;
    wgpu::Device wgpuDevice = nullptr;

    // Physics
    int maxConcurrentJobs = std::thread::hardware_concurrency();		// How many jobs to run in parallel
    JPH::TempAllocator* tempAllocator;
    JPH::JobSystem* jobSystem;

    // Singleton class, no copy or assignment
    Application(const Application &) = delete;
    Application & operator = (const Application &) = delete;
private:
    Application();
    ~Application();
};

