#include <memory>

module auto_registration;
import import_manager;
import resource_importer;


void AddImporterToImportManager(std::unique_ptr<ResourceImporter> imp)
{
    ImportManager::Get().RegisterImporter(std::move(imp));
}

