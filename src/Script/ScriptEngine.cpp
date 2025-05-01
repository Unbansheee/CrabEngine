//
// Created by Vinnie on 30/04/2025.
//
module;
#include <windows.h>
#include <hostfxr.h>
#include <nethost.h>
#include <coreclr_delegates.h>

module Engine.ScriptEngine;
import Engine.Reflection.ClassDB;
import Engine.ScriptInstance;
import Engine.Object;
import Engine.Reflection.Class;
import Engine.Resource;

std::wstring runtimeConfig = L"Dotnet/Scripts.runtimeconfig.json";

void ScriptEngine::Init() {
    if (!LoadHostFXR()) {
        std::cerr << "Failed to load hostfxr." << std::endl;
        return;
    }

    if (!GetManagedDelegate(runtimeConfig)) {
        std::cerr << "Failed to initialize .NET runtime." << std::endl;
        return;
    }
}

void ScriptEngine::LoadModule(const std::wstring &assembly, const std::wstring& libName) {
    loadedModules.emplace_back(std::make_unique<ScriptModule>(this, assembly, libName));
}

bool ScriptEngine::LoadHostFXR() {
    char_t buffer[MAX_PATH];
    size_t size = sizeof(buffer) / sizeof(char_t);
    int rc = get_hostfxr_path(buffer, &size, nullptr);

    if (rc != 0) {
        std::cerr << "Failed to locate hostfxr.dll" << std::endl;
        return false;
    }

    HMODULE lib = LoadLibraryW(buffer);
    if (!lib) return false;

    init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(lib, "hostfxr_initialize_for_runtime_config");
    get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(lib, "hostfxr_get_runtime_delegate");
    close_fptr = (hostfxr_close_fn)GetProcAddress(lib, "hostfxr_close");

    return init_fptr && get_delegate_fptr && close_fptr;
}

// Load CoreCLR and get delegate
bool ScriptEngine::GetManagedDelegate(const std::wstring& runtimeConfigPath) {
    void* context = nullptr;
    int rc = init_fptr(runtimeConfigPath.c_str(), nullptr, &context);
    if (rc != 0 || context == nullptr) return false;

    rc = get_delegate_fptr(context, hdt_load_assembly_and_get_function_pointer, (void**)&load_assembly_fn);
    if (rc != 0 || load_assembly_fn == nullptr) return false;

    close_fptr(context);
    return true;
}

ScriptModule::ScriptModule(ScriptEngine *scriptEngine, const std::wstring &assembly, const std::wstring& libraryName): engine(scriptEngine), assemblyPath(assembly), libName(libraryName) {
    CallManaged(L"Scripts.ScriptHost", L"RegisterAllScripts");
    int count = *CallManaged<int>(L"Scripts.ScriptHost", L"GetScriptCount");
    ScriptInfo* script_info = *CallManaged<ScriptInfo*>(L"Scripts.ScriptHost", L"GetScriptInfoList");
    for (int i = 0; i < count; i++) {
        std::cout << script_info[i].Name << std::endl;


        scriptClasses.emplace_back();
        auto& t = scriptClasses.back();
        t.Flags |= ClassFlags::ScriptClass | ClassFlags::EditorVisible;
        t.Name = MakeStringID(script_info[i].Name);
        t.Parent = MakeStringID(script_info[i].ParentClassName);
        ClassDB::Get().RegisterClassType(t);
    }

    // Bind native class functions
    for (auto& classType : ClassDB::Get().GetClasses()) {
        if (!classType->HasFlag(ClassFlags::ScriptClass)) {
            for (auto& [name, func] : classType->methodTable) {
                std::string className = classType->Name.string();
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::wstring wideClass = converter.from_bytes(className);
                std::wstring wideFn = converter.from_bytes(name);
                CallManaged<void>(L"Scripts.ScriptHost", L"RegisterNativeFunction", (L"Scripts." + wideClass).c_str(), wideFn.c_str(), func);
            }
        }
    }

    for (auto& classType : scriptClasses) {
        classType.Initializer = [&] {
            auto obj = ClassDB::Get().GetNativeType(&classType)->Initializer();
            ScriptInstance inst;
            inst.ScriptClass = &classType;
            std::string name = {classType.Name.string()};
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::wstring wide = converter.from_bytes(name);

            inst.ManagedHandle = *CallManaged<void*>(L"Scripts.ScriptHost", L"CreateScriptInstance", (void*)obj, wide.c_str());
            obj->scriptInstance = inst;

            return obj;
        };
    }
}

load_assembly_and_get_function_pointer_fn ScriptModule::GetLoadAssemblyFn() {
    return engine->load_assembly_fn;
}

const ClassType * ScriptModule::GetClass(const std::wstring &className) const {
    //for (auto& c : scriptClasses) {
    //    if (c.Name == MakeStringID(className)) return &c;
    //}
    return nullptr;
}
