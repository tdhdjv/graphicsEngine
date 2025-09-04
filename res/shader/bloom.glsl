#version 460 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D outputImage;
layout(rgba16f, binding = 1) uniform image2D inputImage;

void main() {
  ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
  vec3 color = imageLoad(inputImage, texCoord).rgb;
  imageStore(outputImage, texCoord, vec4(vec3(1.0) - color, 1.0));
}

