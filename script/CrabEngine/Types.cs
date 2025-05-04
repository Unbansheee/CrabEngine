using System.Numerics;
using System.Runtime.InteropServices;

namespace CrabEngine;

[StructLayout(LayoutKind.Sequential)]
public struct Vector2Interop
{
    public float X, Y;
    
    public override string ToString()
    {
        return "{" + $"X: {X}, Y: {Y}" + "}";
    }
    
    
    public static implicit operator Vector2(Vector2Interop v) => new(v.X, v.Y);
    public static implicit operator Vector2Interop(Vector2 v) => new() { X = v.X, Y = v.Y };
}


[StructLayout(LayoutKind.Sequential)]
public struct Vector2U
{
    public uint X;
    public uint Y;
    
    public override string ToString()
    {
        return "{" + $"X: {X}, Y: {Y}" + "}";
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Vector2I
{
    public int X { get; set; } 
    public int Y { get; set; } 
    
    public override string ToString()
    {
        return "{" + $"X: {X}, Y: {Y}" + "}";
    }
}

[StructLayout(LayoutKind.Sequential)]
public struct Vector3Interop
{
    public Vector3Interop(float x, float y, float z)
    {
        X = x;
        Y = y;
        Z = z;
    }
    
    public float X, Y, Z;

    public override string ToString()
    {
        return "{" + $"X: {X}, Y: {Y}, Z: {Z}" + "}";
    }
    
    
    public static implicit operator Vector3(Vector3Interop v) => new(v.X, v.Y, v.Z);
    public static implicit operator Vector3Interop(Vector3 v) => new() { X = v.X, Y = v.Y, Z = v.Z };
}

[StructLayout(LayoutKind.Sequential)]
public struct Vector4Interop
{
    public float X, Y, Z, W;
    
    public override string ToString()
    {
        return "{" + $"{X}, {Y}, {Z}, {W}" + "}";
    }
    
    // Implicit conversion to System.Numerics.Quaternion
    public static implicit operator Vector4(Vector4Interop q) =>
        new Vector4(q.W, q.X, q.Y, q.Z);

    // Implicit conversion from System.Numerics.Quaternion
    public static implicit operator Vector4Interop(Vector4 q) =>
        new Vector4Interop{ W = q.W, X = q.X, Y = q.Y, Z = q.Z };
}

[StructLayout(LayoutKind.Sequential)]
public struct QuaternionInterop
{
    public float W = 1;
    public float X = 0;
    public float Y = 0;
    public float Z = 0;

    public QuaternionInterop()
    {
    }

    public override string ToString()
    {
        return "{" + $"{W}, {X}, {Y}, {Z}" + "}";
    }
    
    
    // Implicit conversion to System.Numerics.Quaternion
    public static implicit operator Quaternion(QuaternionInterop q) =>
        new Quaternion(q.X, q.Y, q.Z, q.W);

    // Implicit conversion from System.Numerics.Quaternion
    public static implicit operator QuaternionInterop(Quaternion q) =>
        new QuaternionInterop { W = q.W, X = q.X, Y = q.Y, Z = q.Z };
}

[StructLayout(LayoutKind.Sequential)]
public struct Transform
{
    public Vector3Interop Position { get; set; } 
    public QuaternionInterop Orientation { get; set; } 
    public Vector3Interop Scale { get; set; }


    public override string ToString()
    {
        return "{\n" + $"    Position: {Position}\n    Orientation: {Orientation}\n    Scale: {Scale}" + "\n}";
    }
}


[AttributeUsage(AttributeTargets.Property | AttributeTargets.Field)]
public class SerializeFieldAttribute : Attribute
{
    public string DisplayName { get; set; }

    public SerializeFieldAttribute(string displayName = "")
    {
        DisplayName = displayName;
    }
}

public enum InputEventType
{
    Key,
    MouseMove,
    MouseButton,
    Scroll
}

[InteropType(typeof(InputEventInterop))]
public struct InputEvent
{
    public InputEventType Type;

    // Key
    public Input.Key.Code Key;
    public int KeyScancode;
    public Input.Key.Action KeyAction;
    public int KeyMods;

    // Mouse move
    public Vector2 MousePos;

    // Mouse button
    public Input.Mouse.Button MouseButton;
    public Input.Mouse.Action MouseButtonAction;
    public int MouseButtonMods;

    // Scroll
    public Vector2 ScrollOffset;
    
    // Implicit conversion to managed InputEvent
    public static implicit operator InputEvent(InputEventInterop e) => new InputEvent
    {
        Type = e.Type,
        Key = (Input.Key.Code)e.Key,
        KeyScancode = e.KeyScancode,
        KeyAction = (Input.Key.Action)e.KeyAction,
        KeyMods = e.KeyMods,
        MousePos = new Vector2((float)e.MouseX, (float)e.MouseY),
        MouseButton = (Input.Mouse.Button)e.MouseButton,
        MouseButtonAction = (Input.Mouse.Action)e.MouseButtonAction,
        MouseButtonMods = e.MouseButtonMods,
        ScrollOffset = new Vector2((float)e.ScrollX, (float)e.ScrollY)
    };
    
    // Implicit conversion back to interop
    public static implicit operator InputEventInterop(InputEvent e) => new InputEventInterop
    {
        Type = e.Type,
        Key = (int)e.Key,
        KeyScancode = e.KeyScancode,
        KeyAction = (int)e.KeyAction,
        KeyMods = e.KeyMods,
        MouseX = e.MousePos.X,
        MouseY = e.MousePos.Y,
        MouseButton = (int)e.MouseButton,
        MouseButtonAction = (int)e.MouseButtonAction,
        MouseButtonMods = e.MouseButtonMods,
        ScrollX = e.ScrollOffset.X,
        ScrollY = e.ScrollOffset.Y
    };
}

[StructLayout(LayoutKind.Sequential, Pack = 1)]
public struct InputEventInterop
{
    public InputEventType Type;

    public int Key;
    public int KeyScancode;
    public int KeyAction;
    public int KeyMods;

    public double MouseX;
    public double MouseY;

    public int MouseButton;
    public int MouseButtonAction;
    public int MouseButtonMods;

    public double ScrollX;
    public double ScrollY;
}

public enum InputResult
{
    Ignored = 0,
    Handled = 1
}
