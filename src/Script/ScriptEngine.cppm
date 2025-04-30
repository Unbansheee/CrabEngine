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


export class ScriptEngine;
export struct ClassType;

struct ScriptInfo {
    const char* Name;
    const char* ParentClassName;
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

    std::list<ClassType> scriptClasses;
    std::unordered_map<std::wstring, void*> functionCache;
    std::wstring assemblyPath;
    std::wstring libName;
    ScriptEngine* engine;

    load_assembly_and_get_function_pointer_fn GetLoadAssemblyFn();
public:
    const ClassType* GetClass(const std::wstring& className) const;

    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallManaged(const std::wstring& className, const std::wstring& fnName, Args... args) {
        using ManagedFn = ReturnType(__stdcall*)(Args...);
        ManagedFn fn = nullptr;

        auto key = className + L"." + fnName;
        if (functionCache.contains(key)) {
            fn = static_cast<ManagedFn>(functionCache.at(key));
        }
        else {
            int rc = GetLoadAssemblyFn()(
            assemblyPath.c_str(),
            (className + L", " + libName).c_str(),
            fnName.c_str(),
            UNMANAGEDCALLERSONLY_METHOD,
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
        void** argArray = nullptr;
        int argCount = sizeof...(Args);

        if constexpr (sizeof...(Args) > 0) {
            static void* tempArgs[] = { (void*)&args... };
            argArray = tempArgs;
        }

        void* returnBuffer = nullptr;
        if constexpr (!std::is_void_v<ReturnType>) {
            returnBuffer = alloca(sizeof(ReturnType));
        }

        auto ret = CallManaged<void>(
            L"Scripts.ScriptHost",
            L"CallScriptMethod",
            managedHandle,
            methodName.c_str(),
            argArray,
            static_cast<int>(sizeof...(Args)),
            returnBuffer
        );

        if (!ret.SuccessfullyExecuted) return {false, {}};

        if constexpr (!std::is_void_v<ReturnType>) {
            return {true, *reinterpret_cast<ReturnType*>(returnBuffer)};
        }
        return {true, {}};
    }
};


export class ScriptEngine {
    friend class ScriptModule;

    hostfxr_initialize_for_runtime_config_fn init_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;
    load_assembly_and_get_function_pointer_fn load_assembly_fn = nullptr;
    std::vector<std::unique_ptr<ScriptModule>> loadedModules;

    bool LoadHostFXR();
    bool GetManagedDelegate(const std::wstring& runtimeConfigPath);

public:
    void Init();
    void LoadModule(const std::wstring &assembly, const std::wstring &libName);

    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallManaged(const std::wstring& className, const std::wstring& fnName, Args... args) {
        for (auto& module : loadedModules) {
            ScriptFunctionResult<ReturnType> res = module->CallManaged<ReturnType>(className, fnName, args...);
            if (res.SuccessfullyExecuted) {
                return res;
            }
        }
        return {false, {}};
    }

    template<typename ReturnType = void, typename... Args>
    ScriptFunctionResult<ReturnType> CallScriptMethod(void* managedHandle, const std::wstring& methodName, Args&&... args) {
        for (auto& module : loadedModules) {
            ScriptFunctionResult<ReturnType> res = module->CallScriptMethod(managedHandle, methodName, args...);
            if (res.SuccessfullyExecuted) {
                return res;
            }
        }
        return {false, {}};
    }
};





