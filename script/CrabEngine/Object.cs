using System;

namespace CrabEngine;

[NativeEngineType]
public class Object
{
    protected IntPtr _nativeOwner = IntPtr.Zero;

    public IntPtr NativeOwner => _nativeOwner;

    public Object(IntPtr nativeOwner)
    {
        _nativeOwner = nativeOwner;
    }
}

