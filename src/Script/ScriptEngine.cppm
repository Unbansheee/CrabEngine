//
// Created by Vinnie on 29/04/2025.
//

module;
#include <windows.h>
#include <hostfxr.h>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <malloc.h>
#pragma comment(lib, "nethost.lib")

export module Engine.ScriptEngine;
import Engine.Assert;
import std;
import Engine.Reflection;
import Engine.Variant;
import efsw;


export struct ScriptInstance;
export class ScriptEngine;

struct ScriptPropertyInfo {
    const char* Name;
    const char* Type;
    const char* DisplayName;
};

struct ScriptInfo {
    const char* Name;
    const char* ParentClassName;
    int PropertyCount;
    ScriptPropertyInfo* Properties;
};

typedef void (*SetPropertyFunc)(void* instanceHandle, const char* propName, void* value);
typedef void (*GetPropertyFunc)(void* instanceHandle, const char* propName, void* outValue);

// Setter and getter functions for scripts
export struct ScriptInterop {
    SetPropertyFunc Set;
    GetPropertyFunc Get;
};

export template<typename T = void>
struct ScriptFunctionResult {
    using InternalResultType = std::conditional_t<std::is_void_v<T>, bool, T>;
    bool SuccessfullyExecuted = false;
    std::optional<InternalResultType> Result = {};

    InternalResultType& operator* ()
    {
        return Result.value();
    }

    InternalResultType& operator-> ()
    {
        return Result.value();
    }
};

// Represents a loaded dll
export class ScriptModule {
public:
    ScriptModule(ScriptEngine* scriptEngine, const std::wstring& assemblyPath, const std::wstring& libName);
    ~ScriptModule();
    ScriptModule(const ScriptModule& other) = delete;

    // Unloads the module and returns a list of Objects that were actively using types from this module and have been invalidated
    std::vector<Object*> Unload();

    // Scripts should register to the module when created and unregister when deleted
    // This is so when the module is unloaded, any objects using its scripts can be invalidated
    void RegisterInstance(ScriptInstance* inst);
    void UnregisterInstance(ScriptInstance* inst);
    std::vector<Object*> GetRegisteredObjects();

    const std::wstring& GetName() { return libName; }
    const std::wstring& GetPath() { return assemblyPath; }

private:
    std::list<ScriptInstance*> registeredScripts{};
    // All ClassTypes from this module are stored with the module
    std::list<ClassType> scriptClasses;
    // Path to the dll
    std::wstring assemblyPath;
    // Library name, for our reference
    std::wstring libName;

    ScriptEngine* engine;

    // Is currently unloading flag
    bool unloading = false;

    load_assembly_fn LoadAssemblyFn();
    get_function_pointer_fn GetFunctionPointerFn();
public:
    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallScriptMethod(void* managedHandle, const std::wstring& methodName, Args&&... args);
};

// Handles loading of hostfxr and coreclr
// Contains all loaded script modules
// Represents CrabEngine.dll
export class ScriptEngine {
    friend class ScriptModule;
    using RegisterScriptAssemblyFn = void(*)(const wchar_t*, const wchar_t*);
    using CreateScriptInstanceFn = void(*)(void* nativePtr, const wchar_t* typeName);
    using UnregisterScriptAssemblyFn = void(*)(const wchar_t*);

    // hostfxr and coreclr functions
    hostfxr_initialize_for_runtime_config_fn init_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;
    load_assembly_fn load_assembly;
    get_function_pointer_fn get_fn_ptr;

    // CrabEngine.dll method pointers (TODO: use these instead of calling by string all the time)
    RegisterScriptAssemblyFn RegisterScriptAssembly = nullptr;
    UnregisterScriptAssemblyFn UnregisterScriptAssembly = nullptr;
    CreateScriptInstanceFn CreateScriptInstanceManaged = nullptr;

    std::unordered_map<std::wstring, std::unique_ptr<ScriptModule>> loadedModules;
    std::unordered_map<std::wstring, void*> functionCache;

    bool LoadHostFXR();
    // Initializes CoreCLR runtime
    bool GetManagedDelegate(const std::wstring& runtimeConfigPath);

public:
    ScriptEngine(const ScriptEngine& other) = delete;
    ScriptEngine() = default;

    void Init();

    // Queue a dll path to be reloaded next frame of the event loop
    void EnqueueModuleReload(const std::wstring& path);
    // Should be called on the engine event loop. Polls reload queue and reloads any changed dlls
    void ProcessReloadQueue();

