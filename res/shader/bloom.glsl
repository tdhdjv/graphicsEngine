#version 460 core

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(rgba16f, binding = 0) uniform image2D outputImage;
layout(rgba16f, binding = 1) uniform image2D inputImage;

void main() {
  ivec2 texCoord = 2*ivec2(gl_GlobalInvocationID.xy);
  vec3 a = imageLoad(inputImage, texCoord+ivec2(-2, 2)).rgb;
  vec3 b = imageLoad(inputImage, texCoord+ivec2(0, 2)).rgb;
  vec3 c = imageLoad(inputImage, texCoord+ivec2(2, 2)).rgb;

  vec3 d = imageLoad(inputImage, texCoord+ivec2(-2, 0)).rgb;
  vec3 e = imageLoad(inputImage, texCoord+ivec2(0, 0)).rgb;
  vec3 f = imageLoad(inputImage, texCoord+ivec2(2, 0)).rgb;

  vec3 g = imageLoad(inputImage, texCoord+ivec2(-2, -2)).rgb;
  vec3 h = imageLoad(inputImage, texCoord+ivec2(0, -2)).rgb;
  vec3 i = imageLoad(inputImage, texCoord+ivec2(2, -2)).rgb;

  vec3 j = imageLoad(inputImage, texCoord+ivec2(1, 1)).rgb;
  vec3 k = imageLoad(inputImage, texCoord+ivec2(-1, 1)).rgb;
  vec3 l = imageLoad(inputImage, texCoord+ivec2(1, -1)).rgb;
  vec3 m = imageLoad(inputImage, texCoord+ivec2(-1, -1)).rgb;

  vec3 color = e*0.125;
  color += (a+c+g+i)*0.03125;
  color += (b+d+f+h)*0.0625;
  color += (j+k+l+m)*0.125;
  color = max(color, vec3(0.00001));

  imageStore(outputImage, texCoord/2, vec4(color, 1.0));
}

