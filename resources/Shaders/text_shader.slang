

import attributes;
import standard_shader;
import vertex;

layout(push_constant)
ConstantBuffer<Model> PushConstant : register(b0);

ParameterBlock<StandardRendererParameters> uRendererParams;

[SerializedField] ConstantBuffer<float4> TextColour;

SamplerState StandardTextureSampler;
[SerializedField] Texture2D AlbedoTexture;

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

    float4 baseColor = AlbedoTexture.Sample(StandardTextureSampler, input.UV).rgba;
    if (baseColor.a <= 0)
    {
        discard;
    }
    uRendererParams.idPassTexture.Store( { input.ProjectedPosition.x, input.ProjectedPosition.y }, PushConstant.DrawID);

    float3 color = baseColor.xyz; // * TextColour.rgb;
    var corrected_color = pow(color, float3(2.2));

    return float4(corrected_color.xyz, 1);
};
