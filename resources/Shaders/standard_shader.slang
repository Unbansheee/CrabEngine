
module standard_shader;



public struct StandardMaterial{
    public float3 BaseColorFactor = {1,1,1};
    public float Hardness = 32.0;
    public float Kd = 1.0;
    public float Ks = 1.0;
    public float NormalStrength = 1.0;
}

public struct CameraData{
    public float4x4 ViewMatrix;
    public float4x4 ProjectionMatrix;
    public float3 CameraPosition;
}

public struct DirectionalLight{
    public float4 Direction;
    public float4 Color;
};

public struct LightingData{
    public Array<DirectionalLight, 4> DirectionalLights;
    public int DirectionalLightCount = 0;
}

public struct GlobalData{
    public float Time;
}

public struct Model {
    public float4x4 ModelMatrix;
    public float4x4 LocalMatrix;
    public uint32_t DrawID;
};

public struct StandardRendererParameters{
    public ConstantBuffer<GlobalData> uGlobalData;
    public ConstantBuffer<CameraData> uCameraData;
    public ConstantBuffer<LightingData> uLightingData;
    public RWTexture2D<uint32_t> idPassTexture;
}

public float3 BlinnPhong(in DirectionalLight light, in StandardMaterial material, float3 normal, float3 baseColor, float3 viewDirection)
{
    var lightColor = light.Color.rgb;
    var L = normalize(light.Direction.xyz);
    var R = reflect(-L, normal);

    var diffuse = max(0.1, dot(R, viewDirection)) * lightColor;

    var RoV = max(0.0, dot(R, viewDirection));
    var specular = pow(RoV, material.Hardness);
    return (baseColor * material.Kd * diffuse) + (material.Ks * specular);
}