
import attributes;
import standard_shader;
import vertex;

layout(push_constant)
ConstantBuffer<Model> PushConstant : register(b0);
ParameterBlock<StandardRendererParameters> uRendererParams;

ConstantBuffer<float4> Colour;

struct CollisionDebugFragmentInput {
    float4 ProjectedPosition : SV_Position;
}

[shader("vertex")]
CollisionDebugFragmentInput vs_main(in StandardVertex v)
{
    CollisionDebugFragmentInput output;
    float3 worldPos = mul(float4(v.Position + (v.Normal * 0.01), 1.0), PushConstant.ModelMatrix).xyz;
    float3 viewPosition = mul(float4(worldPos, 1), uRendererParams.uCameraData.ViewMatrix).xyz;
    float4 projPosition = mul(float4(viewPosition, 1), uRendererParams.uCameraData.ProjectionMatrix);

    output.ProjectedPosition = projPosition;
    return output;
};

[shader("fragment")]
float4 fs_main(in CollisionDebugFragmentInput input)
{
    uRendererParams.idPassTexture.Store({input.ProjectedPosition.x, input.ProjectedPosition.y}, PushConstant.DrawID);

    var corrected_color = pow(Colour.xyz, float3(2.2));
    return float4(corrected_color.xyz, 0.3);
};
