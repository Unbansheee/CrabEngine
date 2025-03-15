//#include "ClassDB.cppm"

module class_db;
import class_type;

const ClassType& ClassType::Get(std::type_index id)
{
    auto& db = ClassDB::Get();
    //assert(db.classTypes.contains(id));

    return db.classTypes.at(id);
}

bool ClassType::IsSubclassOf(const ClassType& parent) const
{
    auto& db = ClassDB::Get();
    
    // Are we registered as a class?
    if (!db.classTypes.contains(ClassIndex)) return false;
    auto& current = db.classTypes.at(ClassIndex);

    if (parent == Object::GetStaticClass()) return true;

    // If we are the same as the parent, return true
    if (current.ClassIndex == parent.ClassIndex) return true;
    
    while (current.ParentClass != parent.ClassIndex)
    {
        // If we get all the way up to Object we can safely say we are not related
        if (current.ParentClass == Object::GetStaticClass().ClassIndex) return false;

        // Move up the hierarchy to the parent
        current = db.classTypes.at(current.ParentClass);
    }
    
    return true;
}

std::vector<ClassType> ClassDB::GetClasses()
{
    std::vector<ClassType> types;
    for (auto i : classTypes)
    {
        types.push_back(i.second);
    }
    return types;
}
