struct VertexInput
{
    @location(0) position:  vec4f,
}

struct VertexOutput
{
    @builtin(position) position:           vec4f,
}

struct Frame
{
    view:       mat4x4f,
    projection: mat4x4f,
}

struct Model
{
    model: mat4x4f,
}

@group(0) @binding(0)
var<uniform> frame: Frame;

@group(1) @binding(0)
var<uniform> model: Model;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput
{
    var out: VertexOutput;

    out.position = frame.projection * frame.view * model.model * in.position;
    return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @builtin(frag_depth) f32
{
    return 0.0;
}

// @fragment
// fn fs_main()
// {
// }