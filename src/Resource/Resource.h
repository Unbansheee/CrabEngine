#pragma once
#include "Utility/ObservableDtor.h"


class Resource : public observable_dtor
{
public:
    Resource() = default;
    virtual ~Resource() override = default;
    
    Resource (const Resource&) = delete;
    Resource& operator= (const Resource&) = delete;
};
