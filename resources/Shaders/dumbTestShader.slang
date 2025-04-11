
import standard_vertex;




struct RendererParameters{
    ConstantBuffer<GlobalData> uGlobalData;
    ConstantBuffer<CameraData> uCameraData;
    ConstantBuffer<LightingData> uLightingData;
    RWTexture2D<uint32_t> idPassTexture;
}

layout(push_constant)
Model PushConstant : register(b0);

struct StandardMaterial{
    float3 BaseColorFactor = {1,1,1};
    float Hardness = 32.0;
    float Kd = 1.0;
    float Ks = 1.0;
    float NormalStrength = 1.0;
}



float3 BlinnPhong(in DirectionalLight light, in StandardMaterial material, float3 normal, float3 baseColor, float3 viewDirection)
{
    var lightColor = light.Color.rgb;
    var L = normalize(light.Direction.xyz);
    var R = reflect(-L, normal);

    var diffuse = max(0.1, dot(R, viewDirection)) * lightColor;

    var RoV = max(0.0, dot(R, viewDirection));
    var specular = pow(RoV, material.Hardness);
    return (baseColor * material.Kd * diffuse) + (material.Ks * specular);
}

ParameterBlock<RendererParameters> uRendererParams;
ParameterBlock<StandardMaterial> uMaterialProperties;

SamplerState StandardTextureSampler;
Texture2D AlbedoTexture;
Texture2D NormalTexture;

[shader("vertex")]
StandardShaderFragmentInput vs_main(in StandardVertex v)
{
    StandardShaderFragmentInput output;
    float3 worldPos = mul(float4(v.Position, 1.0), PushConstant.ModelMatrix).xyz;
    float3 viewPosition = mul(float4(worldPos, 1), uRendererParams.uCameraData.ViewMatrix).xyz;
    float4 projPosition = mul(float4(viewPosition, 1), uRendererParams.uCameraData.ProjectionMatrix);

    output.WorldNormal = mul(float4(v.Normal, 0), PushConstant.ModelMatrix).xyz;
    output.VertexNormal = v.Normal;
    output.Tangent = mul(float4(v.Tangent, 0), PushConstant.ModelMatrix).xyz;
    output.Bitangent = mul(float4(v.Bitangent, 0), PushConstant.ModelMatrix).xyz;
    output.ViewDirection = uRendererParams.uCameraData.CameraPosition - v.Position.xyz;
    output.ProjectedPosition = projPosition;
    output.WorldPosition = worldPos;
    output.Color = v.Color;
    output.UV = v.UV; //+ float2(uRendererParams.Test, uRendererParams.Test);
    return output;
};

[shader("fragment")]
float4 fs_main(in StandardShaderFragmentInput input)
{
    uRendererParams.idPassTexture.Store({input.ProjectedPosition.x, input.ProjectedPosition.y}, PushConstant.DrawID);

    float normalMapStrength = uMaterialProperties.NormalStrength;
    float3 encodedN = NormalTexture.Sample(StandardTextureSampler, input.UV).rgb;
    float3 localN = encodedN * 2.0 - 1.0;

    var localToWorld = float3x3(
            normalize(input.Tangent),
            normalize(input.Bitangent),
            normalize(input.WorldNormal)
        );

    var worldN = mul(localN, localToWorld);
    var N = normalize(lerp(input.WorldNormal, worldN, normalMapStrength));
    var V = normalize(input.ViewDirection);

    float3 baseColor = AlbedoTexture.Sample(StandardTextureSampler, input.UV).rgb;

    float3 color = {0,0,0};
    for(int i = 0; i < uRendererParams.uLightingData.DirectionalLightCount; i++){
        color += BlinnPhong(uRendererParams.uLightingData.DirectionalLights[i], uMaterialProperties, N, baseColor, V);
    }

    var corrected_color = pow(color, float3(2.2));
    return float4(corrected_color.xyz, 1);
};
