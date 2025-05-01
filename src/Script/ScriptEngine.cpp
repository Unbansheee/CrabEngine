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

std::wstring StringToWString(const std::string& str)
{
    std::wstring wstr;
    size_t size;
    wstr.resize(str.length());
    mbstowcs_s(&size,&wstr[0],wstr.size()+1,str.c_str(),str.size());
    return wstr;
}

std::string WStringToString(const std::wstring& wstr)
{
    std::string str;
    size_t size;
    str.resize(wstr.length());
    wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
    return str;
}


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
    loadedModules.insert({assembly, std::make_unique<ScriptModule>(this, assembly, libName)});
}

void ScriptEngine::UnloadModule(const std::wstring &assembly) {
    loadedModules.erase(assembly);
}


void ScriptEngine::ReloadModule(const std::wstring &assembly) {
    std::wcout << "Reloading module: " << assembly << std::endl;
    auto libName = loadedModules.at(assembly)->libName;
    UnloadModule(assembly);
    LoadModule(assembly, libName);
    std::wcout << "Reloaded module: " << assembly << std::endl;
}


std::optional<Property> ScriptEngine::CreateScriptProperty(const ScriptPropertyInfo &info) {
    std::string name = info.Name;
    std::string displayName = info.DisplayName;
    std::string type = info.Type;

    uint32_t flags = PropertyFlags::ScriptProperty;

    // Use some form of type registry or mapping to resolve the type
    if (type == "System.Single" || type == "float") {
        return Property(
            name, displayName, "ScriptClass",
            // getter
            std::function<ValueVariant(void *)>(
            // getter
            [=](void *obj) -> ValueVariant {
                return static_cast<Object*>(obj)->GetScriptInstance()->Get<float>(name);
            }),
            // setter
            [=](void* obj, const ValueVariant& val) {
                static_cast<Object*>(obj)->GetScriptInstance()->Set<float>(name, std::get<float>(val));
            },
            nullptr,
            flags
        );
    }
    else if (type == "System.Int32" || type == "int") {
        return Property(
            name, displayName, "ScriptClass",
            std::function<ValueVariant(void *)>([=](void *obj) -> ValueVariant {
                return static_cast<Object *>(obj)->GetScriptInstance()->Get<int>(name);
            }),
            [=](void *obj, const ValueVariant &val) {
                static_cast<Object*>(obj)->GetScriptInstance()->Set<int>(name, std::get<int>(val));
            },
            nullptr, flags
        );
    }
    else if (type == "System.Boolean" || type == "bool") {
        return Property(
            name, displayName, "ScriptClass",
            std::function<ValueVariant(void *)>([=](void *obj) -> ValueVariant {
                return static_cast<Object *>(obj)->GetScriptInstance()->Get<bool>(name);
            }),
            [=](void *obj, const ValueVariant &val) {
                static_cast<Object*>(obj)->GetScriptInstance()->Set<bool>(name, std::get<bool>(val));
            },
            nullptr, flags
        );
    }
    else if (type == "System.String" || type == "string") {
        return Property(
            name, displayName, "ScriptClass",
            std::function<ValueVariant(void *)>([=](void *obj) -> ValueVariant {
                return static_cast<Object *>(obj)->GetScriptInstance()->Get<std::string>(name);
            }),
            [=](void *obj, const ValueVariant &val) {
                static_cast<Object*>(obj)->GetScriptInstance()->Set<std::string>(name, std::get<std::string>(val));
            },
            nullptr, flags
        );
    }

    return {};

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

    rc = get_delegate_fptr(context, hdt_load_assembly, (void**)&load_assembly);
    if (rc != 0 || load_assembly == nullptr) return false;

    rc = get_delegate_fptr(context, hdt_get_function_pointer, (void**)&get_fn_ptr);
    if (rc != 0 || get_fn_ptr == nullptr) return false;

    close_fptr(context);
    return true;
}

ScriptModule::ScriptModule(ScriptEngine *scriptEngine, const std::wstring &assembly, const std::wstring& libraryName): engine(scriptEngine), assemblyPath(assembly), libName(libraryName) {
    LoadAssemblyFn()(assemblyPath.c_str(), nullptr, nullptr);

    CallManaged(L"Scripts.ScriptHost", L"RegisterAllScripts");

    int count = *CallManaged<int>(L"Scripts.ScriptHost", L"GetScriptCount");
    auto cresult = CallManaged<ScriptInfo*>(L"Scripts.ScriptHost", L"GetScriptInfoList");
    ScriptInfo* script_info = cresult.Result.value();
    for (int i = 0; i < count; i++) {
        scriptClasses.emplace_back();
        ClassType& t = scriptClasses.back();
        t.Flags |= ClassFlags::ScriptClass | ClassFlags::EditorVisible;
        t.Name = MakeStringID(script_info[i].Name);
        t.Parent = MakeStringID(script_info[i].ParentClassName);

        std::string className = t.Name.string();
        std::string parent = t.Parent.string();

        if (auto parentClass = ClassDB::Get().GetClassByName(parent)) {
            for (auto& prop : parentClass->Properties) {
                t.Properties.push_back(prop);
            }
        }

        for (int j = 0; j < script_info[i].PropertyCount; j++) {
            auto& prop = script_info[i].Properties[j];
            auto createdProp = engine->CreateScriptProperty(script_info[i].Properties[j]);
            if (createdProp.has_value()) {
                createdProp->ownerClass = className;
                t.Properties.push_back(createdProp.value());
            }
        }

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
            inst.interop = engine->CallManaged<ScriptInterop>(L"Scripts.InteropBootstrap", L"GetInterop").Result.value();
            obj->scriptInstance = inst;

            return obj;
        };
    }
}

ScriptModule::~ScriptModule() {
    for (auto& type : scriptClasses) {
        ClassDB::Get().UnregisterClassType(type);
    }
}

load_assembly_fn ScriptModule::LoadAssemblyFn() {
    return engine->load_assembly;
}

get_function_pointer_fn ScriptModule::GetFunctionPointerFn() {
    return engine->get_fn_ptr;
}
