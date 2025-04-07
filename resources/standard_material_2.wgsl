struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
    @location(3) uv: vec2f,
    @location(4) tangent: vec3f,
    @location(5) bitangent: vec3f,
}

struct VertexOutput {
    @builtin(position) clipPosition: vec4f,
    @location(0) worldPosition: vec3f,
    @location(1) color: vec3f,
    @location(2) worldNormal: vec3f,
    @location(3) vertexNormal: vec3f,
    @location(4) uv: vec2f,
    @location(5) viewDirection: vec3f,
    @location(6) tangent: vec3f,
    @location(7) bitangent: vec3f,
    @location(8) custom_01: vec4f,
    @location(9) custom_02: vec4f,
}

struct CameraData
{
    viewMatrix: mat4x4f,
    projectionMatrix: mat4x4f,
    cameraPosition: vec3f
}

struct LightingData
{
    lightDirections: array<vec4f, 2>,
    lightColors: array<vec4f, 2>
}

struct ObjectData
{
    modelMatrix: mat4x4f,
    localMatrix: mat4x4f,
    drawID: u32
}

struct GlobalData
{
    time: f32
}

// Global
@group(0) @binding(0) var<uniform> uGlobalUniforms: GlobalData;

// Renderer
@group(1) @binding(0) var<uniform> uCameraUniforms: CameraData;
@group(1) @binding(1) var<uniform> uLightingData: LightingData;
@group(1) @binding(2) var idPassTexture: texture_storage_2d<r32uint, write>;

// Per Object
@group(2) @binding(0) var<uniform> uPerObjectUniforms: ObjectData;

// Material Properties
@group(3) @binding(0) var<uniform> uMaterialProperties: MaterialProperties;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    let VERTEX_POSITION = in.position;
    let VERTEX_NORMAL = in.normal;
    var TIME = uGlobalUniforms.time;
    var COLOR = in.color;
    var UV = in.uv;
    var TANGENT: vec3f = in.tangent;
    var BITANGENT: vec3f = in.bitangent;
    var CUSTOM_01 = vec4f(0,0,0,0);
    var CUSTOM_02 = vec4f(0,0,0,0);

    let VIEW_MATRIX = uCameraUniforms.viewMatrix;
    let PROJECTION_MATRIX = uCameraUniforms.projectionMatrix;
    let CAMERA_POSITION = uCameraUniforms.cameraPosition;

    let MODEL_MATRIX = uPerObjectUniforms.modelMatrix;
    let VIEW_PROJECTION_MATRIX = PROJECTION_MATRIX * VIEW_MATRIX;

    var VIEW_DIRECTION = CAMERA_POSITION - VERTEX_POSITION.xyz;
    var WORLD_POSITION: vec3f = (MODEL_MATRIX * vec4f(VERTEX_POSITION.xyz, 1.0f)).xyz;
    
    TANGENT = (MODEL_MATRIX * vec4f(TANGENT, 0.0)).xyz;
    BITANGENT = (MODEL_MATRIX * vec4f(BITANGENT, 0.0)).xyz;
    var WORLD_NORMAL = (MODEL_MATRIX * vec4f(VERTEX_NORMAL, 0.0)).xyz;

    #VERTEX_FN
    
    var out: VertexOutput;
    out.clipPosition = VIEW_PROJECTION_MATRIX * vec4f(WORLD_POSITION, 1.0f);
    out.worldPosition = WORLD_POSITION;
    out.vertexNormal = VERTEX_NORMAL;
    out.worldNormal = WORLD_NORMAL;
    out.uv = UV;
    out.tangent = TANGENT;
    out.bitangent = BITANGENT;
    out.custom_01 = CUSTOM_01;
    out.custom_02 = CUSTOM_02;
    out.viewDirection = VIEW_DIRECTION;
    
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    var CLIP_POSITION = in.clipPosition;
    var WORLD_POSITION = in.worldPosition.xyz;
    var VERTEX_NORMAL = in.vertexNormal;
    var WORLD_NORMAL = in.worldNormal;
    var COLOR = in.color;
    var UV = in.uv;
    var TANGENT = in.tangent;
    var BITANGENT = in.bitangent;
    var CUSTOM_01 = in.custom_01;
    var CUSTOM_02 = in.custom_02;
    let DRAW_ID = uPerObjectUniforms.drawID;
    
    var ALBEDO = vec3f(0,0,0);
    var OPACITY = 1.0f;
    
    #FRAGMENT_FN
    
    for (var i: i32 = 0; i < 2; i++) {
        #LIGHT_FN
    }

    // StoreID value
    textureStore(idPassTexture, vec2u(u32(CLIP_POSITION.x), u32(CLIP_POSITION.y)), vec4u(uPerObjectUniforms.drawID,0,0,0));

    return vec4f(ALBEDO, OPACITY);
}