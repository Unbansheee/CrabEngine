//
// Created by Vinnie on 29/04/2025.
//

module;
#include <windows.h>
#include <hostfxr.h>
#include <nethost.h>
#include <coreclr_delegates.h>
#pragma comment(lib, "nethost.lib")

export module Engine.ScriptEngine;
import std;
import Engine.Assert;
import Engine.Reflection.Class;

using namespace std;

struct ScriptInfo {
    const char* Name;
    const char* ParentClassName;
};


export class ScriptEngine {
    hostfxr_initialize_for_runtime_config_fn init_fptr;
    hostfxr_get_runtime_delegate_fn get_delegate_fptr;
    hostfxr_close_fn close_fptr;

    load_assembly_and_get_function_pointer_fn load_assembly_fn = nullptr;



    std::wstring runtimeConfig = L"Dotnet/Scripts.runtimeconfig.json";
    std::wstring assemblyPath = L"Dotnet/Scripts.dll";

    std::unordered_map<std::wstring, void*> functionCache;

    std::list<ClassType> scriptClasses{};

    bool LoadHostFXR();
    bool GetManagedDelegate(const std::wstring& runtimeConfigPath);

public:
    void Init();

    template<typename ReturnType = void, typename... Args>
    ReturnType CallManaged(const std::wstring& className, const std::wstring& fnName, Args... args) {
        using ManagedFn = ReturnType(__stdcall*)(Args...);
        ManagedFn fn = nullptr;

        auto key = className + L"." + fnName;
        if (functionCache.contains(key)) {
            fn = static_cast<ManagedFn>(functionCache.at(key));
        }
        else {
            int rc = load_assembly_fn(
            assemblyPath.c_str(),
            (className + L", Scripts").c_str(),
            fnName.c_str(),
            UNMANAGEDCALLERSONLY_METHOD,
            nullptr,
            (void**)&fn
        );

            if (rc != 0 || !fn) {
                std::wcerr << "Failed to load managed method: " << className << "." << fnName << "()" << ". HRESULT: 0x" << std::hex << rc << std::endl;
                if constexpr (std::is_same_v<ReturnType, void>) {
                    return;
                }
                else {
                    return ReturnType();
                }
            }

            functionCache.insert({key, static_cast<void*>(fn)});
        }

        if constexpr (std::is_same_v<ReturnType, void>) {
            fn(args...);
            return;
        }
        else {
            return fn(args...);
        }
    }

};

