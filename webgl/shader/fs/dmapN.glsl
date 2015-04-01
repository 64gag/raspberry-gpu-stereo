precision lowp float;

uniform sampler2D stereo;
uniform sampler2D disparity_map;
uniform float disparity2texel;

varying vec2 tc; /* Texture coordinates */

void main(void)
{
  float matching = texture2D(stereo, tc).r;
  float disparity = texture2D(disparity_map, tc).r;

  vec3 candidates = vec3(
    texture2D(stereo, tc - vec2((disparity + 0.0039) * disparity2texel, 0.0)).g,
    texture2D(stereo, tc - vec2(disparity * disparity2texel, 0.0)).g,
    texture2D(stereo, tc - vec2((disparity - 0.0039) * disparity2texel, 0.0)).g
  );

  mediump vec3 similarity = ceil(abs(matching - candidates) * 255.0);
  similarity.rg += vec2(0.0078, 0.0039);

  gl_FragColor = vec4(disparity + fract(min(min(min(similarity.r, similarity.g), similarity.b), 150.0039)) - 0.0039, 0.0, 0.0, 1.0);
}
