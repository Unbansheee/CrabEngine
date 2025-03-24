#include <memory>

module Engine.Reflection.AutoRegistration;
import Engine.Resource.ImportManager;
import Engine.Resource.Importer;
import Engine.Reflection.Class;

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
