module;

#pragma once

//#include "Core/ClassDB.h"
//#include "Core/Object.h"
#include "ReflectionMacros.h"

export module Engine.Resource.Importer.ImportSettings;
import Engine.Object;
import Engine.Reflection.Class;

export class ImportSettings : public Object
{
    CRAB_CLASS(ImportSettings, Object)
    BEGIN_PROPERTIES
    END_PROPERTIES
};
