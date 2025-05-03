using System.Runtime.InteropServices;
using System.Text;
using Generated;

namespace CrabEngine;

[NativeEngineType]
public class Resource : Object
{
    private static readonly Dictionary<string, WeakReference<Resource>> _loadedResources = new(StringComparer.OrdinalIgnoreCase);
    
    [NativeBind("ReleaseResource", typeof(void), typeof(IntPtr))]
    private static ReleaseResourceDelegate? NativeReleaseResource;
    
    [NativeBind("LoadResource", typeof(IntPtr), typeof(nint))]
    private static LoadResourceDelegate? NativeLoadResource;

    [NativeBind("LoadData", typeof(void), typeof(IntPtr))]
    private static LoadDataDelegate? NativeLoadData;

    public static T? Load<T>(string path) where T : Resource
    {
        if (_loadedResources.TryGetValue(path, out var cached))
        {
            if (cached.TryGetTarget(out var cachedRes))
            {
                return cachedRes as T;
            }
        }
        
        var nameBytes = Encoding.ASCII.GetBytes(path!);
        var ptr = Marshal.AllocHGlobal(nameBytes.Length);
        Marshal.Copy(nameBytes, 0, ptr, nameBytes.Length);
        
        IntPtr r = NativeLoadResource.Invoke(ptr);
        if (r == IntPtr.Zero)
        {
            Console.WriteLine("Could not load resource: " + path);
            return null;
        }

        var res = Activator.CreateInstance(typeof(T), r) as T;
        Console.WriteLine("Created Resource from path: " + path);
        _loadedResources[path] = new WeakReference<Resource>(res);
        return res;
    }
    
    public Resource(IntPtr nativeOwner) : base(nativeOwner)
    {
    }
    
    public void LoadData()
    {
        NativeLoadData?.Invoke(_nativeOwner);
        Console.WriteLine(_nativeOwner);
    }
    
    ~Resource()
    {
        foreach (var kvp in _loadedResources)
        {
            if (kvp.Value.TryGetTarget(out var loadedRes) && loadedRes == this)
            {
                _loadedResources.Remove(kvp.Key);
                break;
            }
        }
        
        Console.WriteLine("Released resource from C#");
        NativeReleaseResource?.Invoke(_nativeOwner);
    }
}

[NativeEngineType]
public class TextureResource : Resource
{
    public TextureResource(IntPtr nativeOwner) : base(nativeOwner)
    {
    }

    [NativeBind("GetSize", typeof(void), typeof(IntPtr), typeof(IntPtr))] /*Vector2U*/ 
    private static GetSizeDelegate? NativeGetSize;


    public Vector2U Size
    {
        get
        {
            unsafe
            {
                Vector2U result = new Vector2U();
                Vector2U* ptr = &result;
                NativeGetSize(_nativeOwner, (IntPtr)ptr);
                return result;
            }
        }
    }
};

[NativeEngineType]
public class ImageTextureResource : TextureResource
{
    public ImageTextureResource(IntPtr nativeOwner) : base(nativeOwner)
    {
    }
    
};