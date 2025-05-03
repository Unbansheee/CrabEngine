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
import Engine.Object;
import Engine.Reflection.Class;
import Engine.Resource;
import Engine.Filesystem;
import Engine.ScriptInstance;
import Engine.Filesystem;

std::string runtimeConfig = "/dotnet/CrabEngine.runtimeconfig.json";
std::string engineAssembly = "/dotnet/CrabEngine.dll";


class DLLListener : public efsw::FileWatchListener {
public:
    DLLListener(ScriptEngine* engine) : scriptEngine(engine) {  } ;
    ScriptEngine* scriptEngine;

    void handleFileAction( efsw::WatchID watchid, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename ) override {

        if (action == efsw::Actions::Modified) {
            std::filesystem::path item = dir.substr(0, dir.length()-1);
            item /= filename;
            if (item.extension() == ".dll") {
                scriptEngine->EnqueueModuleReload(item.generic_wstring());
            }
        }
    }
};

void ScriptEngine::Init() {
    if (!LoadHostFXR()) {
        std::cerr << "Failed to load hostfxr." << std::endl;
        return;
    }

    if (!GetManagedDelegate(Filesystem::StringToWString(Filesystem::AbsolutePath(runtimeConfig)))) {
        std::cerr << "Failed to initialize .NET runtime." << std::endl;
        return;
    }

    load_assembly(Filesystem::StringToWString(Filesystem::AbsolutePath(engineAssembly)).c_str(), nullptr, nullptr);

    void* regFn = nullptr;
    int rc1 = get_fn_ptr(
        L"CrabEngine.ScriptHost, CrabEngine",
        L"LoadScriptAssembly",
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr, nullptr, &regFn);

    void* unRegFn = nullptr;
    int rc2 = get_fn_ptr(
        L"CrabEngine.ScriptHost, CrabEngine",
        L"UnloadScriptAssembly",
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr, nullptr, &unRegFn);

    void* createFn = nullptr;
    int rc3 = get_fn_ptr(
        L"CrabEngine.ScriptHost, CrabEngine",
        L"CreateScriptInstance",
        UNMANAGEDCALLERSONLY_METHOD,
        nullptr, nullptr, &createFn);

    if (rc1 != 0 || rc2 != 0 || rc3 != 0 || !regFn || !createFn || !unRegFn) {
        std::cerr << "Failed to get function pointers!" << std::endl;
        return;
    }

    RegisterScriptAssembly = reinterpret_cast<RegisterScriptAssemblyFn>(regFn);
    CreateScriptInstanceManaged = reinterpret_cast<CreateScriptInstanceFn>(createFn);
    UnregisterScriptAssembly = reinterpret_cast<UnregisterScriptAssemblyFn>(unRegFn);

    // Bind native class functions
    for (auto& classType : ClassDB::Get().GetClasses()) {
        if (!classType->HasFlag(ClassFlags::ScriptClass)) {
            for (auto& [name, func] : classType->methodTable) {
                std::string className = classType->Name.string();
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::wstring wideClass = converter.from_bytes(className);
                std::wstring wideFn = converter.from_bytes(name);
                CallManaged<void>(L"CrabEngine.ScriptHost", L"RegisterNativeFunction", (L"CrabEngine." + wideClass).c_str(), wideFn.c_str(), func);
            }
        }
    }

    // TODO: Manage memory on this
    DLLListener* listener = new DLLListener(this);
    fileWatcher = std::make_unique<efsw::FileWatcher>();
    efsw::WatchID watchID = fileWatcher->addWatch( Filesystem::AbsolutePath("/dotnet/"), listener, true );
    fileWatcher->watch();
}

void ScriptEngine::EnqueueModuleReload(const std::wstring &path) {
    std::lock_guard<std::mutex> lock(reloadMutex);
    pendingReloads.push(path);
}

void ScriptEngine::ProcessReloadQueue() {
    std::queue<std::wstring> localQueue;

    // Move all pending reloads to a local queue
    {
        std::lock_guard<std::mutex> lock(reloadMutex);
        std::swap(localQueue, pendingReloads);
    }

    // Process each reload safely on the main thread
    while (!localQueue.empty()) {
        const std::wstring& path = localQueue.front();
        ReloadModule(path);  // safe to call here
        localQueue.pop();
    }
}

void ScriptEngine::LoadModule(const std::wstring &assembly, const std::wstring& libName) {
    RegisterScriptAssembly(assembly.c_str(), libName.c_str());
    loadedModules.insert({assembly, std::make_unique<ScriptModule>(this, assembly, libName)});
}

