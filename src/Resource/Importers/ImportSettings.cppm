module;

#pragma once

//#include "Core/ClassDB.h"
//#include "Core/Object.h"
#include "ReflectionMacros.h"

export module import_settings;
import object;
import class_type;

export class ImportSettings : public Object
{
    CRAB_CLASS(ImportSettings, Object)
    BEGIN_PROPERTIES
    END_PROPERTIES
};
