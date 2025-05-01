//
// Created by Vinnie on 30/04/2025.
//

module;

export module Engine.ScriptInstance;
import Engine.Reflection.Class;
import Engine.ScriptEngine;
import Engine.Variant;



export struct ScriptInstance {
     const ClassType* ScriptClass = nullptr;
     void* ManagedHandle = nullptr; // C# GCHandle
     ScriptInterop interop;

     template<typename ReturnValue, typename ...Args>
     ReturnValue Call(const std::wstring& method, Args... args) {
         if constexpr (std::is_same_v<ReturnValue, void>) {
             GetScriptEngine()->CallScriptMethod<ReturnValue>(ManagedHandle, method.c_str(), args...);
             return;
         }
         else {
             return GetScriptEngine()->CallScriptMethod<ReturnValue>(ManagedHandle, method.c_str(), args...);
         }
     }

    float GetFloat(const std::string& name);
    void SetFloat(const std::string& name, float value);

    template<typename T>
    void Set(const std::string& prop, const T& value) {
        interop.Set(ManagedHandle, prop.c_str(), (void*)&value);
    }

    template<typename T>
    T Get(const std::string& prop) {
        T outVal{};
        interop.Get(ManagedHandle, prop.c_str(), &outVal);
        return outVal;
    }

    static ScriptEngine* GetScriptEngine();
};

template<>
inline std::string ScriptInstance::Get<std::string>(const std::string& prop) {
    const int MaxStringSize = 512;  // or dynamically allocate if needed
    char buffer[MaxStringSize]{};

    interop.Get(ManagedHandle, prop.c_str(), buffer); // C# side must write UTF-8 or ANSI string
    return std::string(buffer);
}

template<>
inline void ScriptInstance::Set<std::string>(const std::string& prop, const std::string& value) {
    interop.Set(ManagedHandle, prop.c_str(), (void*)value.c_str());  // Null-terminated string
}