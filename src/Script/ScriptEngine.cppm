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


struct ScriptInstance;
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

class ScriptModule {
public:
    ScriptModule(ScriptEngine* scriptEngine, const std::wstring& assemblyPath, const std::wstring& libName);
    ~ScriptModule();

    std::list<ClassType> scriptClasses;

    std::wstring assemblyPath;
    std::wstring libName;
    ScriptEngine* engine;

    load_assembly_fn LoadAssemblyFn();
    get_function_pointer_fn GetFunctionPointerFn();
public:
    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallScriptMethod(void* managedHandle, const std::wstring& methodName, Args&&... args);
};

export class ScriptEngine {

    friend class ScriptModule;
    using RegisterScriptAssemblyFn = void(*)(const wchar_t*, const wchar_t*);
    using CreateScriptInstanceFn = void(*)(void* nativePtr, const wchar_t* typeName);

    hostfxr_initialize_for_runtime_config_fn init_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;
    load_assembly_fn load_assembly;
    get_function_pointer_fn get_fn_ptr;

    RegisterScriptAssemblyFn RegisterScriptAssembly = nullptr;
    CreateScriptInstanceFn CreateScriptInstance = nullptr;

    std::unordered_map<std::wstring, std::unique_ptr<ScriptModule>> loadedModules;
    std::unordered_map<std::wstring, void*> functionCache;

    bool LoadHostFXR();
    bool GetManagedDelegate(const std::wstring& runtimeConfigPath);

public:
    ScriptEngine(const ScriptEngine& other) = delete;
    ScriptEngine() = default;

    void Init();
    void LoadModule(const std::wstring &assembly, const std::wstring &libName);
    void UnloadModule(const std::wstring &assembly);
    void ReloadModule(const std::wstring &assembly);


    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallManaged(const std::wstring& className, const std::wstring& fnName, Args... args) {
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

    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallScriptMethod(void* managedHandle, const std::wstring& methodName, Args&&... args) {
        for (auto &module: loadedModules | std::views::values) {
            ScriptFunctionResult<ReturnType> res = module->CallScriptMethod(managedHandle, methodName, args...);
            if (res.SuccessfullyExecuted) {
                return res;
            }
        }
        return {false, {}};
    }

private:
    std::optional<Property> CreateScriptProperty(const ScriptPropertyInfo& info);

};




template<typename ReturnType, typename ... Args>
ScriptFunctionResult<ReturnType> ScriptModule::CallScriptMethod(void *managedHandle, const std::wstring &methodName,
    Args &&...args) {
    constexpr bool HasReturn = !std::is_void_v<ReturnType>;

    // Allocate argument buffers safely
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

    // Call the managed shim
    auto result = engine->CallManaged<void>(
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
        ReturnType ret{};
        std::memcpy(&ret, returnBuffer, sizeof(ReturnType));
        return {true, ret};
    } else {
        return {true, {}};
    }
}
