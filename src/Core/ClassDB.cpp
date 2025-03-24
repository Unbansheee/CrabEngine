module Engine.Reflection.ClassDB;
import Engine.Reflection.Class;

bool ClassType::IsSubclassOf(const ClassType& parent) const
{
    auto& db = ClassDB::Get();
    
    auto current = this;
    if (parent == Object::GetStaticClass()) return true;

    // If we are the same as the parent, return true
    if (Name == parent.Name) return true;
    if (!parent.IsValid()) return false;
    while (current->Parent != parent.Name)
    {
        // If we get all the way up to Object we can safely say we are not related
        if (current->Parent == Object::GetStaticClass().Name) return false;

        // Move up the hierarchy to the parent
        if (auto c = db.GetParentClass(current->Parent))
        {
            current = c;
        }
        else
        {
            return false;
        }
    }
    
    return true;
}


const std::vector<const ClassType*>& ClassDB::GetClasses()
{
    return allClassTypes;
}