    // Loads a DLL as a module using an absolute path
    void LoadModule(const std::wstring &assembly, const std::wstring &libName);
    // Unloads a DLL module. Must use the path it was loaded with
    std::vector<Object*> UnloadModule(const std::wstring &assembly);

    // Gets all loaded C# modules
    const std::unordered_map<std::wstring, std::unique_ptr<ScriptModule>>& GetModules();

    // Create a managed script instance for an object
    std::unique_ptr<ScriptInstance> CreateScriptInstance(Object* instanceOwner, const ClassType* type);

    // Calls a managed function from CrabEngine.dll. The function cannot be from a module
    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallManaged(const std::wstring& className, const std::wstring& fnName, Args... args);

    // Calls a method on a Managed script object.
    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallScriptMethod(void* managedHandle, const std::wstring& methodName, Args&&... args);
private:
    // Called by ProcessReloadQueue
    void ReloadModule(const std::wstring &assembly);

    // Hot reloading
    std::unique_ptr<efsw::FileWatcher> fileWatcher;
    std::mutex reloadMutex;
    std::queue<std::wstring> pendingReloads;

private:
    std::optional<Property> CreateScriptProperty(const ScriptPropertyInfo& info);

};

const std::unordered_map<std::wstring, std::unique_ptr<ScriptModule>>& ScriptEngine::GetModules() {
    return loadedModules;
}


template<typename ReturnType, typename ... Args>
ScriptFunctionResult<ReturnType> ScriptEngine::CallManaged(const std::wstring &className, const std::wstring &fnName,
    Args... args) {
    using ManagedFn = ReturnType(__cdecl*)(Args...);
    ManagedFn fn = nullptr;

    auto key = className + L"." + fnName;
    if (functionCache.contains(key)) {
        fn = static_cast<ManagedFn>(functionCache.at(key));
    }
    else {
        int rc = get_fn_ptr(
            (className + L", CrabEngine").c_str(),
            fnName.c_str(),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            nullptr,
            (void**)&fn
        );

        if (rc != 0 || !fn) {
            std::wcerr << "Failed to load managed method: " << className << "." << fnName << "()" << ". HRESULT: 0x" << std::hex << rc << std::endl;
            return { false, {} };
        }

        functionCache.insert({key, static_cast<void*>(fn)});
    }

    if constexpr (std::is_same_v<ReturnType, void>) {
        fn(args...);
        return {true, {}};
    }
    else {
        return {true, fn(args...)};
    }
}

template<typename ReturnType, typename ... Args>
ScriptFunctionResult<ReturnType> ScriptEngine::CallScriptMethod(void *managedHandle, const std::wstring &methodName,
    Args &&...args) {
    constexpr bool HasReturn = !std::is_void_v<ReturnType>;

    // Allocating argument buffers safely to prevent garbage data
    std::vector<std::unique_ptr<std::byte[]>> ownedArgs;
    std::vector<void*> argArray;

    auto pushArg = [&](auto&& arg) {
        using ArgType = std::decay_t<decltype(arg)>;
        auto buffer = std::make_unique<std::byte[]>(sizeof(ArgType));
        std::memcpy(buffer.get(), &arg, sizeof(ArgType));
        argArray.push_back(buffer.get());
        ownedArgs.push_back(std::move(buffer));
    };

    (pushArg(std::forward<Args>(args)), ...);

    void* returnBuffer = nullptr;
    std::unique_ptr<std::byte[]> returnStorage;

    if constexpr (HasReturn) {
        returnStorage = std::make_unique<std::byte[]>(sizeof(ReturnType));
        returnBuffer = returnStorage.get();
    }

    // Call into CrabEngine.dll to trigger a script method call on the current handle
    auto result = CallManaged<void>(
        L"CrabEngine.ScriptHost",
        L"CallScriptMethod",
        managedHandle,
        methodName.c_str(),
        argArray.data(),
        static_cast<int>(argArray.size()),
        returnBuffer
    );

    if (!result.SuccessfullyExecuted)
        return {false, {}};

    if constexpr (HasReturn) {
        // TODO: this could be done better
        ReturnType ret{};
        std::memcpy(&ret, returnBuffer, sizeof(ReturnType));
        return {true, ret};
    } else {
        return {true, {}};
    }
}
