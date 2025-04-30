#include <gl/glew.h>
#include <gl/freeglut.h>
#include <glm/glm.hpp>
#include "Model.h"
#include "FPSCamera.h"
#include "Shader.h"

#define roomSizeX 200.f
#define roomSizeY 60.f
#define roomSizeZ 200.f

#define SkyboxSize 600.f

#define EatBreadDistance 5.f
#define CloseToBreadDistance 200.f

void loadTex(int i, char *filename, GLuint *texture);

void drawRect(GLuint texture);
void drawCube(GLuint texture);

void initCube(Shader shader);
void drawCube(Shader shader, GLuint diffuse, GLuint bump, GLuint spec);

void drawSkybox(GLuint *texture);

void initBoxCollidersProperty();
void initSingleBoxCollider(glm::vec3 pos, glm::vec3 scalar);
void setBoxColliderBoundary(FPSCamera *cam);
void drawBoxColliders(GLuint *texture);
void drawBoxColliders(Shader shader, GLuint diffuse, GLuint bump, GLuint spec,
                      FPSCamera *cam);

void initBreadModels();
void drawBreadModels();
void playBreadEatenEffect(FPSCamera *cam);
void deleteBreadModels();

void detectBreadBeingEaten(FPSCamera *cam);

void drawMenuSceneUIText(FPSCamera *cam);
void drawGameSceneUIText(FPSCamera *cam, int x, int y);

void setupLights();
