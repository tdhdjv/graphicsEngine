#ifndef MESH_HEADER
#define MESH_HEADER

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/util.h>
#include <cglm/vec3.h>

#include <stdint.h>
#include <stdio.h>

#include "arena.c"

#define FLOATS_PER_VERTEX 6

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  size_t indexCount;
} Mesh;

Mesh gen_mesh(float *vertexData, uint32_t *indexData, size_t vertexCount,
              size_t indexCount) {
  GLuint VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, FLOATS_PER_VERTEX * sizeof(float) * vertexCount, vertexData,
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indexCount,
               indexData, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float), (void *)0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, FLOATS_PER_VERTEX * sizeof(float),(void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  return (Mesh){VAO, VBO, EBO, indexCount};
}

Mesh gen_quad(Arena* arena, vec3 horizontal, vec3 vertical, uint32_t subDivision) {
  size_t vertexCount = (subDivision + 2) * (subDivision + 2);
  size_t indexCount = 6 * (subDivision + 1) * (subDivision + 1);
  float vertexData[vertexCount * FLOATS_PER_VERTEX];
  uint32_t indexData[indexCount];

  vec3 normal;
  glm_cross(horizontal, vertical, normal);
  glm_normalize(normal);

  uint32_t n = 0;

  for (uint32_t i = 0; i < subDivision + 2; i++) {
    for (uint32_t j = 0; j < subDivision + 2; j++) {
      float u = ((float)i) / (float)(subDivision + 1) - 0.5f;
      float v = ((float)j) / (float)(subDivision + 1) - 0.5f;
      vertexData[n++] = horizontal[0] * u + vertical[0] * v;
      vertexData[n++] = horizontal[1] * u + vertical[1] * v;
      vertexData[n++] = horizontal[2] * u + vertical[2] * v;

      vertexData[n++] = normal[0];
      vertexData[n++] = normal[1];
      vertexData[n++] = normal[2];
    }
  }

  n = 0;

  for (uint32_t i = 0; i < subDivision + 1; i++) {
    for (uint32_t j = 0; j < subDivision + 1; j++) {
      size_t firstIndex = (j + i * (subDivision + 2));
      indexData[n++] = firstIndex;
      indexData[n++] = firstIndex + (subDivision + 2);
      indexData[n++] = firstIndex + 1;
      indexData[n++] = firstIndex + 1;
      indexData[n++] = firstIndex + (subDivision + 2);
      indexData[n++] = firstIndex + (subDivision + 2) + 1;
    }
  }
  return gen_mesh(vertexData, indexData, vertexCount, indexCount);
}


