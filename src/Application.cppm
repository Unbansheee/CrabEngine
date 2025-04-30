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
    static Application& Get();

    void Begin();
    void Update();
    bool ShouldClose() const;
    void Close();

    wgpu::Instance GetInstance() const { return wgpuInstance; }
    wgpu::Device GetDevice() const { return wgpuDevice; }
    wgpu::Queue GetQueue() const { return wgpuDeviceGetQueue(wgpuDevice); }

    static wgpu::Limits GetRequiredLimits(wgpu::Adapter adapter);

    SceneTree& GetSceneTree() {return sceneTree;}
    
    struct MyUniforms {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        vec4 color;  // or float color[4]
        glm::vec3 cameraWorldPosition;
        float time;
        float _pad[3];
    };

    float DeltaTime() {return dt;};

    static sid::default_database& GetStringDB() {
        static sid::default_database s_defaultStringDatabase;
        return s_defaultStringDatabase;
    }

    JPH::TempAllocator* GetPhysicsAllocator() const {return tempAllocator;}
    JPH::JobSystem* GetJobSystem() const {return jobSystem;}

    vfspp::VirtualFileSystemPtr GetFilesystem();

    ScriptEngine* GetScriptEngine() {return scriptEngine.get();};
protected:
    SceneTree sceneTree;
    DeltaTicker deltaTime;
    float dt;
    bool bShouldClose = false;

    std::unique_ptr<ScriptEngine> scriptEngine;

    wgpu::Instance wgpuInstance = nullptr;
    wgpu::Device wgpuDevice = nullptr;


    int maxConcurrentJobs = std::thread::hardware_concurrency();		// How many jobs to run in parallel
    JPH::TempAllocator* tempAllocator;
    JPH::JobSystem* jobSystem;
    
    Application(const Application &) = delete;
    Application & operator = (const Application &) = delete;
private:
    Application();
    ~Application();
};



vfspp::VirtualFileSystemPtr Application::GetFilesystem() {
    return Filesystem::GetFilesystem();
}
