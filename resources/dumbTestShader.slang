
struct VertexOutput{
    float4 Position : SV_Position;
    float4 Hehe;
};

float4 TestUniform;
ParameterBlock<float4> TestParams;
Texture2D FunnyTexture;
SamplerState FunnySampler;

[shader("vertex")]
VertexOutput vertexMain()
{
    VertexOutput output;
    output.Position = TestUniform;

    return output;
};

[shader("fragment")]
float4 fragmentMain()
{
    //return FunnyTexture.Load({0,0,0}) + TestUniform;
    return {0,0,0,0};
};
