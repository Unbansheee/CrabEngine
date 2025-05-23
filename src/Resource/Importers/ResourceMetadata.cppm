﻿module;

#pragma once

#include "ReflectionMacros.h"

export module Engine.Resource.Importer.ImportSettings;
import Engine.Object;
import Engine.Reflection.Class;

export class ResourceMetadata : public Object
{
    CRAB_CLASS(ResourceMetadata, Object)
    BEGIN_PROPERTIES
        ADD_PROPERTY("ResourceID", ResourceID)
    END_PROPERTIES

    UID ResourceID = UID::Empty();
};
