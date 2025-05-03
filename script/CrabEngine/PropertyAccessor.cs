using System.Reflection;
using System.Runtime.InteropServices;

namespace CrabEngine;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void SetPropertyDelegate(IntPtr instanceHandle, IntPtr propName, IntPtr value);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
public delegate void GetPropertyDelegate(IntPtr instanceHandle, IntPtr propName, IntPtr outVal);


[StructLayout(LayoutKind.Sequential)]
public struct ScriptInteropFunctions
{
    public IntPtr SetPropertyFunc;
    public IntPtr GetPropertyFunc;
}

public static class InteropBootstrap
{
    public static void SetProperty(IntPtr instanceHandle, IntPtr propName, IntPtr value)
    {
        var handle = GCHandle.FromIntPtr(instanceHandle);
        var scriptObject = (object)handle.Target!;
        string property = Marshal.PtrToStringAnsi(propName)!;

        {
            var prop = scriptObject.GetType().GetProperty(property, BindingFlags.Public | BindingFlags.Instance | BindingFlags.NonPublic);
            if (prop != null)
            {
                if (prop.PropertyType == typeof(string))
                {
                    var valueStr = Marshal.PtrToStringAnsi(value);
                    prop.SetValue(scriptObject, valueStr);
                    return;
                }
                if (prop.PropertyType.IsValueType)
                {
                    object val = Marshal.PtrToStructure(value, prop.PropertyType)!;
                    scriptObject.GetType().GetProperty(property)?.SetValue(scriptObject, val);
                    return;
                }
            
                Console.WriteLine($"({prop.Name}) {prop.PropertyType.Name} Not a valid C# Property type");
            }
        }


        {
            var field = scriptObject.GetType().GetField(property, BindingFlags.Public | BindingFlags.Instance | BindingFlags.NonPublic);
            if (field != null)
            {
                if (field.FieldType == typeof(string))
                {
                    var valueStr = Marshal.PtrToStringAnsi(value);
                    field.SetValue(scriptObject, valueStr);
                    return;
                }
                if (field.FieldType.IsValueType)
                {
                    object val = Marshal.PtrToStructure(value, field.FieldType)!;
                    field.SetValue(scriptObject, val);
                    return;
                }
            
                Console.WriteLine($"({field.Name}) {field.FieldType.Name} Not a valid C# Property type");
            }
        }
        
        Console.WriteLine($"Property ({property}) not found");

    }

    public static void GetProperty(IntPtr instanceHandle, IntPtr propName, IntPtr outVal)
    {
        var handle = GCHandle.FromIntPtr(instanceHandle);
        var scriptObject = (object)handle.Target!;
        
        string property = Marshal.PtrToStringAnsi(propName)!;

        {
            var prop = scriptObject.GetType().GetProperty(property, BindingFlags.Public | BindingFlags.Instance | BindingFlags.NonPublic);
            if (prop != null)
            {
                if (prop.PropertyType == typeof(string))
                {
                    var value = (string?)prop.GetValue(scriptObject) ?? "";
                    var bytes = System.Text.Encoding.ASCII.GetBytes(value); // ANSI string
                    Marshal.Copy(bytes, 0, outVal, bytes.Length);
                    return;
                }
                if (prop.PropertyType.IsValueType)
                {
                    object? value = prop.GetValue(scriptObject);
                    if (value != null)
                    {
                        Marshal.StructureToPtr(value, outVal, false);
                        return;
                    }
                }
                
                Console.WriteLine($"({prop.Name}) {prop.PropertyType.Name} Not a valid C# Property type");
            }
        }


        {
            var field = scriptObject.GetType().GetField(property, BindingFlags.Public | BindingFlags.Instance | BindingFlags.NonPublic);
            if (field != null)
            {
                if (field.FieldType == typeof(string))
                {
                    var value = (string?)field.GetValue(scriptObject) ?? "";
                    var bytes = System.Text.Encoding.ASCII.GetBytes(value ); // ANSI string
                    Marshal.Copy(bytes, 0, outVal, bytes.Length);
                    return;
                }
                if (field.FieldType.IsValueType)
                {
                    object? value = field.GetValue(scriptObject);
                    if (value != null)
                    {
                        Marshal.StructureToPtr(value, outVal, false);
                        return;
                    }
                }
                Console.WriteLine($"({field.Name}) {field.FieldType.Name} Not a valid C# Property type");
            }
        }

        Console.WriteLine($"Property ({property}) not found");
    }
    
    
    [UnmanagedCallersOnly]
    public static ScriptInteropFunctions GetInterop()
    {
        return new ScriptInteropFunctions
        {
            SetPropertyFunc = Marshal.GetFunctionPointerForDelegate((SetPropertyDelegate)SetProperty),
            GetPropertyFunc = Marshal.GetFunctionPointerForDelegate((GetPropertyDelegate)GetProperty),
        };
    }
}