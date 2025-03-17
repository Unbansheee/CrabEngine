module resource_ref;
import reflection;
import class_db;
import class_type;
import resource;

//#include <cassert>
//#include <memory>
//#include <stdexcept>



ResourceRef::ResourceRef() : typeHash(typeid(Resource))
{
}


bool StrongResourceRef::IsCompatible(const std::shared_ptr<Resource>& with) const
{
    return (with->GetStaticClassFromThis().IsSubclassOf(*m_classType));
}

bool StrongResourceRef::IsType(const ClassType& type) const {
    if (m_resource)
    {
        if (m_resource->GetStaticClassFromThis().IsSubclassOf(type)) return true;
    }
    return m_classType && m_classType->IsSubclassOf(type);
}

void StrongResourceRef::SetTypeFilter(const ClassType& filterType) {
    if (m_classType && !m_classType->IsSubclassOf(filterType)) {
        throw std::runtime_error("Existing resource violates new type filter");
    }
    m_classType = &filterType;
}
/*
bool ResourceHandle::IsValid() const
{
    return ResourceDB::Get().IsResourceIDValid(ResourceID);
}
*/
