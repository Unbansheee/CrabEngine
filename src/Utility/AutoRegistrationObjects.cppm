﻿module;

export module Engine.Reflection.AutoRegistration;
import Engine.Reflection.ClassDB;
import std;
import std.compat;
export class ResourceImporter;
export struct ClassType;

void AddImporterToImportManager(std::unique_ptr<ResourceImporter> imp);
void AddFlagToClassType(ClassType& type, uint32_t flag);

// Static initializers for reflection registration

export template<typename T>
struct AutoRegisterResourceImporter
{
    AutoRegisterResourceImporter()
    {
        AddImporterToImportManager(std::make_unique<T>());
    }
};

// Auto-registration helper
export struct AutoClassRegister {
    AutoClassRegister(const ClassType& classType);
};

// Auto-registration helper
export template <typename T>
struct AutoClassFlagRegister {
    AutoClassFlagRegister(uint32_t flags) {
        const ClassType& type = T::GetStaticClass();
        ClassType& unconst = const_cast<ClassType&>(type);
        AddFlagToClassType(unconst, flags);
    }
};

export template<typename T, typename... Args>
struct MethodRegister {
    MethodRegister(const std::string& name, void* method) {
        const_cast<ClassType&>(T::GetStaticClass()).methodTable[name] = method;
    }
};