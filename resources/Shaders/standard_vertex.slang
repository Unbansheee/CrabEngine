
module standard_vertex;

public struct StandardVertex{
    public float3 Position : POSITION;
    public float3 Normal : NORMAL;
    public float3 Color : COLOR;
    public float2 UV : UV;
    public float3 Tangent : TANGENT;
    public float3 Bitangent : BITANGENT;
};

public struct Model {
    public float4x4 ModelMatrix;
    public float4x4 LocalMatrix;
    public uint32_t DrawID;
};

public struct StandardShaderFragmentInput {
    public float4 ProjectedPosition : SV_Position;
    public float3 WorldPosition;
    public float3 Color;
    public float3 WorldNormal;
    public float3 VertexNormal;
    public float2 UV;
    public float3 ViewDirection;
    public float3 Tangent;
    public float3 Bitangent;
    public float4 Custom_01;
    public float4 Custom_02;
};

public struct CameraData{
    public float4x4 ViewMatrix;
    public float4x4 ProjectionMatrix;
    public float3 CameraPosition;
}

public struct LightingData{
    public Array<float4, 2> LightDirections;
    public Array<float4, 2> LightColors;
}

public struct GlobalData{
    public float Time;
}