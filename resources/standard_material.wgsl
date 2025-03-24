
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
    @location(0) color: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
    @location(3) viewDirection: vec3f,
    @location(4) tangent: vec3f,
    @location(5) bitangent: vec3f
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
    localMatrix: mat4x4f
}

struct GlobalData
{
    time: f32
}

struct StandardMaterialParameters
{
    baseColorFactor: vec3f,
    hardness: f32,
    kd: f32,
    ks: f32,
    normalStrength: f32 
}

// Global
@group(0) @binding(0) var<uniform> uGlobalUniforms: GlobalData;

// Renderer
@group(1) @binding(0) var<uniform> uCameraUniforms: CameraData;
@group(1) @binding(1) var<uniform> uLightingData: LightingData;

// Per Object
@group(2) @binding(0) var<uniform> uPerObjectUniforms: ObjectData;

// Material Specific
@group(3) @binding(0) var<uniform> uMaterialUniforms: StandardMaterialParameters;
@group(3) @binding(1) var textureSampler: sampler;
@group(3) @binding(2) var baseColorTexture: texture_2d<f32>;
@group(3) @binding(3) var normalTexture: texture_2d<f32>;

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

	let cameraWorldPosition = uCameraUniforms.cameraPosition;
    out.viewDirection = cameraWorldPosition - worldPosition.xyz;

    out.tangent = (uPerObjectUniforms.modelMatrix * vec4f(in.tangent, 0.0)).xyz;
    out.bitangent = (uPerObjectUniforms.modelMatrix * vec4f(in.bitangent, 0.0)).xyz;
    out.normal = (uPerObjectUniforms.modelMatrix * vec4f(in.normal, 0.0)).xyz;
    
	out.color = in.color;
    out.uv = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    // Gamma-correction
    let normalMapStrength = uMaterialUniforms.normalStrength; // could be a uniform
        let encodedN = textureSample(normalTexture, textureSampler, in.uv).rgb;
        let localN = encodedN * 2.0 - 1.0;
        // The TBN matrix converts directions from the local space to the world space
        let localToWorld = mat3x3f(
            normalize(in.tangent),
            normalize(in.bitangent),
            normalize(in.normal),
        );
        let worldN = localToWorld * localN;
        let N = normalize(mix(in.normal, worldN, normalMapStrength));
    
        let V = normalize(in.viewDirection);
    
        // Sample texture
        let baseColor = textureSample(baseColorTexture, textureSampler, in.uv).rgb * uMaterialUniforms.baseColorFactor;
        //let baseColor = vec3f(1,1,1);
        let kd = uMaterialUniforms.kd;
        let ks = uMaterialUniforms.ks;
        let hardness = uMaterialUniforms.hardness;
    
        // Compute shading
        var color = vec3f(0.0);
        for (var i: i32 = 0; i < 2; i++) {
            let lightColor = uLightingData.lightColors[i].rgb;
            let L = normalize(uLightingData.lightDirections[i].xyz);
            let R = reflect(-L, N); // equivalent to 2.0 * dot(N, L) * N - L
    
            let diffuse = max(0.1, dot(L, N)) * lightColor;
    
            // We clamp the dot product to 0 when it is negative
            let RoV = max(0.0, dot(R, V));
            let specular = pow(RoV, hardness);
    
            color += baseColor * kd * diffuse + ks * specular;
        }
    
        //color = N * 0.5 + 0.5;
    
        
    
        // Gamma-correction
        let corrected_color = pow(color, vec3f(2.2));
        return vec4f(corrected_color.xyz, 1.0);
}