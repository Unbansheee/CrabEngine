//
// Created by Vinnie on 30/04/2025.
//

module Engine.ScriptInstance;
import Engine.Application;
import Engine.ScriptEngine;


ScriptInstance::~ScriptInstance() {
    if (ManagedHandle) {
        Application::Get().GetScriptEngine()->CallManaged(L"CrabEngine.ScriptHost", L"DestroyScript", ManagedHandle);
        Module->UnregisterInstance(this);
    }
}

ScriptInstance::ScriptInstance(Object *owner, const ClassType *classType, void *managedHandle,
    const ScriptInterop &interop): ScriptClass(classType), ManagedHandle(managedHandle), interop(interop) {
    Module = classType->ScriptModule;
    InstanceOwner = owner;
}

ScriptEngine * ScriptInstance::GetScriptEngine() {
    return Application::Get().GetScriptEngine();
}
