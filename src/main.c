#include "engine.h"
#include <GLFW/glfw3.h>
#include <cglm/affine-mat.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

typedef struct Unit {
  vec3 target;
  Instance *instance;
} Unit;

void updateUnit(Unit *unit, InputState *state) {
  if (state->mouseButtons & 2) {
    unit->target[0] = state->worldMouse[0];
    unit->target[1] = state->worldMouse[1];
    unit->target[2] = state->worldMouse[2];
  }
  if (unit->target[0] == 0 && unit->target[1] == 0 && unit->target[2] == 0) {
    unit->target[0] = unit->instance->position[0];
    unit->target[1] = unit->instance->position[1];
    unit->target[2] = unit->instance->position[2];
  }
  vec3 vel;
  glm_vec3_sub(unit->target, unit->instance->position, vel);
  glm_normalize(vel);
  glm_vec3_mul(vel, (vec3){0.002, 0.002, 0.002}, vel);
  glm_vec3_add(unit->instance->position, vel, unit->instance->position);
  unit->instance->position[1] =
      stb_perlin_fbm_noise3(unit->instance->position[0] / 512.f, 0,
                            unit->instance->position[2] / 512.f, 2.3, 0.3, 6) *
      100;
  *unit->instance->dirty = true;
}

int main(int argc, char **argv) {
  glfwInit();
  GraphicsState graphics = InitGraphics();
  Texture grassTexture, standardTexture, dirtTexture, albedoTexture,
      waterTexture, treeTexture;
  OpenTexture(&graphics, &grassTexture, "data/textures/grass.png");
  OpenTexture(&graphics, &dirtTexture, "data/textures/dirt.png");
  OpenTexture(&graphics, &standardTexture, "data/textures/standard.png");
  OpenTexture(&graphics, &albedoTexture, "data/textures/albedo.png");
  OpenTexture(&graphics, &waterTexture, "data/textures/blue_transparent.png");
  OpenTexture(&graphics, &treeTexture, "data/textures/tree.png");
  // == TERRAIN
  Model terrain = (Model){};
  terrain.vertices =
      malloc(sizeof(Vertex) * 512 * 512 *
             6); // 512x512 quads, each quad is 2 tris, with 3 vertices each
  terrain.vertexCount = 512 * 512 * 6;
  float heightMap[512][512];
  for (float i = 0; i < 512; i++) {
    for (float t = 0; t < 512; t++) {
      heightMap[(int)i][(int)t] =
          stb_perlin_fbm_noise3(i / 512, 0, t / 512, 2.3, 0.3, 6);
    }
  }
  vec3 terraincoefficient = {1, 100, 1};
  for (uint32_t i = 0; i < 511; i++) {
    for (uint32_t t = 0; t < 511; t++) {
      vec3 pos1 = {i, heightMap[i][t], t};
      vec2 uvcoord1 = {0, 0};
      glm_vec3_mul(pos1, terraincoefficient, pos1);
      vec3 pos2 = {i + 1, heightMap[i + 1][t], t};
      vec2 uvcoord2 = {1, 0};
      glm_vec3_mul(pos2, terraincoefficient, pos2);
      vec3 pos3 = {i + 1, heightMap[i + 1][t + 1], t + 1};
      vec2 uvcoord3 = {1, 1};
      glm_vec3_mul(pos3, terraincoefficient, pos3);
      vec3 pos4 = {i, heightMap[i][t + 1], t + 1};
      vec2 uvcoord4 = {0, 1};
      glm_vec3_mul(pos4, terraincoefficient, pos4);
      vec3 cross1 = {pos2[0] - pos1[0], pos2[1] - pos1[1], pos2[2] - pos1[2]};
      vec3 cross2 = {pos4[0] - pos1[0], pos4[1] - pos1[1], pos4[2] - pos1[2]};
      glm_normalize(cross1);
      glm_normalize(cross2);
      vec3 norm;
      glm_vec3_cross(cross2, cross1, norm);
      float blendFactor = glm_vec3_dot(norm, (vec3){0, 1, 0}) * 10 - 9.1;
      // Row i, column t, stride of 6
      memcpy(&terrain.vertices[(i + t * 512) * 6],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {1, 1, 1, 1},
                       .position = {pos2[0], pos2[1], pos2[2]},
                       .uvcoords = {uvcoord2[0], uvcoord2[1]},
                       .blendFactor = blendFactor},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 1],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {1, 1, 1, 1},
                       .position = {pos1[0], pos1[1], pos1[2]},
                       .uvcoords = {uvcoord1[0], uvcoord1[1]},
                       .blendFactor = blendFactor},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 2],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {1, 1, 1, 1},
                       .position = {pos4[0], pos4[1], pos4[2]},
                       .uvcoords = {uvcoord4[0], uvcoord4[1]},
                       .blendFactor = blendFactor},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 3],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {1, 1, 1, 1},
                       .position = {pos2[0], pos2[1], pos2[2]},
                       .uvcoords = {uvcoord2[0], uvcoord2[1]},
                       .blendFactor = blendFactor},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 4],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {1, 1, 1., 1},
                       .position = {pos4[0], pos4[1], pos4[2]},
                       .uvcoords = {uvcoord4[0], uvcoord4[1]},
                       .blendFactor = blendFactor},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 5],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {1, 1, 1, 1},
                       .position = {pos3[0], pos3[1], pos3[2]},
                       .uvcoords = {uvcoord3[0], uvcoord3[1]},
                       .blendFactor = blendFactor},
             sizeof(Vertex));
    }
  }
  uint32_t terrainDef = CreateEntityDef(&graphics, &terrain);
  Instance terrainInstance = {.position = {0, 0, 0},
                              .scale = {1, 1, 1},
                              .textureId1 = grassTexture.textureId,
                              .textureId2 = dirtTexture.textureId};
  glm_mat4_identity(terrainInstance.rotation);
  AddEntityInstance(&graphics.entities[terrainDef], terrainInstance);
  // == WATER
  Model water = (Model){.vertexCount = 6,
                        .vertices = (Vertex[6]){{.position = {0, 0, 0},
                                                 .color = {1, 1, 1, 1},
                                                 .normal = {0, 1, 0},
                                                 .uvcoords = {0, 0}},
                                                {.position = {0, 0, 1},
                                                 .color = {1, 1, 1, 1},
                                                 .normal = {0, 1, 0},
                                                 .uvcoords = {0, 0}},
                                                {.position = {1, 0, 0},
                                                 .color = {1, 1, 1, 1},
                                                 .normal = {0, 1, 0},
                                                 .uvcoords = {0, 0}},
                                                {.position = {1, 0, 0},
                                                 .color = {1, 1, 1, 1},
                                                 .normal = {0, 1, 0},
                                                 .uvcoords = {0, 0}},
                                                {.position = {0, 0, 1},
                                                 .color = {1, 1, 1, 1},
                                                 .normal = {0, 1, 0},
                                                 .uvcoords = {0, 0}},
                                                {.position = {1, 0, 1},
                                                 .color = {1, 1, 1, 1},
                                                 .normal = {0, 1, 0},
                                                 .uvcoords = {0, 0}}}};
  uint32_t waterDef = CreateEntityDef(&graphics, &water);
  Instance waterInstance = {.position = {0, 0, 0},
                            .scale = {512, 1, 512},
                            .textureId1 = waterTexture.textureId,
                            .textureId2 = waterTexture.textureId};
  glm_mat4_identity(waterInstance.rotation);
  AddEntityInstance(&graphics.entities[waterDef], waterInstance);
  // == TREE
  Model tree = (Model){};
  tree.vertexCount = 24;
  tree.vertices = (Vertex[24]){
      {.position = {-1, 0, 0},
       .uvcoords = {0, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {-1, 2, 0},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {1, 0, 0},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {-1, 2, 0},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {1, 2, 0},
       .uvcoords = {1, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {1, 0, 0},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {-1, 2, 0},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {-1, 0, 0},
       .uvcoords = {0, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {1, 0, 0},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {1, 2, 0},
       .uvcoords = {1, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {-1, 2, 0},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {1, 0, 0},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 0, -1},
       .uvcoords = {0, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 2, -1},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 0, 1},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 2, -1},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 2, 1},
       .uvcoords = {1, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 0, 1},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 2, -1},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 0, -1},
       .uvcoords = {0, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 0, 1},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 2, 1},
       .uvcoords = {1, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 2, -1},
       .uvcoords = {0, 0},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
      {.position = {0, 0, 1},
       .uvcoords = {1, 1},
       .color = {1, 1, 1, 1},
       .normal = {0, 0, 1}},
  };
  uint32_t treeDef = CreateEntityDef(&graphics, &tree);
  for (uint32_t t = 0; t < 1024; t++) {
    vec3 pos;
    while (1) {
      pos[0] = (float)(rand() % (512 * 512)) / 512;
      pos[2] = (float)(rand() % (512 * 512)) / 512;
      pos[1] =
          stb_perlin_fbm_noise3(pos[0] / 512, 0, pos[2] / 512, 2.3, 0.3, 6) *
          100;
      if (pos[1] > 0) {
        break;
      }
    }
    Instance treeInstance = {
        .position = {pos[0], pos[1], pos[2]},
        .scale = {0.4, 1, 0.4},
        .textureId1 = treeTexture.textureId,
        .textureId2 = treeTexture.textureId,
    };
    glm_mat4_identity(treeInstance.rotation);
    AddEntityInstance(&graphics.entities[treeDef], treeInstance);
  }
  // == STANDARD
  Model standard = (Model){.vertexCount = 6,
                           .vertices = (Vertex[6]){{.position = {-0.5, 0, 0},
                                                    .color = {1, 1, 1, 1},
                                                    .normal = {0, 1, 0},
                                                    .uvcoords = {0, 1}},
                                                   {.position = {-0.5, 1, 0},
                                                    .color = {1, 1, 1, 1},
                                                    .normal = {0, 1, 0},
                                                    .uvcoords = {0, 0}},
                                                   {.position = {0.5, 1, 0},
                                                    .color = {1, 1, 1, 1},
                                                    .normal = {0, 1, 0},
                                                    .uvcoords = {1, 0}},
                                                   {.position = {0.5, 0, 0},
                                                    .color = {1, 1, 1, 1},
                                                    .normal = {0, 1, 0},
                                                    .uvcoords = {1, 1}},
                                                   {.position = {-0.5, 0, 0},
                                                    .color = {1, 1, 1, 1},
                                                    .normal = {0, 1, 0},
                                                    .uvcoords = {0, 1}},
                                                   {.position = {0.5, 1, 0},
                                                    .color = {1, 1, 1, 1},
                                                    .normal = {0, 1, 0},
                                                    .uvcoords = {1, 0}}}};
  uint32_t standardDef = CreateEntityDef(&graphics, &standard);
  Instance standardInstance = {
      .position = {200,
                   stb_perlin_fbm_noise3(200.f / 512.f, 0, 100.f / 512.f, 2.3,
                                         0.3, 6) *
                       100,
                   // 0,
                   100},
      .scale = {0.01 * standardTexture.width, 0.01 * standardTexture.height, 1},
      .textureId1 = standardTexture.textureId,
      .textureId2 = standardTexture.textureId};
  glm_mat4_identity(standardInstance.rotation);
  uint32_t standardId =
      AddEntityInstance(&graphics.entities[standardDef], standardInstance);

  Unit standardUnit = {
      .instance = &graphics.entities[standardDef].instances[standardId]};

  while (true) {
    if (glfwWindowShouldClose(graphics.window)) {
      return 0;
    }
    glfwPollEvents();
    MoveCamera(&graphics);
    updateUnit(&standardUnit, &graphics.input);
    DrawGraphics(&graphics);
  }
}
