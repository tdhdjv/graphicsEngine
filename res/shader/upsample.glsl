#version 460 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D outputImage;
layout(rgba16f, binding = 1) uniform image2D inputImage;

void main() {
  ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
  ivec2 texCoordHalf = texCoord/2; 
  vec3 a = imageLoad(inputImage, texCoordHalf+ivec2(-1, 1)).rgb;
  vec3 b = imageLoad(inputImage, texCoordHalf+ivec2(0, 1)).rgb;
  vec3 c = imageLoad(inputImage, texCoordHalf+ivec2(1, 1)).rgb;

  vec3 d = imageLoad(inputImage, texCoordHalf+ivec2(-1, 0)).rgb;
  vec3 e = imageLoad(inputImage, texCoordHalf+ivec2(0, 0)).rgb;
  vec3 f = imageLoad(inputImage, texCoordHalf+ivec2(1, 0)).rgb;

  vec3 g = imageLoad(inputImage, texCoordHalf+ivec2(-1, -1)).rgb;
  vec3 h = imageLoad(inputImage, texCoordHalf+ivec2(0, -1)).rgb;
  vec3 i = imageLoad(inputImage, texCoordHalf+ivec2(1, -1)).rgb;

  vec3 color = e*0.25;
  color += (a+c+g+i)*0.0625;
  color += (b+d+f+h)*0.125;

  imageStore(outputImage, texCoord, vec4(color, 1.0));
}
