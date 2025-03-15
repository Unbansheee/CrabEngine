//#include <Reflection.cppm>
module reflection;
void Property::TriggerPropertySetOn(IPropertyInterface* obj) const
{
    obj->OnPropertySet(*const_cast<Property*>(this));
}
