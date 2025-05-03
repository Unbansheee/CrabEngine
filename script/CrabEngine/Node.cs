using System;
using System.Runtime.InteropServices;
using System.Text;

namespace CrabEngine;

using Generated;

[NativeEngineType]
public class Node : Object
{
    public Node(IntPtr nativeOwner) : base(nativeOwner) {}
    
    [NativeBind("Ready", typeof(void), typeof(IntPtr))]
    private static ReadyDelegate? NativeReady; 
    
    [NativeBind("Update", typeof(void), typeof(IntPtr), typeof(float))]
    private static UpdateDelegate? NativeUpdate;
    
    [NativeBind("EnterTree", typeof(void), typeof(IntPtr))]
    private static EnterTreeDelegate? NativeEnterTree;
    
    [NativeBind("ExitTree", typeof(void), typeof(IntPtr))]
    private static ExitTreeDelegate? NativeExitTree;
    
    [NativeBind("SetName", typeof(void), typeof(IntPtr), typeof(nint))]
    private static SetNameDelegate? NativeSetName;
    
    [NativeBind("NativeGetName", typeof(void), typeof(IntPtr), typeof(nint))]
    private static NativeGetNameDelegate? NativeGetName;
    
    protected virtual void EnterTree() {  
        NativeEnterTree?.Invoke(_nativeOwner);
    }

    protected virtual void ExitTree()
    {
        NativeExitTree?.Invoke(_nativeOwner);
    }

    protected virtual void Ready()
    {
        NativeReady?.Invoke(_nativeOwner);
    }

    protected virtual void Update(float dt)
    {
        NativeUpdate?.Invoke(_nativeOwner, dt);
    }

    public string Name
    {
        get
        {
            var ptr = Marshal.AllocHGlobal(512);
            NativeGetName.Invoke(_nativeOwner, ptr);
            string name = Marshal.PtrToStringUni(ptr)!;
            Marshal.FreeHGlobal(ptr);
            return name;
        }
        set
        {
            var ptr = Marshal.StringToHGlobalAnsi(value);
            NativeSetName.Invoke(_nativeOwner, ptr);
        }
    }
}