/* Math behind this is explained in /documentation/icosphere.md*/
Mesh gen_icosphere(Arena* arena, uint32_t subDivision) {
  size_t vertexCount = 10 * (subDivision + 1) * (subDivision + 1) + 2;
  size_t indexCount = 60 * (subDivision + 1) * (subDivision + 1);

  Mesh result = {0};

  ArenaMark mark = create_arena_mark(arena);
  float* vertexData = arena_alloc_array(arena, float, vertexCount * FLOATS_PER_VERTEX);
  uint32_t* indexData = arena_alloc_array(arena, int, indexCount);

  float phi = 0.5 * (1.0 + sqrt(5.0));
  vec3 northPole = {0.0, 1.0, phi};
  vec3 southPole = {0.0, -1.0, -phi};
  vec3 strip1[4] = {
      {phi, 0.0, 1.0}, {1.0, phi, 0.0}, {phi, 0.0, -1.0}, {0.0, 1.0, -phi}};
  vec3 strip2[4] = {
      {1.0, phi, 0.0}, {-1.0, phi, 0.0}, {0.0, 1.0, -phi}, {-phi, 0.0, -1.0}};
  vec3 strip3[4] = {
      {-1.0, phi, 0.0}, {-phi, 0.0, 1.0}, {-phi, 0.0, -1.0}, {-1.0, -phi, 0.0}};
  vec3 strip4[4] = {
      {-phi, 0.0, 1.0}, {0.0, -1.0, phi}, {-1.0, -phi, 0.0}, {1.0, -phi, 0.0}};
  vec3 strip5[4] = {
      {0.0, -1.0, phi}, {phi, 0.0, 1.0}, {1.0, -phi, 0.0}, {phi, 0.0, -1.0}};

  vec3 *strips[5] = {strip1, strip2, strip3, strip4, strip5};

  size_t n = 0;
  vec3 normalizedNorth;
  glm_normalize_to(northPole, normalizedNorth);
  vertexData[n++] = normalizedNorth[0];
  vertexData[n++] = normalizedNorth[1];
  vertexData[n++] = normalizedNorth[2];

  vertexData[n++] = normalizedNorth[0];
  vertexData[n++] = normalizedNorth[1];
  vertexData[n++] = normalizedNorth[2];

  for (int i = 0; i < 5; i++) {
    vec3 *strip = strips[i];
    for (uint32_t j = 0; j < subDivision + 1; j++) {
      for (uint32_t k = 0; k < 2 * (subDivision + 1); k++) {
        vec3 vertex;
        glm_vec3_zero(vertex);

        float u = (float)(k) / (float)(subDivision + 1);
        float v = (float)(j) / (float)(subDivision + 1);

        vec3 U, V, W;

        if (v >= u) {
          v = 1.0 - v;
          glm_vec3_copy(strip[1], U);
          glm_vec3_copy(strip[0], V);
          glm_vec3_copy(northPole, W);
        } else if (u <= 1.0) {
          u = 1.0 - u;
          glm_vec3_copy(strip[0], U);
          glm_vec3_copy(strip[1], V);
          glm_vec3_copy(strip[2], W);
        } else if (u <= v + 1.0) {
          u = u - 1.0;
          v = 1.0 - v;
          glm_vec3_copy(strip[3], U);
          glm_vec3_copy(strip[2], V);
          glm_vec3_copy(strip[1], W);
        } else {
          u = 2.0 - u;
          glm_vec3_copy(strip[2], U);
          glm_vec3_copy(strip[3], V);
          glm_vec3_copy(southPole, W);
        }

        float w = 1.0 - u - v;

        glm_vec3_scale(U, u, U);
        glm_vec3_scale(V, v, V);
        glm_vec3_scale(W, w, W);

        glm_vec3_add(vertex, U, vertex);
        glm_vec3_add(vertex, V, vertex);
        glm_vec3_add(vertex, W, vertex);

        glm_normalize(vertex);

        vertexData[n++] = vertex[0];
        vertexData[n++] = vertex[1];
        vertexData[n++] = vertex[2];

        vertexData[n++] = vertex[0];
        vertexData[n++] = vertex[1];
        vertexData[n++] = vertex[2];
      }
    }
  }

  glm_normalize(southPole);
  vertexData[n++] = southPole[0];
  vertexData[n++] = southPole[1];
  vertexData[n++] = southPole[2];

  vertexData[n++] = southPole[0];
  vertexData[n++] = southPole[1];
  vertexData[n++] = southPole[2];

  n = 0;
  size_t vertexPerRow = 2 * (subDivision + 1);
  size_t vertexPerStrip = vertexPerRow * (subDivision + 1);

  for (int i = 0; i < 5; i++) {
    for (uint32_t j = 0; j < subDivision; j++) {
      for (uint32_t k = 0; k < 2 * subDivision + 1; k++) {
        uint32_t firstIndex = k + vertexPerRow * j + vertexPerStrip * i + 1;

        indexData[n++] = firstIndex;
        indexData[n++] = firstIndex + 1;
        indexData[n++] = firstIndex + vertexPerRow + 1;

        indexData[n++] = firstIndex;
        indexData[n++] = firstIndex + vertexPerRow + 1;
        indexData[n++] = firstIndex + vertexPerRow;
      }
    }

    for (uint32_t k = 0; k < subDivision + 1; k++) {
      uint32_t firstIndex = k + vertexPerStrip * (i + 1) - vertexPerRow + 1;
      uint32_t secondIndex = (subDivision + 1 - k) * vertexPerRow +
                             vertexPerStrip * ((i + 1) % 5) + 1;
      uint32_t thirdIndex = secondIndex - vertexPerRow;
      uint32_t forthIndex = firstIndex + 1;

      if (k == 0)
        secondIndex = 0;

      indexData[n++] = firstIndex;
      indexData[n++] = forthIndex;
      indexData[n++] = thirdIndex;

      indexData[n++] = firstIndex;
      indexData[n++] = thirdIndex;
      indexData[n++] = secondIndex;
    }

    for (uint32_t k = 0; k < subDivision; k++) {
      uint32_t firstIndex =
          subDivision + 1 + k + vertexPerStrip * (i + 1) - vertexPerRow + 1;
      uint32_t secondIndex = k + vertexPerStrip * ((i + 1) % 5) + 1;
      uint32_t thirdIndex = secondIndex + 1;
      uint32_t forthIndex = firstIndex + 1;

      indexData[n++] = firstIndex;
      indexData[n++] = forthIndex;
      indexData[n++] = thirdIndex;

      indexData[n++] = firstIndex;
      indexData[n++] = thirdIndex;
      indexData[n++] = secondIndex;
    }

    for (uint32_t k = 0; k < subDivision + 1; k++) {
      uint32_t firstIndex = vertexPerRow * (k + 1) + vertexPerStrip * i;
      uint32_t secondIndex = firstIndex + vertexPerRow;
      uint32_t thirdIndex = vertexPerRow - k + vertexPerStrip * ((i + 1) % 5);
      uint32_t forthIndex = thirdIndex + 1;

      if (k == subDivision)
        secondIndex = thirdIndex - 1;
      if (k == 0)
        forthIndex = vertexCount - 1;

      indexData[n++] = firstIndex;
      indexData[n++] = forthIndex;
      indexData[n++] = thirdIndex;

      indexData[n++] = firstIndex;
      indexData[n++] = thirdIndex;
      indexData[n++] = secondIndex;
    }
  }
  result = gen_mesh(vertexData, indexData, vertexCount, n);
  arena_return_to_mark(&mark);

  return result;
}

