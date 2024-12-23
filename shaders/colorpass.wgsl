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
    view:                 mat4x4f,
    projection:           mat4x4f,
    shadowViewProjection: mat4x4f,
    viewPositionWorld:    vec4f,
    lightPositionWorld:   vec4f
}

struct Model
{
    model:                  mat4x4f,
    modelInverseTranspose:  mat4x4f
}

@group(0) @binding(0)
var<uniform> frame: Frame;

@group(0) @binding(1)
var shadowTexture: texture_depth_2d;

@group(0) @binding(2) 
var shadowSampler: sampler_comparison;

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

@group(1) @binding(5)
var emissiveTexture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    let pos: vec4f = frame.projection * frame.view * model.model * in.position;

    out.position            = pos;
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
    var result:         vec3f = surfaceNormal;
    let tangentNormal:  vec3f = textureSample(normalsTexture, linearSampler, uv).xyz * 2.0 - 1.0;
    
    let tbn = mat3x3f(normalize(tangent.xyz), normalize(bitangent.xyz), normalize(surfaceNormal.xyz));
    result = normalize(tbn * tangentNormal);
    return result;
}

fn toLightSpace(coord: vec4f) -> vec4f
{
    var result: vec4f = frame.shadowViewProjection * coord;
    result /= result.w;

    return vec4f(
        result.xy * vec2f(0.5, -0.5) + vec2f(0.5),
        result.z,
        1.0
    );
}

fn shadow(coord: vec4f) -> f32
{
    let shadowCoord: vec4f = toLightSpace(coord);
    let uv = shadowCoord.xy;

    var shadow = 0.0;
    let shadowTexelSize = 1.0 / vec2f(2048, 2048);
    for (var y = -3; y <= 3; y++) {
        for (var x = -3; x <= 3 ; x++) {
        let offset = vec2f(vec2(x, y)) * shadowTexelSize;

        shadow += textureSampleCompare(
            shadowTexture, shadowSampler,
            uv + offset, shadowCoord.z - 0.007
        );
        }
    }
    shadow /= 49.0;

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        shadow = 1.0;
    }
    return shadow;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f 
{
    let lightIn : vec4f  = normalize(frame.lightPositionWorld - in.fragPositionWorld);

    let normal          = normal(in.normal.xyz, in.uv, in.tangent.xyz, in.bitangent.xyz);
    let shading         = max(0.0, dot(lightIn.xyz, normal));
    let color           = textureSample(baseColorTexture, linearSampler, in.uv).rgb;
    let occludedColor   = occlusion(color, in.uv, 1.0);
    let emissive        = textureSample(emissiveTexture, linearSampler, in.uv).rgb;
    let shadow          = shadow(in.fragPositionWorld);

    return vec4f((occludedColor * shading + emissive) * (0.5 + (shadow  * 0.5)), 1.0);
}