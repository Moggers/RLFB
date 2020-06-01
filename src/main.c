#include "engine.h"
#include <cglm/vec3.h>
#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
int main(int argc, char **argv) {
  glfwInit();
  GraphicsState graphics = InitGraphics();
	Texture grassTexture;
	Texture standardTexture;
	OpenTexture(&graphics, &grassTexture, "data/textures/grass.png");
	OpenTexture(&graphics, &standardTexture, "data/textures/standard.png");
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
  for (uint32_t i = 0; i < 511; i++) {
    for (uint32_t t = 0; t < 511; t++) {
      vec3 pos1 = {i, heightMap[i][t], t};
      glm_vec3_mul(pos1, (vec3){0.1, 6, 0.1}, pos1);
      vec3 pos2 = {i + 1, heightMap[i + 1][t], t};
      glm_vec3_mul(pos2, (vec3){0.1, 6, 0.1}, pos2);
      vec3 pos3 = {i + 1, heightMap[i + 1][t + 1], t + 1};
      glm_vec3_mul(pos3, (vec3){0.1, 6, 0.1}, pos3);
      vec3 pos4 = {i, heightMap[i][t + 1], t + 1};
      glm_vec3_mul(pos4, (vec3){0.1, 6, 0.1}, pos4);
      vec3 cross1 = {pos2[0] - pos1[0], pos2[1] - pos1[1], pos2[2] - pos1[2]};
      vec3 cross2 = {pos3[0] - pos1[0], pos3[1] - pos1[1], pos3[2] - pos1[2]};
      glm_normalize(cross1);
      glm_normalize(cross2);
      vec3 norm;
      glm_vec3_cross(cross2, cross1, norm);
      // Row i, column t, stride of 6
      memcpy(&terrain.vertices[(i + t * 512) * 6],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {0.8, 1, 0.8, 1},
                       .position = {pos2[0], pos2[1], pos2[2]}},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 1],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {0.8, 1, 0.8, 1},
                       .position = {pos1[0], pos1[1], pos1[2]}},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 2],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {0.8, 1, 0.8, 1},
                       .position = {pos4[0], pos4[1], pos4[2]}},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 3],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {0.8, 1, 0.8, 1},
                       .position = {pos2[0], pos2[1], pos2[2]}},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 4],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {0.8, 1, 0.8, 1},
                       .position = {pos4[0], pos4[1], pos4[2]}},
             sizeof(Vertex));
      memcpy(&terrain.vertices[(i + t * 512) * 6 + 5],
             &(Vertex){.normal = {norm[0], norm[1], norm[2], 1},
                       .color = {0.8, 1, 0.8, 1},
                       .position = {pos3[0], pos3[1], pos3[2]}},
             sizeof(Vertex));
    }
  }
  uint32_t terrainDef = CreateEntityDef(&graphics, &terrain);
  Instance inst = {.position = {0, 0, 0}, .scale = {1, 1, 1}};
  glm_mat4_identity(inst.rotation);
  AddEntityInstance(&graphics.entities[terrainDef], inst);

  while (true) {
    if (glfwWindowShouldClose(graphics.window)) {
      return 0;
    }
    glfwPollEvents();
    MoveCamera(&graphics);
    DrawGraphics(&graphics);
  }
}