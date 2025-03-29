@vs vs
layout(binding = 0) uniform vs_params {
  float inv_aspect;
};

out vec4 vert_color;

void main() {
  const vec4 colors[3] = {
    vec4(1, 1, 0, 1),
    vec4(0, 1, 1, 1),
    vec4(1, 0, 1, 1),
  };

  const vec2 positions[3] = {
    vec2(-0.6, -0.4),
    vec2(+0.6, -0.4),
    vec2(+0.0, +0.6),
  };

  vec2 position = positions[gl_VertexIndex] * vec2(inv_aspect, 1.0);

  gl_Position = vec4(position, 0.0, 1.0);
  vert_color  = colors[gl_VertexIndex];
}
@end

@fs fs
in  vec4 vert_color;
out vec4 frag_color;

void main() {
  frag_color = vert_color;
}
@end

@program triangle vs fs
