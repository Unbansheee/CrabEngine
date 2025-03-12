#include "ResourceHandle.h"

bool ResourceHandle::IsValid() const
{
    return ResourceDB::Get().IsResourceIDValid(ResourceID);
}
