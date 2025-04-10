
import standard_vertex;

struct StandardMaterialProperties{
    float3 BaseColorFactor;
    float Hardness;
    float Kd;
    float Ks;
    float NormalStrength;
}





[[vk::push_constant]]
Model PushConstant;

[[vk::binding(0, 1)]]
ConstantBuffer<GlobalData> uGlobalData;

[[vk::binding(0, 2)]]
ConstantBuffer<CameraData> uCameraData;
[[vk::binding(1, 2)]]
ConstantBuffer<LightingData> uLightingData;
[[vk::binding(2, 2)]]
RWTexture2D<uint32_t> idPassTexture;

/*
[[vk::binding(0, 2)]]
ConstantBuffer<GlobalData> uGlobalData;
*/

[[vk::binding(0, 3)]]
ConstantBuffer<StandardMaterialProperties> uMaterialProperties;



[shader("vertex")]
StandardShaderFragmentInput vs_main(in StandardVertex v)
{
    StandardShaderFragmentInput output;
    float3 worldPos = mul(float4(v.Position, 1.0), PushConstant.ModelMatrix).xyz;
    float3 viewPosition = mul(float4(worldPos, 1), uCameraData.ViewMatrix).xyz;
    float4 projPosition = mul(float4(viewPosition, 1), uCameraData.ProjectionMatrix);

    output.WorldNormal = mul(float4(v.Normal, 0), PushConstant.ModelMatrix).xyz;
    output.VertexNormal = v.Normal;
    output.Tangent = mul(float4(v.Tangent, 0), PushConstant.ModelMatrix).xyz;
    output.Bitangent = mul(float4(v.Bitangent, 0), PushConstant.ModelMatrix).xyz;
    output.ViewDirection = uCameraData.CameraPosition - v.Position.xyz;
    output.ProjectedPosition = projPosition;
    output.WorldPosition = worldPos;
    output.Color = v.Color;
    output.UV = v.UV;
    return output;
};

[shader("fragment")]
float4 fs_main(in StandardShaderFragmentInput input)
{
    idPassTexture.Store({input.ProjectedPosition.x, input.ProjectedPosition.y}, PushConstant.DrawID);
    return float4(input.WorldPosition, 1);
};
