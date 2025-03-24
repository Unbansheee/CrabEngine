//#include <Reflection.cppm>
module Engine.Reflection;
void Property::TriggerPropertySetOn(IPropertyInterface* obj) const
{
    obj->OnPropertySet(*const_cast<Property*>(this));
}
