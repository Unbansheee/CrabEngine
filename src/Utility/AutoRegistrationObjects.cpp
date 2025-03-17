#include <memory>

module auto_registration;
import import_manager;
import resource_importer;
import class_type;

void AddImporterToImportManager(std::unique_ptr<ResourceImporter> imp)
{
    ImportManager::Get().RegisterImporter(std::move(imp));
}

void AddFlagToClassType(ClassType& type, uint32_t flag)
{
    type.AddFlag(flag);
}

AutoClassRegister::AutoClassRegister(const ClassType& classType)
{
    ClassDB::Get().RegisterClassType(classType);
}
