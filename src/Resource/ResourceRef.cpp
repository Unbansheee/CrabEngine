module Engine.Resource.Ref;
import Engine.Reflection;
import Engine.Reflection.ClassDB;
import Engine.Reflection.Class;
import Engine.Resource;

//#include <cassert>
//#include <memory>
//#include <stdexcept>



ResourceRef::ResourceRef() : typeHash(typeid(Resource))
{
}


bool StrongResourceRef::IsResourceCompatible(const std::shared_ptr<Resource>& resource) const
{
    if (!resource) return false;
    if (!m_classType) return true;
    return (resource->GetStaticClassFromThis().IsSubclassOf(*m_classType));
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

StrongResourceRef::StrongResourceRef(std::shared_ptr<Resource> resource, const ClassType* filter): m_resource(resource)
{
    if (filter)
    {
        m_classType = filter;
    }
    else
    {
        m_classType = &resource->GetStaticClassFromThis();
    }
}

StrongResourceRef& StrongResourceRef::operator=(std::shared_ptr<Resource> resource)
{
    if (m_classType && !resource->GetStaticClassFromThis().IsSubclassOf(*m_classType)) {
        throw std::runtime_error("Resource type mismatch in assignment");
    }
    m_resource = resource;
    if (!m_classType) {
        m_classType = &resource->GetStaticClassFromThis(); // Set type on first assignment
    }
    return *this;
}
