#include "ClassDB.h"
#include "Node.h"

std::vector<ClassType> ClassDB::GetClasses()
{
    std::vector<ClassType> types;
    for (auto i : classTypes)
    {
        types.push_back(i.second);
    }
    return types;
}
