

[System.AttributeUsage(System.AttributeTargets.Class |
                       System.AttributeTargets.Struct, Inherited = false)
]
public class NativeEngineTypeAttribute : System.Attribute
{
}

[AttributeUsage(AttributeTargets.Class | AttributeTargets.Struct)]
public class InteropTypeAttribute : Attribute
{
    public Type InteropType { get; }

    public InteropTypeAttribute(Type interopType)
    {
        InteropType = interopType;
    }
}