#include "fileIO.c"
#include "hashtable.c"


typedef struct{
  uint32_t positionIndex;
  uint32_t normalIndex;
  uint32_t textureIndex;
} PosNormTex;

bool is_equal(PosNormTex a, PosNormTex b) {
  return a.positionIndex == b.positionIndex && a.normalIndex == b.normalIndex && a.textureIndex == b.textureIndex;
}

uint64_t hash(PosNormTex key) {
  uint64_t hash;
  hash = key.positionIndex;
  hash <<= 21;
  hash = key.normalIndex;
  hash <<= 21;
  hash = key.textureIndex;
  return hash;
}

DEFINE_HASH_TABLE(PosNormTex, uint32_t, hash, is_equal);

Mesh gen_obj_model(Arena* arena, const char* filename) {
  String objSource = read_file(arena, filename);
  ObjModel model = parse_obj(arena, objSource);
  Mesh result = {0};

  ArenaMark mark = create_arena_mark(arena);

  KeyValue(PosNormTex, uint32_t)* data = arena_alloc_array(arena, KeyValue(PosNormTex, uint32_t), 3*model.faceCount);
  HashTable(PosNormTex, uint32_t) table = create_hash_table(PosNormTex, uint32_t, data, 3*model.faceCount);

  size_t vertexCount = 0;
  const size_t indexCount = 3*model.faceCount;

  //count
  //for every face
  for(size_t i = 0; i < model.faceCount; i++) {
    //for every vertex per face
    for(size_t j = 0; j < 3; j++) {
      PosNormTex pnt = (PosNormTex){model.positionIndices[i][j], model.normalIndices[i][j], 0};
      if(hash_table_contains(PosNormTex, uint32_t, &table, pnt)) continue;
      hash_table_set(PosNormTex, uint32_t, &table, pnt, vertexCount++);
    }
  }

  float* vertexData = arena_alloc_array(arena, float, FLOATS_PER_VERTEX*vertexCount);
  uint32_t floatIndex = 0;
  uint32_t vertexIndex = 0;

  //define the vertices
  for(size_t i = 0; i < model.faceCount; i++) {
    for(size_t j = 0; j < 3; j++) {
      PosNormTex pnt = (PosNormTex){model.positionIndices[i][j], model.normalIndices[i][j], 0};
      //check if this is duplicate vertex
      vertexIndex = floatIndex/FLOATS_PER_VERTEX;
      if(hash_table_get(PosNormTex, uint32_t, &table, pnt, 0) != vertexIndex) continue;
      
      //-1 is added since objs are 1 indexed for some reason
      vertexData[floatIndex++] = model.positions[pnt.positionIndex-1][0];
      vertexData[floatIndex++] = model.positions[pnt.positionIndex-1][1];
      vertexData[floatIndex++] = model.positions[pnt.positionIndex-1][2];

      vertexData[floatIndex++] = model.normals[pnt.normalIndex-1][0];
      vertexData[floatIndex++] = model.normals[pnt.normalIndex-1][1];
      vertexData[floatIndex++] = model.normals[pnt.normalIndex-1][2]; 
    }
  }

  uint32_t* indexData = arena_alloc_array(arena, uint32_t, 3*model.faceCount);
  uint32_t uintIndex = 0;

  //define indices 
  for(size_t i = 0; i < model.faceCount; i++) {
    for(size_t j = 0; j < 3; j++) {
      PosNormTex pnt = (PosNormTex){model.positionIndices[i][j], model.normalIndices[i][j], 0};
      uint32_t index = hash_table_get(PosNormTex, uint32_t, &table, pnt,0);
      //check if this is duplicate vertex
      indexData[uintIndex++] = index;
    }
  }
  
  result = gen_mesh(vertexData, indexData, vertexCount, indexCount);
  arena_return_to_mark(&mark);
  return result;
}

#endif
