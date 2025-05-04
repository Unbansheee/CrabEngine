
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
    float3 lightColor = light.Color.rgb;
    float3 L = normalize(light.Direction.xyz);
    float3 V = normalize(viewDirection);
    float3 H = reflect(-L, normal);;

    // Diffuse
    float diff = max(dot(normal, L), 0.0);
    float3 diffuse = baseColor * material.Kd * diff * lightColor;

    // Specular
    float spec = pow(max(dot(normal, H), 0.0), material.Hardness);
    float3 specular = material.Ks * spec * lightColor;

    return diffuse + specular;
}