# Crab Engine

<img width="1242" height="378" alt="crabengineheader" src="https://github.com/user-attachments/assets/395fd302-0f01-41da-bd79-d7188f045b0e" />

Crab Engine is a hobby C++ game engine, created to explore various avenues of software architecture, rendering, language embedding, and more.

## Features
- WebGPU-Native 3D renderer
- Slang shader compilation
- Material system based on Slang modules and shader reflection
- .NET Core C# scripting
- .NET DLL hot reloading
- Jolt physics
- Custom reflection system
- Automatic property serialization and deserialization
- C++20 Modules

## Usage
#### Visit [Crab Application](https://github.com/Unbansheee/CrabApplication) to see an app built on Crab Engine, including C# scripting examples.

### Include via CMake
In your project's `CMakeLists.txt`:
```cmake
add_subdirectory(vendor/CrabEngine)
target_link_libraries(MyApp PRIVATE CrabEngine)

# Ensures CrabEngine dll dependencies, resources and extras are copied to the output dir
target_copy_crabengine_dependencies(CrabApplication)
target_copy_engine_resources(CrabApplication)

# Optional, allows embedding of C# into your project
add_dotnet_project(CrabApplication
        ${CMAKE_SOURCE_DIR}/script/CrabApplication/CrabApplication.csproj # C# Project Path
        $<TARGET_FILE_DIR:CrabApplication>/Dotnet) # C# Compilation DLL Output
```

### Using the Engine
In `main.cpp`:
```cpp
import Engine.Application;
import Engine.Node.Window;
import Engine.Filesystem;
import Engine.Resource.ResourceManager;
import Engine.Resource.Scene;
import Engine.Node.Camera3D;
import Engine.Node.MeshInstance3D;
import Engine.Resource.OBJMesh;
import Engine.Resource.Material;
import Engine.GFX.UniformDefinitions;

#define RESOURCE_DIR "./resources"

int main (int argc, char** argv) {
    // Initialize the engine
    Application& app = Application::Get();

    // Set up Resource directory
    Filesystem::AddFileSystemDirectory("/res", RESOURCE_DIR);

    // Create a Window node as the application's root node
    auto window = app.GetSceneTree().SetRoot(Node::NewNode<NodeWindow>("My Window"));

    // Setup resources
    auto material = std::make_shared<MaterialResource>("default_shader"); // Create new Material from code based on default_shader
    Uniforms::UStandardMaterialParameters params = {
        {1.f, 1.f, 1.f}, // Tint
        32.f, // Hardness
        1.f, // Diffuse contribution
        1.f, // Specular contribution
        1.f // Normal strength
    };
    material->SetUniform("uMaterialProperties", params);

    // Load texture from file
    auto texture = ResourceManager::Load<TextureResource>("/engine/null_texture_white.png"); 
    material->SetUniform("AlbedoTexture", texture);
    
    // Any additional nodes can be added to the application's root
    NodeMeshInstance3D* mesh = window->AddChild<NodeMeshInstance3D>();
    mesh->SetMesh(ResourceManager::Load<OBJMeshResource>("/engine/Shapes/cube.obj"));
    mesh->SetMaterial(material);
    mesh->SetOrientation(glm::angleAxis(deg_to_rad(45.f), Vector3{0.f, 0.f, 1.f}));

    // Create a camera angled downwards
    NodeCamera3D* camera = window->AddChild<NodeCamera3D>();
    camera->SetPosition({-5.f, 0.f, 2.f});
    camera->SetOrientation(glm::angleAxis(deg_to_rad(25.f), Vector3{0.f, 1.f, 0.f}));

    // Start the application's SceneTree
    app.Begin();

    // Update loop
    while (!app.ShouldClose())
    {
        if (window->WantsToClose())
        {
            app.Close();
        }
        app.Update();
    }

    return 0;
}
```

<img width="1282" height="992" alt="CrabApplication_qZ0i7wWPUd" src="https://github.com/user-attachments/assets/ceb199a0-aed6-48e6-9d6f-2182ac41eb97" />


We can also instantiate Scenes created with the CrabApplication editor:
```cpp
  auto sceneResource = ResourceManager::Load<SceneResource>("/res/SolarSystem.scene");
  window->AddChild(sceneResource->Instantiate());
```
