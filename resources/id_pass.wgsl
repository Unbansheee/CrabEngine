struct VertexInput {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
    @location(3) uv: vec2f,
    @location(4) tangent: vec3f,
    @location(5) bitangent: vec3f,
}

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) drawID : u32
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

// Per Object
@group(2) @binding(0) var<uniform> uPerObjectUniforms: ObjectData;

// Material Specific


fn from_linear(linearC: vec4<f32>) -> vec4<f32> {
    let cutoff = step(linearC, vec4<f32>(0.0031308));
    let higher = vec4<f32>(1.055) * pow(linearC, vec4(1.0 / 2.4)) - vec4(0.055);
    let lower = linearC * vec4<f32>(12.92);
    return mix(higher, lower, cutoff);
}

fn to_linear(nonlinear: vec4<f32>) -> vec4<f32> {
    let cutoff = step(nonlinear, vec4<f32>(0.04045));
    let higher = pow((nonlinear + vec4<f32>(0.055)) / vec4<f32>(1.055), vec4<f32>(2.4));
    let lower = nonlinear / vec4<f32>(12.92);
    return mix(higher, lower, cutoff);
}

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;

    let viewProj = uCameraUniforms.projectionMatrix * uCameraUniforms.viewMatrix; 
    let worldPosition = uPerObjectUniforms.modelMatrix * vec4f(in.position, 1.0);
    
    out.position = viewProj * worldPosition;
	out.drawID = uPerObjectUniforms.drawID;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) u32 {
    return in.drawID;
}