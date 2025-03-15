module;
#include <memory>

export module auto_registration;
import class_db;
import string;
export class ResourceImporter;

void AddImporterToImportManager(std::unique_ptr<ResourceImporter> imp);

export template<typename T>
struct AutoRegisterResourceImporter
{
    AutoRegisterResourceImporter()
    {
        AddImporterToImportManager(std::make_unique<T>());
    }
};

// Auto-registration helper
export template <typename T, typename ParentClass>
struct AutoRegister {
    AutoRegister(const std::string& Name) {
        ClassDB::Get().RegisterClass<T, ParentClass>(Name);
    }
};

// Auto-registration helper
export template <typename T>
struct AutoClassFlagRegister {
    AutoClassFlagRegister(uint32_t flags) {
        ClassDB::Get().AddClassFlag<T>(flags);
    }
};
