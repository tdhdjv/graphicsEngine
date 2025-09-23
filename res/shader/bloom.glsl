#version 460 core

layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

uniform sampler2D inputImage;
float kernel[9] = float[](0.01621622, 0.05405405, 0.12162162, 0.19459459, 0.22702703, 0.19459459, 0.12162162, 0.05405405, 0.01621622); 

layout(rgba16f, binding = 0) uniform image2D outputImage;

shared vec3 shared_data[384];
uniform int inputWidth;
uniform int inputHeight;
uniform int lod;
uniform int vertical;

void main() {
  //we use 4 for the vertical rows for sampling out of bounds
  ivec2 texCoord = ivec2(gl_GlobalInvocationID.xy);
  vec2 deltaUV = vec2(0.5, 0.5)/vec2(inputWidth, inputHeight);

  //sample
  const int columnSize = 16 + 8*vertical;
  const int rowSize = 40 - columnSize;
  const int columnOffset = 4*vertical;
  const int rowOffset = 4 - columnOffset;

  int sharedIndex1 = 2 * (int(gl_LocalInvocationID.x) + 16*int(gl_LocalInvocationID.y));
  int sharedIndex2 = sharedIndex1 + 1;

  sharedIndex1 %= 384;
  sharedIndex2 %= 384;

  ivec2 sampleTexCoord1 = 16 * ivec2(gl_WorkGroupID.xy) + ivec2(sharedIndex1%rowSize, sharedIndex1/rowSize) - ivec2(rowOffset, columnOffset);
  ivec2 sampleTexCoord2 = 16 * ivec2(gl_WorkGroupID.xy) + ivec2(sharedIndex2%rowSize, sharedIndex2/rowSize) - ivec2(rowOffset, columnOffset);
  vec2 sampleUVCoord1 = vec2(sampleTexCoord1)/vec2(inputWidth, inputHeight) + deltaUV; //delta uv is added to make is so that the image doesn't get offset when down sampling
  vec2 sampleUVCoord2 = vec2(sampleTexCoord2)/vec2(inputWidth, inputHeight) + deltaUV; //delta uv is added to make is so that the image doesn't get offset when down sampling
  

  if(lod == -1) {
    vec3 sample1 = texture(inputImage, sampleUVCoord1).rgb;
    vec3 sample2 = texture(inputImage, sampleUVCoord2).rgb;
    float brightness1 = dot(sample1, vec3(0.2126, 0.7152, 0.0722));
    float brightness2 = dot(sample2, vec3(0.2126, 0.7152, 0.0722));
  
    sample1 /= 1.0 + brightness1;
    sample2 /= 1.0 + brightness2;

    shared_data[sharedIndex1] = 20.0*sample1;
    shared_data[sharedIndex2] = 20.0*sample2;
    
    shared_data[sharedIndex1] = sample1;
    shared_data[sharedIndex2] = sample2;
  }
  else {
    vec3 sample1 = textureLod(inputImage, sampleUVCoord1, lod).rgb;
    vec3 sample2 = textureLod(inputImage, sampleUVCoord2, lod).rgb;

    shared_data[sharedIndex1] = sample1;
    shared_data[sharedIndex2] = sample2;
  }
  barrier();

  //draw
  vec3 color = vec3(0.0);
  int midSampleIndex = int(gl_LocalInvocationID.x) + rowOffset + rowSize*(int(gl_LocalInvocationID.y) + columnOffset);
  for(int i = -4; i <= 4; i++) {
    int sampleIndex = midSampleIndex + i * (15*int(vertical) + 1);

    float weight = kernel[i+4];
    vec3 sampleColor = shared_data[sampleIndex];
    color += sampleColor * weight;
  }
  imageStore(outputImage, texCoord, vec4(color, 1.0));
}
