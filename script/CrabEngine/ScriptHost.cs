using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Diagnostics;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;

namespace CrabEngine;

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct ScriptPropertyInfo
{
    public nint Name;
    public nint Type;
    public nint DisplayName;
}

[StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
public struct ScriptInfo
{
    public nint Name; // char*
    public nint ParentClassName; // char*
    public int PropertyCount; // int
    public nint Properties; // ScriptPropertyInfo*
}

public static class ScriptHost
{
    private static readonly List<Type> _scriptTypes = new();
    private static ScriptInfoMarshaler _marshaler = new ScriptInfoMarshaler();
    private static List<ScriptModule> _loadedModules = new List<ScriptModule>();

    private static Dictionary<IntPtr, Object> _collisionChecker = new Dictionary<IntPtr, Object>();
    
    [UnmanagedCallersOnly(EntryPoint = "LoadScriptAssembly")]
    public static void LoadScriptAssembly(IntPtr pathPtr, IntPtr libName)
    {
        string path = Marshal.PtrToStringUni(pathPtr);
        string lib = Marshal.PtrToStringUni(libName);
        
        Console.WriteLine("[C# Engine] Loading Library: " + lib);

        var module = ScriptEngine.LoadScriptAssembly(path, lib);

        if (module == null)
        {
            Console.WriteLine($"[C# Engine] Failed to load library [{lib}] at [{path}]");
            return;
        }
        
        Console.WriteLine("[C# Engine] Library Loaded: " + lib);
        
        _scriptTypes.AddRange(module.GetScriptTypes());
        _loadedModules.Add(module);
        module.InvokeEntryPoint();
    }
    
    [UnmanagedCallersOnly(EntryPoint = "UnloadScriptAssembly")]
    public static void UnloadScriptAssembly(IntPtr pathPtr)
    {
        string? path = Marshal.PtrToStringUni(pathPtr);
        var module = _loadedModules.First(a => a.AssemblyPath == path);
        _scriptTypes.RemoveAll(a => module.GetScriptTypes().Contains(a));
        _loadedModules.Remove(module);
        module?.Unload();
    }

    [UnmanagedCallersOnly(EntryPoint = "GetScriptCount")]
    public static int GetScriptCount(IntPtr assemblyPath)
    {
        var pathStr = Marshal.PtrToStringUni(assemblyPath);
        var mod = _loadedModules.FirstOrDefault(a => a.AssemblyPath == pathStr);
        
        return mod.GetScriptTypes().Count;
    }
    
    [UnmanagedCallersOnly(EntryPoint = "PrintTransform")]
    public static void PrintTransform(Transform t)
    {
        Console.WriteLine(t);
    }
    
    [UnmanagedCallersOnly(EntryPoint = "GetScriptInfoList")]
    public static nint GetScriptInfoList(IntPtr assemblyPath)
    {
        var pathStr = Marshal.PtrToStringUni(assemblyPath);
        var mod = _loadedModules.FirstOrDefault(a => a.AssemblyPath == pathStr);
        
        var types = mod.GetScriptTypes();

        _marshaler = new ScriptInfoMarshaler();
        IntPtr scriptList = _marshaler.MarshalScriptInfoList(_scriptTypes);
        
        return scriptList;
    }
    
    [UnmanagedCallersOnly(EntryPoint = "CreateScriptInstance")]
    public static unsafe IntPtr CreateScriptInstance(IntPtr nativeOwner, IntPtr typeName) {
        string instanceType = Marshal.PtrToStringUni((IntPtr)typeName);
        var instance = Activator.CreateInstance(_scriptTypes.Find(type => type.FullName == instanceType) ??
                                                throw new InvalidOperationException(), nativeOwner) as Object;

        if (instance == null)
            throw new InvalidOperationException($"Could not create instance of {instanceType}");
        
        GCHandle handle = GCHandle.Alloc(instance);
        _collisionChecker[nativeOwner] = instance;
        
        Console.WriteLine($"Created {instanceType} instance for native ptr 0x{nativeOwner:X}, GCHandle: 0x{GCHandle.ToIntPtr(handle):X}");
        
        return (IntPtr)handle;
    }
    
    [UnmanagedCallersOnly(EntryPoint = "RegisterNativeFunction")]
    public static unsafe void RegisterNativeFunction(char* typeName, char* functionName, IntPtr fn) {
        string instanceTypeName = Marshal.PtrToStringUni((IntPtr)typeName);
        string fnName = Marshal.PtrToStringUni((IntPtr)functionName);

        Console.WriteLine($"Trying to register function {fnName} on Class {instanceTypeName}");
        var type = AppDomain.CurrentDomain.GetAssemblies()
            .SelectMany(a => a.GetTypes())
            .FirstOrDefault(t => t.FullName == instanceTypeName);
        
        if (fn == IntPtr.Zero)
            throw new InvalidOperationException($"No native function found for method '{fnName}'");
        
        foreach (var field in type.GetFields(BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic))
        {
            var attr = field.GetCustomAttribute<NativeBindAttribute>();
            if (attr == null)
                continue;

            if (attr.FunctionName != fnName)
            {
                continue;
            }
            
            var del = Marshal.GetDelegateForFunctionPointer(fn, field.FieldType);
            field.SetValue(null, del);

            Console.WriteLine($"Bound native method '{fnName}' to field '{field.Name}'");
        }
    }
    
    
    [UnmanagedCallersOnly(EntryPoint = "CallScriptMethod")]
    public static unsafe void CallScriptMethod(IntPtr handle, char* methodName, void** args, int argCount, void* returnBuffer)
    {
        string name = Marshal.PtrToStringUni((IntPtr)methodName)!;
        object? instance = GCHandle.FromIntPtr(handle).Target;

        if (instance is not Object obj)
        {
            throw new InvalidCastException("Handle does not point to a valid Object");
        }
        
        if (_collisionChecker.TryGetValue(obj.NativeOwner, out var expected))
        {
            if (!ReferenceEquals(expected, instance))
            {
                Console.WriteLine($"[ERROR] Handle mismatch! Native 0x{obj.NativeOwner:X} is bound to different instance.");
            }
        }
        else
        {
            Console.WriteLine($"[WARNING] Native 0x{obj.NativeOwner:X} not tracked in instance map.");
        }
        
        var method = instance!.GetType().GetMethod(name, BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance);
        if (method == null) throw new MissingMethodException(name);

        var parameters = method.GetParameters();
        var convertedArgs = new object?[argCount];
        for (int i = 0; i < argCount; i++) {
            var paramType = parameters[i].ParameterType;
            convertedArgs[i] = Marshal.PtrToStructure(new IntPtr(args[i]), paramType);
        }

        object? result = method.Invoke(instance, convertedArgs);
        if (returnBuffer != null && result != null) {
            Marshal.StructureToPtr(result, new IntPtr(returnBuffer), false);
        }
        
    }
    
    [UnmanagedCallersOnly(EntryPoint = "DestroyScript")]
    public static void DestroyScript(IntPtr handle) {
        var gch = GCHandle.FromIntPtr(handle);
        var instance = gch.Target;
        
        if (instance is Object obj)
        {
            IntPtr nativePtr = obj.NativeOwner; // assuming your Object base class has a NativeOwner property
            _collisionChecker.Remove(nativePtr);
            Console.WriteLine($"Destroyed script for native 0x{nativePtr:X}");
        }
        gch.Free();

    }
}

public static class ReflectionBridge
{

}

public sealed class ScriptInfoMarshaler : IDisposable
{
    private readonly List<IntPtr> _allocated = new();
    private IntPtr _scriptInfoArray;

    public IntPtr MarshalScriptInfoList(IEnumerable<Type> types)
    {
        var list = new List<ScriptInfo>();
        foreach (var t in types)
        {
            bool parentIsNative =
                (t.BaseType?.GetCustomAttributes(typeof(NativeEngineTypeAttribute), false).Length != 0);
            
            var namePtr = AllocString(t.FullName!);
            var parentName = parentIsNative ? t.BaseType.Name!: t.BaseType.FullName!;
            var parentNamePtr = AllocString(parentName);

            var props = GetClassProperties(t);
            var propArrayPtr = AllocStructArray(props);
            
            
            list.Add(new ScriptInfo
            {
                Name = namePtr,
                ParentClassName = parentNamePtr,
                PropertyCount = props.Length,
                Properties = propArrayPtr
            });
        }
        
        _scriptInfoArray = AllocStructArray(list.ToArray());
        return _scriptInfoArray;
    }

    public ScriptPropertyInfo[] GetClassProperties(Type type)
    {
        var props = type.GetProperties(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        var result = new List<ScriptPropertyInfo>();

        foreach (var prop in props)
        {
            var attr = prop.GetCustomAttribute<SerializeFieldAttribute>();
            if (attr != null)
            {
                var info = new ScriptPropertyInfo();
                info.Name = AllocString(prop.Name);
                info.Type = AllocString(prop.PropertyType.FullName ?? "Unknown");
                info.DisplayName = AllocString(attr.DisplayName);

                result.Add(info);
                
                Console.WriteLine(prop.Name + " " + attr.DisplayName);

            }
            

        }
        
        var fields = type.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic);
        foreach (var field in fields)
        {
            var attr = field.GetCustomAttribute<SerializeFieldAttribute>();
            if (attr != null)
            {
                var info = new ScriptPropertyInfo();
                info.Name = AllocString(field.Name);
                info.Type = AllocString(field.FieldType.FullName ?? "Unknown");
                info.DisplayName = AllocString(attr.DisplayName);

                result.Add(info);
            }
        }

        return result.ToArray();
    }
    
    private IntPtr AllocString(string s)
    {
        var ptr = Marshal.StringToHGlobalAnsi(s);
        _allocated.Add(ptr);
        return ptr;
    }

    private IntPtr AllocStructArray<T>(T[] items)
    {
        int size = Marshal.SizeOf<T>();
        IntPtr arrayPtr = Marshal.AllocHGlobal(size * items.Length);
        _allocated.Add(arrayPtr); // Track the whole array block

        for (int i = 0; i < items.Length; ++i)
        {
            IntPtr itemPtr = arrayPtr + i * size;
            Marshal.StructureToPtr(items[i], itemPtr, false);
        }

        return arrayPtr;
    }

    public void Dispose()
    {
        Console.WriteLine("Disposing");
        foreach (var ptr in _allocated)
            Marshal.FreeHGlobal(ptr);

        _allocated.Clear();
    }
    
}
