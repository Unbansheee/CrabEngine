
struct CameraData
{
    viewMatrix: mat4x4f,
    projectionMatrix: mat4x4f,
    cameraPosition: mat4x4f
}

struct LightingData
{
    lightDirections: array<vec4f, 2>,
    lightColors: array<vec4f, 2>
}

struct ObjectData
{
    modelMatrix: mat4x4f,
    localMatrix: mat4x4f
}

struct GlobalData
{
    time: float
}