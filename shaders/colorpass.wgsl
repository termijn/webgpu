struct VertexInput
{
    @location(0) position:  vec4f,
    @location(1) normal:    vec4f,
    @location(2) tangent:   vec4f,
    @location(3) bitangent: vec4f,
    @location(4) uv:        vec2f,
}

struct VertexOutput
{
     @builtin(position) position:           vec4f,
     @location(0)       normal:             vec4f,
     @location(1)       viewPositionWorld:  vec4f,
     @location(2)       fragPositionWorld:  vec4f,
     @location(3)       uv:                 vec2f,
     @location(4)       tangent:            vec4f,
     @location(5)       bitangent:          vec4f,


}

struct Frame
{
    view:               mat4x4f,
    projection:         mat4x4f,
    viewPositionWorld:  vec4f,
    lightPositionWorld: vec4f
}

struct Model
{
    model:                  mat4x4f,
    modelInverseTranspose:  mat4x4f
}

@group(0) @binding(0)
var<uniform> frame: Frame;

@group(1) @binding(0)
var<uniform> model: Model;

@group(1) @binding(1) 
var linearSampler: sampler;

@group(1) @binding(2)
var baseColorTexture: texture_2d<f32>;

@group(1) @binding(3)
var occlusionTexture: texture_2d<f32>;

@group(1) @binding(4)
var normalsTexture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    out.position            = frame.projection * frame.view * model.model * in.position;
    out.normal              = model.modelInverseTranspose * in.normal;
    out.viewPositionWorld   = frame.viewPositionWorld;
    out.fragPositionWorld   = model.model * in.position;
    out.uv                  = in.uv;
    out.tangent             = in.tangent;
    out.bitangent           = in.bitangent;
    return out;
}

fn occlusion(color: vec3f, uv: vec2f, strength: f32) -> vec3f
{
    let occlusionValue   = textureSample(occlusionTexture, linearSampler, uv).r;
    let occlusionStrength = 1.0;
    let occludedColor = mix(color, color * occlusionValue, occlusionStrength);
    return occludedColor;
}

fn normal(surfaceNormal: vec3f, uv: vec2f, tangent: vec3f, bitangent: vec3f) -> vec3f
{
    var result: vec3f = surfaceNormal;
    let tangentNormal: vec3f = textureSample(normalsTexture, linearSampler, uv).xyz * 2.0 - 1.0;
    let tbn = mat3x3f(normalize(tangent.xyz), normalize(bitangent.xyz), normalize(surfaceNormal.xyz));
    result = normalize(tbn * tangentNormal);
    return result;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f 
{
    let lightIn : vec4f  = normalize(frame.lightPositionWorld - in.fragPositionWorld);

    let normal          = normal(in.normal.xyz, in.uv, in.tangent.xyz, in.bitangent.xyz);
    let shading         = max(0.0, dot(lightIn.xyz, normal));
    let color           = textureSample(baseColorTexture, linearSampler, in.uv).rgb;
    let occludedColor   = occlusion(color, in.uv, 1.0);
    
    return vec4f(occludedColor * shading + vec3f(0.1), 1.0);
}