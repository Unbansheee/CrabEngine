//
// Created by Vinnie on 30/04/2025.
//

module;

export module Engine.ScriptInstance;
import Engine.Reflection.Class;
import Engine.ScriptEngine;

export struct ScriptInstance {
     const ClassType* ScriptClass = nullptr;
     void* ManagedHandle = nullptr; // C# GCHandle

     template<typename ReturnValue, typename ...Args>
     ReturnValue Call(const std::wstring& method, Args... args) {
         if constexpr (std::is_same_v<ReturnValue, void>) {
             GetScriptEngine()->CallManaged<ReturnValue>(L"Scripts.ScriptHost", L"CallMethod", ManagedHandle, method.c_str(), args...);
             return;
         }
         else {
             return GetScriptEngine()->CallManaged<ReturnValue>(L"Scripts.ScriptHost", L"CallMethod", ManagedHandle, method.c_str(), args...);
         }
     }

    static ScriptEngine* GetScriptEngine();
};
