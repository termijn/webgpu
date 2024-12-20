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
     @location(3)       uv:                 vec2f
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
    model:          mat4x4f,
    modelInverseTranspose:   mat4x4f
}

@group(0)
@binding(0)
var<uniform> frame: Frame;

@group(1)
@binding(0)
var<uniform> model: Model;

@group(1) @binding(1)
var baseColorTexture: texture_2d<f32>;

@group(1) @binding(2) 
var linearSampler: sampler;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    out.position            = frame.projection * frame.view * model.model * in.position;
    out.normal              = model.modelInverseTranspose * in.normal;
    out.viewPositionWorld   = frame.viewPositionWorld;
    out.fragPositionWorld   = model.model * in.position;
    out.uv                  = in.uv;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f 
{
    let lightIn : vec4f  = normalize(frame.lightPositionWorld - in.fragPositionWorld);

    let shading     = max(0.0, dot(lightIn.xyz, in.normal.xyz));
    let color       = textureSample(baseColorTexture, linearSampler, in.uv).rgb;
    return vec4f(color * shading + vec3f(0.2), 1.0);
}