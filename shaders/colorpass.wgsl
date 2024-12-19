struct VertexInput
{
    @location(0) position:  vec4f,
    @location(1) normal:    vec4f,
    @location(2) uv:        vec2f,
    @location(3) tangent:   vec4f,
    @location(4) bitangent: vec4f,
}

struct VertexOutput
{
     @builtin(position) position:           vec4f,
     @location(0)       normal:             vec4f,
     @location(1)       viewPositionWorld:  vec4f,
     @location(2)       fragPositionWorld:  vec4f
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

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    out.position            = frame.projection * frame.view * model.model * in.position;
    out.normal              = model.modelInverseTranspose * in.normal;
    out.viewPositionWorld   = frame.viewPositionWorld;
    out.fragPositionWorld   = model.model * in.position;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f 
{
    let lightIn : vec4f  = normalize(frame.lightPositionWorld - in.fragPositionWorld);

    let shading     = max(0.0, dot(lightIn.xyz, in.normal.xyz));
    let color       = vec3f(1.0, 0.6, 0.0);
    return vec4f(color * shading + vec3f(0.3), 1.0);
}