using System.Numerics;
using Generated;

namespace CrabEngine;

[NativeEngineType]
public class Node3D : Node
{
    public Node3D(IntPtr nativeOwner) : base(nativeOwner)
    {
    }

    [NativeBind("SetPosition", typeof(void), typeof(IntPtr), typeof(Vector3Interop))]
    private static SetPositionDelegate? NativeSetPosition;
    
    [NativeBind("SetScale", typeof(void), typeof(IntPtr), typeof(Vector3Interop))]
    private static SetScaleDelegate? NativeSetScale;
    
    [NativeBind("SetOrientation", typeof(void), typeof(IntPtr), typeof(QuaternionInterop))]
    private static SetOrientationDelegate? NativeSetOrientation;
    
    [NativeBind("SetGlobalPosition", typeof(void), typeof(IntPtr), typeof(Vector3Interop))]
    private static SetGlobalPositionDelegate? NativeSetGlobalPosition;
    
    [NativeBind("SetGlobalScale", typeof(void), typeof(IntPtr), typeof(Vector3Interop))]
    private static SetGlobalScaleDelegate? NativeSetGlobalScale;
    
    [NativeBind("SetGlobalOrientation", typeof(void), typeof(IntPtr), typeof(QuaternionInterop))]
    private static SetGlobalOrientationDelegate? NativeSetGlobalOrientation;

    [NativeBind("GetPosition", typeof(Vector3Interop), typeof(IntPtr))]
    private static GetPositionDelegate? NativeGetPosition;
    
    [NativeBind("GetScale", typeof(Vector3Interop), typeof(IntPtr))]
    private static GetScaleDelegate? NativeGetScale;
    
    [NativeBind("GetOrientation", typeof(QuaternionInterop), typeof(IntPtr))]
    private static GetOrientationDelegate? NativeGetOrientation;
    
    [NativeBind("GetGlobalPosition", typeof(Vector3Interop), typeof(IntPtr))]
    private static GetGlobalPositionDelegate? NativeGetGlobalPosition;
    
    [NativeBind("GetGlobalScale", typeof(Vector3Interop), typeof(IntPtr))]
    private static GetGlobalScaleDelegate? NativeGetGlobalScale;
    
    [NativeBind("GetGlobalOrientation", typeof(QuaternionInterop), typeof(IntPtr))]
    private static GetGlobalOrientationDelegate? NativeGetGlobalOrientation;
    
    public Vector3 Position
    {
        get => NativeGetPosition.Invoke(_nativeOwner);
        set => NativeSetPosition.Invoke(_nativeOwner, value);
    }
    public Vector3 Scale
    {
        get => NativeGetScale.Invoke(_nativeOwner);
        set => NativeSetScale.Invoke(_nativeOwner, value);
    }
    public Quaternion Orientation
    {
        get => NativeGetOrientation.Invoke(_nativeOwner);
        set => NativeSetOrientation.Invoke(_nativeOwner, value);
    }
    public Vector3 GlobalPosition
    {
        get => NativeGetGlobalPosition.Invoke(_nativeOwner);
        set => NativeSetGlobalPosition.Invoke(_nativeOwner, value);
    }
    public Vector3 GlobalScale
    {
        get => NativeGetGlobalScale.Invoke(_nativeOwner);
        set => NativeSetGlobalScale.Invoke(_nativeOwner, value);
    }
    public Quaternion GlobalOrientation
    {
        get => NativeGetGlobalOrientation.Invoke(_nativeOwner);
        set => NativeSetGlobalOrientation.Invoke(_nativeOwner, value);
    }
}