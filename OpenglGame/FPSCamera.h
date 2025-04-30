#include <gl/freeglut.h>
#include <glm/glm.hpp>
#include <vector>
#include "PhysicsEngine.h"
using namespace std;

class FPSCamera {
public:
  FPSCamera();
  ~FPSCamera();
  void resetWinSize(int w, int h);

  void rotate(GLfloat const pitchRad, GLfloat const yawRad);
  void keyPressed(const unsigned char key);
  void keyUp(const unsigned char key);

  void setSceneOuterBoundary(float x1, float z1, float x2, float z2);
  void setSceneInnerBoundary(float x1, float y1, float z1, float x2, float y2,
                             float z2);

  void updateCameraMovement();

  glm::vec3 getForward();

  bool detectPlayerEatingBread(glm::vec3 breadPos, float dist);

  bool detectPlayerCloseToBread(glm::vec3 breadPos, float dist);

  glm::vec3 cameraPos;
  glm::vec3 targetPos;

  glm::mat4 viewMatrix;
  glm::mat4 projectionMatrix;

private:
  void updateCameraHoriMovement();
  void updateCameraVertMovement();
  void updateView();

  int winH = 0;
  int winW = 0;

  bool isWPressing, isSPressing, isAPressing, isDPressing;

  GLfloat pfov, pratio, pnear, pfar;

  GLfloat roll, pitch, yaw;

  PhysicsEngine *physicsEngine;
};
