#ifndef PHYSICSENGINE_H
#define PHYSICSENGINE_H

#include <glm/glm.hpp>
#include <iostream>
#include <vector>
using namespace std;

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) < (y) ? (y) : (x))

#define HeroHeight 7.5f

#define GravityAcceler -9.8f

#define MoveSpeed 0.15f
#define BoundaryGap 1.0f
#define JumpInitialSpeed 12.0f
#define JumpFactor 0.04f
#define GravityFactor 0.04f

struct dot {
  float x;
  float y;
  dot(float _x, float _y) : x(_x), y(_y) {}
};

class PhysicsEngine {
public:
  PhysicsEngine();
  ~PhysicsEngine();

  void setSceneOuterBoundary(float x1, float z1, float x2, float z2);

  void outCollisionTest(glm::vec3 &cameraPos, glm::vec3 &targetPos);

  void setSceneInnerBoundary(float x1, float y1, float z1, float x2, float y2,
                             float z2);

  void inCollisionTest(glm::vec3 &cameraPos, glm::vec3 &targetPos);

  bool isJumping;
  void jumpAndUpdateVelocity();

  void updateCameraVertMovement(glm::vec3 &cameraPos, glm::vec3 &targetPos);

private:
  void inCollisionTestWithHeight(float x1, float y1, float z1, float x2,
                                 float y2, float z2, glm::vec3 &cameraPos,
                                 glm::vec3 &targetPos);

  void inCollisionTestXZ(float x1, float z1, float x2, float z2,
                         glm::vec3 &cameraPos, glm::vec3 &targetPos);

  void outCollisionTestXZ(float x1, float z1, float x2, float z2,
                          glm::vec3 &cameraPos, glm::vec3 &targetPos);

  glm::vec3 velocity;
  glm::vec3 gravity;
  glm::vec3 accelerUp;

  glm::vec4 outerBoundary;
  vector<glm::vec3> innerBoundaryMin;
  vector<glm::vec3> innerBoundaryMax;
};

#endif