std::vector<Object*> ScriptEngine::UnloadModule(const std::wstring &assembly) {
    if (loadedModules.contains(assembly)) {
        auto invalidatedObjects = loadedModules[assembly]->Unload();
        loadedModules.erase(assembly);
        return invalidatedObjects;
    }
    else {
        std::wcerr << "Failed to unload " << assembly <<". No modules loaded with this path. Ensure you use the exact path. Current modules: \n";
        for (auto& [modname, mod] : loadedModules) {
            std::wcerr << modname << "\n";
        }
        std::wcerr << std::endl;
    }
    return {};
}


void ScriptEngine::ReloadModule(const std::wstring &assembly) {
    std::wcout << "Reloading module: " << assembly << std::endl;
    auto libName = loadedModules.at(assembly)->libName;
    auto invalidatedObjects = UnloadModule(assembly);
    LoadModule(assembly, libName);

    for (auto object : invalidatedObjects) {
        object->ReloadScriptInstance();
    }
    std::wcout << "Reloaded module: " << assembly << std::endl;
}

std::unique_ptr<ScriptInstance> ScriptEngine::CreateScriptInstance(Object *instanceOwner, const ClassType *type) {
    std::string name = type->Name.string();
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(name);

    auto ManagedHandle = *(CallManaged<void*>(L"CrabEngine.ScriptHost", L"CreateScriptInstance", (void*)instanceOwner, wide.c_str()));
    auto interop = CallManaged<ScriptInterop>(L"CrabEngine.InteropBootstrap", L"GetInterop").Result.value();
    auto module = type->ScriptModule;
    auto inst = std::make_unique<ScriptInstance>(instanceOwner, type, ManagedHandle, interop);
    module->RegisterInstance(inst.get()); // TODO: Move this all to module code?
    return inst;
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
    //LoadAssemblyFn()(assemblyPath.c_str(), nullptr, nullptr);
    //CallManaged(L"CrabEngine.ScriptHost", L"RegisterAllScripts");

    int count = *scriptEngine->CallManaged<int>(L"CrabEngine.ScriptHost", L"GetScriptCount", assembly.c_str());
    auto cresult = scriptEngine->CallManaged<ScriptInfo*>(L"CrabEngine.ScriptHost", L"GetScriptInfoList", assembly.c_str());
    ScriptInfo* script_info = cresult.Result.value();
    for (int i = 0; i < count; i++) {
        scriptClasses.emplace_back();
        ClassType& t = scriptClasses.back();
        t.Flags |= ClassFlags::ScriptClass | ClassFlags::EditorVisible;
        t.Name = MakeStringID(script_info[i].Name);
        t.Parent = MakeStringID(script_info[i].ParentClassName);
        t.ScriptModule = this;

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

    for (auto& classType : scriptClasses) {
        classType.Initializer = [&] {
            auto obj = ClassDB::Get().GetNativeType(&classType)->Initializer();
            std::string name = {classType.Name.string()};

            obj->scriptInstance = engine->CreateScriptInstance(obj, &classType);
            obj->scriptTypeName = name;

            return obj;
        };
    }
}

std::vector<Object *> ScriptModule::GetRegisteredObjects() {
    std::vector<Object *> objects;
    for (auto& script : registeredScripts) {
        objects.push_back(script->InstanceOwner);
    }

    return objects;
}

ScriptModule::~ScriptModule() {
}

std::vector<Object*> ScriptModule::Unload() {
    for (auto& type : scriptClasses) {
        ClassDB::Get().UnregisterClassType(type);
    }
    engine->UnregisterScriptAssembly(assemblyPath.c_str());

    std::vector<Object*> objects;

    std::list<ScriptInstance*>::iterator i = registeredScripts.begin();

    unloading = true;
    while (i != registeredScripts.end()) {
        objects.push_back((*i)->InstanceOwner);
        (*i)->InstanceOwner->InvalidateScriptInstance();
        i = registeredScripts.erase(i);
    }
    unloading = false;

    registeredScripts.clear();

    return objects;
}

void ScriptModule::RegisterInstance(ScriptInstance *inst) {
    registeredScripts.push_back(inst);
}

void ScriptModule::UnregisterInstance(ScriptInstance *inst) {
    if (!unloading)
        registeredScripts.erase(std::ranges::find(registeredScripts, inst));
}

load_assembly_fn ScriptModule::LoadAssemblyFn() {
    return engine->load_assembly;
}

get_function_pointer_fn ScriptModule::GetFunctionPointerFn() {
    return engine->get_fn_ptr;
}
