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
     @location(1)       fragPositionWorld:  vec4f,
     @location(2)       uv:                 vec2f,
     @location(3)       tangent:            vec4f,
     @location(4)       bitangent:          vec4f,
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

const Fdielectric = vec3f(0.04f);
const PI = 3.1415926535897932384f;

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

@group(1) @binding(6)
var metallicRoughnessTexture: texture_2d<f32>;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    let pos: vec4f = frame.projection * frame.view * model.model * in.position;

    out.position            = pos;
    out.normal              = model.modelInverseTranspose * in.normal;
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

// GGX normal distribution function.
fn ndfGGX(cosLh: f32, alpha: f32) -> f32 
{
    let alpha2: f32 = alpha * alpha;
    let denom: f32 = (cosLh * cosLh * (alpha2 - 1.0) + 1.0);
    return alpha2 / (PI * denom * denom);
}

fn gaSchlickG1(cosTheta: f32, k: f32) -> f32
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
fn gaSchlickGGX(cosLi: f32, cosLo: f32, roughness: f32) -> f32
{
    let r = roughness + 1.0;
    let k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor.
fn fresnelSchlick(F0: vec3f, cosTheta: f32) -> vec3f
{
    return F0 + (vec3(1.0f) - F0) * pow(1.0f - cosTheta, 5.0f);
}

fn aces_approx(vo: vec3f) -> vec3f
{
    let v = vo * vec3f(0.6);
    let a = 2.51;
    let b = 0.03;
    let c = 2.43;
    let d = 0.59;
    let e = 0.14;
    return clamp((v*(a*v+b)) / (v*(c*v+d)+e), vec3f(0.0), vec3f(1.0));
}

fn reinhard_extended(v: vec3f, max_white: f32) -> vec3f
{
    let numerator: vec3f = v * (1.0f + (v / vec3(max_white * max_white)));
    return numerator / (1.0f + v);
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f 
{
    let albedo: vec3f = textureSample(baseColorTexture, linearSampler, in.uv).rgb;

    var finalColor: vec3f = albedo;

    let Lo: vec3f         = normalize(frame.viewPositionWorld - in.fragPositionWorld).xyz;
    let Li: vec3f         = normalize(frame.lightPositionWorld - in.fragPositionWorld).xyz;
    let N: vec3f          = normal(in.normal.xyz, in.uv, in.tangent.xyz, in.bitangent.xyz);

    let metallicRoughness   = textureSample(metallicRoughnessTexture, linearSampler, in.uv).rgb;
    let metallic: f32       = 1.0;
    let roughness: f32      = 1.0;
    let lightColor: vec3f   = vec3f(1.0);
    let finalMetallic: f32  = metallicRoughness.b * metallic;
    let finalRoughness: f32 = metallicRoughness.g * roughness;

    let cosLo: f32 = max(0.0f, dot(N, Lo));

    let F0: vec3f = mix(Fdielectric, albedo, vec3(finalMetallic));

    var directLighting  = vec3f(0.0);
    {
        let Lh: vec3f = normalize(Li + Lo);

        let cosLi: f32 = max(0.0f, dot(N, Li));
        let cosLh: f32 = max(0.0f, dot(N, Lh));

        let  F: vec3f  = fresnelSchlick(F0, max(0.0f, dot(Lh, Li)));
        let D: f32   = ndfGGX(cosLh, finalRoughness);
        let G: f32  = gaSchlickGGX(cosLi, cosLo, finalRoughness);
        let  kd: vec3f = mix(vec3(1.0f) - F, vec3(0.0f), finalMetallic);

        let diffuseBRDF: vec3f    = kd * albedo;
        var specularBRDF: vec3f   = F * D * G / (4.0 * cosLi * cosLo + 0.001);
        specularBRDF        = mix(specularBRDF, vec3(0.0), finalRoughness);

        //let fresnel: f32         = 1.0 - max(dot(N, Lo), 0.0);
        // envReflection   = vec3(0.0);
        // if (reflectionMap.hasReflectionMap)
        // {
        //     vec3  reflected  = reflect(-Lo, N);
        //     float  mipLevel  = finalRoughness * float(reflectionMap.maxMipLevel);
        //     vec3 specColor   = textureLod(reflectionMap.texture, reflected, mipLevel).rgb;
        //     envReflection    = F * pow(specColor, vec3(2.2));

        //     directLighting += (mix(diffuseBRDF, envReflection, finalMetallic * F) + specularBRDF) * lightColor * cosLi;
        // } 
        // else
        // {
            directLighting += (diffuseBRDF +  specularBRDF) * lightColor * cosLi;
        //}

        // float rimFactor = pow(fresnel, rimWidth);
        // vec3   rimLight = rimColor * rimFactor * rimStrength;
        // directLighting += rimLight;
    }

    finalColor =  directLighting;

    finalColor   = occlusion(finalColor, in.uv, 1.0);
    let shadow   = shadow(in.fragPositionWorld);
    finalColor   = finalColor * (0.2 + (0.8 * shadow));
    finalColor  += textureSample(emissiveTexture, linearSampler, in.uv).rgb;

    let exposureStops = 1.8;
    let exposureFactor = pow(2.0, exposureStops);

    return vec4f(reinhard_extended(finalColor * exposureFactor, 1.5), 1.0);
}