//
// Created by Vinnie on 30/04/2025.
//

module Engine.ScriptInstance;
import Engine.Application;

ScriptEngine * ScriptInstance::GetScriptEngine() {
    return Application::Get().GetScriptEngine();
}
