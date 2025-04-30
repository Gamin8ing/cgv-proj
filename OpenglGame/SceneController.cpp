#define _CRT_SECURE_NO_WARNINGS

#include "SceneController.h"
#include "ParticleSystem.h"
#include <iostream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

vector<glm::vec3> boxPosition;
vector<glm::vec3> boxScale;
static int boxSum = 0;

vector<Model *> breadSet;
vector<ParticleSystem> breadEatenEffectSet;
vector<bool> isBreadEatenSet;
static int eatenBreadNum = 0;
static int closeToBreadIndex = -1;

static float angle = 0.0f;

GLfloat LightAmbient[] = {0.5f, 0.5f, 0.5f, 1.0f};
GLfloat LightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
GLfloat LightPosition[] = {0.0f, SkyboxSize / 2.f, 0.f, 1.0f};
GLfloat LightPosition2[] = {-SkyboxSize / 2.f, -SkyboxSize / 2.f,
                            SkyboxSize / 2.f, 1.0f};
GLfloat LightPosition3[] = {SkyboxSize / 2.f, -SkyboxSize / 2.f,
                            SkyboxSize / 2.f, 1.0f};
GLfloat LightPosition4[] = {0.0f, -SkyboxSize / 2.f, -SkyboxSize / 2.f, 1.0f};

void drawRect(GLuint texture) {
  glEnable(GL_TEXTURE_2D);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  glBindTexture(GL_TEXTURE_2D, texture);
  const GLfloat x1 = -0.5, x2 = 0.5;
  const GLfloat y1 = -0.5, y2 = 0.5;
  const GLfloat point[4][2] = {{x1, y1}, {x1, y2}, {x2, y2}, {x2, y1}};
  int dir[4][2] = {{1, 0}, {1, 1}, {0, 1}, {0, 0}};
  glBegin(GL_QUADS);

  for (int i = 0; i < 4; i++) {
    glTexCoord2iv(dir[i]);
    glVertex2fv(point[i]);
  }
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);

  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
}

struct Vertex {
  glm::vec3 pos;
  glm::vec3 norm;
  glm::vec2 texC;
  glm::vec3 tangent;
  glm::vec3 bitangent;

  Vertex(glm::vec3 p, glm::vec3 n, glm::vec2 t) {
    pos = p;
    norm = n;
    texC = t;
  }
};

GLuint CubeVAO, CubeVBO, CubeEBO;
vector<Vertex> CubeVertices;
vector<unsigned int> CubeIndices = {
    0,  1,  2,  0,  2,  3,  4,  5,  6,  4,  6,  7,  8,  9,  10, 8,  10, 11,
    12, 13, 14, 12, 14, 15, 16, 17, 18, 16, 18, 19, 20, 21, 22, 20, 22, 23};

void initCube(Shader shader) {

  const GLfloat x = 0.5;
  const GLfloat y = 0.5;
  const GLfloat z = 0.5;

  CubeVertices.push_back(
      Vertex(glm::vec3(x, y, z), glm::vec3(0, 0, 1), glm::vec2(1, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, -y, z), glm::vec3(0, 0, 1), glm::vec2(1, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, -y, z), glm::vec3(0, 0, 1), glm::vec2(0, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, y, z), glm::vec3(0, 0, 1), glm::vec2(0, 0)));

  CubeVertices.push_back(
      Vertex(glm::vec3(x, y, z), glm::vec3(1, 0, 0), glm::vec2(0, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, y, -z), glm::vec3(1, 0, 0), glm::vec2(1, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, -y, -z), glm::vec3(1, 0, 0), glm::vec2(1, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, -y, z), glm::vec3(1, 0, 0), glm::vec2(0, 1)));

  CubeVertices.push_back(
      Vertex(glm::vec3(x, y, z), glm::vec3(0, 1, 0), glm::vec2(0, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, y, z), glm::vec3(0, 1, 0), glm::vec2(0, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, y, -z), glm::vec3(0, 1, 0), glm::vec2(1, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, y, -z), glm::vec3(0, 1, 0), glm::vec2(1, 0)));

  CubeVertices.push_back(
      Vertex(glm::vec3(-x, -y, -z), glm::vec3(0, 0, -1), glm::vec2(1, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, -y, -z), glm::vec3(0, 0, -1), glm::vec2(0, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, y, -z), glm::vec3(0, 0, -1), glm::vec2(0, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, y, -z), glm::vec3(0, 0, -1), glm::vec2(1, 1)));

  CubeVertices.push_back(
      Vertex(glm::vec3(-x, -y, -z), glm::vec3(-1, 0, 0), glm::vec2(0, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, y, -z), glm::vec3(-1, 0, 0), glm::vec2(0, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, y, z), glm::vec3(-1, 0, 0), glm::vec2(1, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, -y, z), glm::vec3(-1, 0, 0), glm::vec2(1, 0)));

  CubeVertices.push_back(
      Vertex(glm::vec3(-x, -y, -z), glm::vec3(0, -1, 0), glm::vec2(0, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(-x, -y, z), glm::vec3(0, -1, 0), glm::vec2(1, 0)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, -y, z), glm::vec3(0, -1, 0), glm::vec2(1, 1)));
  CubeVertices.push_back(
      Vertex(glm::vec3(x, -y, -z), glm::vec3(0, -1, 0), glm::vec2(0, 1)));

  for (int i = 0; i < CubeIndices.size(); i += 3) {
    Vertex &a = CubeVertices[CubeIndices[i]];
    Vertex &b = CubeVertices[CubeIndices[i + 1]];
    Vertex &c = CubeVertices[CubeIndices[i + 2]];

    glm::vec3 edge1 = b.pos - a.pos;
    glm::vec3 edge2 = c.pos - a.pos;
    glm::vec2 deltaUV1 = b.texC - a.texC;
    glm::vec2 deltaUV2 = c.texC - a.texC;

    GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    glm::vec3 tangent, bitangent;

    tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
    tangent = glm::normalize(tangent);

    bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
    bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
    bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
    bitangent = glm::normalize(bitangent);

    a.tangent = tangent;
    b.tangent = tangent;
    c.tangent = tangent;

    a.bitangent = bitangent;
    b.bitangent = bitangent;
    c.bitangent = bitangent;
  }

  glGenVertexArrays(1, &CubeVAO);
  glGenBuffers(1, &CubeVBO);
  glGenBuffers(1, &CubeEBO);

  glBindVertexArray(CubeVAO);

  glBindBuffer(GL_ARRAY_BUFFER, CubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * CubeVertices.size(),
               &CubeVertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CubeEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               sizeof(unsigned int) * CubeIndices.size(), &CubeIndices[0],
               GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, norm));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, texC));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, tangent));

  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, bitangent));

  glBindVertexArray(0);
}

void drawCube(Shader shader, GLuint diffuse, GLuint bump, GLuint spec) {

  glActiveTexture(GL_TEXTURE0);
  glUniform1i(glGetUniformLocation(shader.Program, "diffuse_map"), 0);
  glBindTexture(GL_TEXTURE_2D, diffuse);

  glActiveTexture(GL_TEXTURE1);
  glUniform1i(glGetUniformLocation(shader.Program, "bump_map"), 1);
  glBindTexture(GL_TEXTURE_2D, bump);

  glActiveTexture(GL_TEXTURE2);
  glUniform1i(glGetUniformLocation(shader.Program, "spec_map"), 2);
  glBindTexture(GL_TEXTURE_2D, spec);

  glBindVertexArray(CubeVAO);
  glDrawElements(GL_TRIANGLES, CubeIndices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glEnable(GL_TEXTURE0);
}

void drawCube(GLuint texture) {
  glEnable(GL_TEXTURE_2D);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  int i, j;
  const GLfloat x1 = -0.5, x2 = 0.5;
  const GLfloat y1 = -0.5, y2 = 0.5;
  const GLfloat z1 = -0.5, z2 = 0.5;

  GLfloat point[6][4][3] = {
      {{x1, y1, z1}, {x1, y2, z1}, {x2, y2, z1}, {x2, y1, z1}},
      {{x1, y1, z2}, {x2, y1, z2}, {x2, y2, z2}, {x1, y2, z2}},
      {{x1, y2, z1}, {x1, y2, z2}, {x2, y2, z2}, {x2, y2, z1}},
      {{x1, y1, z1}, {x2, y1, z1}, {x2, y1, z2}, {x1, y1, z2}},
      {{x2, y1, z1}, {x2, y2, z1}, {x2, y2, z2}, {x2, y1, z2}},
      {{x1, y1, z1}, {x1, y1, z2}, {x1, y2, z2}, {x1, y2, z1}},
  };

  int dir[6][4][2] = {
      {{0, 0}, {0, 1}, {1, 1}, {1, 0}}, {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
      {{0, 1}, {0, 0}, {1, 0}, {1, 1}}, {{1, 1}, {0, 1}, {0, 0}, {1, 0}},
      {{1, 0}, {1, 1}, {0, 1}, {0, 0}}, {{0, 0}, {1, 0}, {1, 1}, {0, 1}},
  };

  for (i = 0; i < 6; i++) {
    glm::vec3 v1(point[i][0][0], point[i][0][1], point[i][0][2]);
    glm::vec3 v2(point[i][1][0], point[i][1][1], point[i][1][2]);
    glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    for (j = 0; j < 4; j++) {
      glTexCoord2iv(dir[i][j]);
      glNormal3fv(glm::value_ptr(normal));
      glVertex3fv(point[i][j]);
    }
    glEnd();
  }

  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_TEXTURE_2D);
}

void drawSkybox(GLuint *texture) {

  glPushMatrix();
  glTranslatef(0.0f, SkyboxSize / 2.0f, 0.0f);
  glRotatef(270, 1, 0, 0);

  glScalef(SkyboxSize, SkyboxSize, 1);
  drawRect(texture[1]);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0f, -SkyboxSize / 2.0f, 0.0f);
  glRotatef(90, 1, 0, 0);

  glScalef(SkyboxSize, SkyboxSize, 1);
  drawRect(texture[2]);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-SkyboxSize / 2.0f, 0.0f, 0.0f);
  glRotatef(270, 0, 1, 0);
  glRotatef(180, 0, 0, 1);

  glScalef(SkyboxSize, SkyboxSize, 1);
  drawRect(texture[3]);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(SkyboxSize / 2.0f, 0.0f, 0.0f);
  glRotatef(90, 0, 1, 0);
  glRotatef(180, 0, 0, 1);

  glScalef(SkyboxSize, SkyboxSize, 1);
  drawRect(texture[4]);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0f, 0.0f, -SkyboxSize / 2.0);
  glRotatef(180, 1, 0, 0);

  glScalef(SkyboxSize, SkyboxSize, 1);
  drawRect(texture[5]);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0f, 0.0f, SkyboxSize / 2.0f);
  glRotatef(180, 0, 0, 1);

  glScalef(SkyboxSize, SkyboxSize, 1);
  drawRect(texture[6]);
  glPopMatrix();
}

void initSingleBoxCollider(glm::vec3 pos, glm::vec3 scalar) {
  boxPosition.push_back(pos);
  boxScale.push_back(scalar);
  boxSum++;
  isBreadEatenSet.push_back(false);
}

void initBoxCollidersProperty() {

  initSingleBoxCollider(glm::vec3(-60.f, -1.0f * roomSizeY / 2.0f + 2.5f, 60.f),
                        glm::vec3(5, 5, 40));

  initSingleBoxCollider(glm::vec3(-70.f, -1.0f * roomSizeY / 2.0f + 7.5f, 40.f),
                        glm::vec3(20, 5, 5));

  initSingleBoxCollider(glm::vec3(-80.f, -1.0f * roomSizeY / 2.0f + 12.5f, 0.f),
                        glm::vec3(5, 5, 80));

  initSingleBoxCollider(
      glm::vec3(-50.f, -1.0f * roomSizeY / 2.0f + 7.5f, -40.f),
      glm::vec3(60, 5, 5));

  initSingleBoxCollider(
      glm::vec3(-20.f, -1.0f * roomSizeY / 2.0f + 12.5f, -20.f),
      glm::vec3(5, 5, 40));

  initSingleBoxCollider(glm::vec3(-30.f, -1.0f * roomSizeY / 2.0f + 17.5f, 0.f),
                        glm::vec3(20, 5, 5));

  initSingleBoxCollider(
      glm::vec3(-40.f, -1.0f * roomSizeY / 2.0f + 22.5f, 10.f),
      glm::vec3(5, 5, 20));

  initSingleBoxCollider(
      glm::vec3(-10.f, -1.0f * roomSizeY / 2.0f + 17.5f, 20.f),
      glm::vec3(60, 5, 5));

  initSingleBoxCollider(
      glm::vec3(20.f, -1.0f * roomSizeY / 2.0f + 22.5f, -30.f),
      glm::vec3(5, 5, 100));

  initSingleBoxCollider(
      glm::vec3(-20.f, -1.0f * roomSizeY / 2.0f + 27.5f, -80.f),
      glm::vec3(80, 5, 5));

  initSingleBoxCollider(
      glm::vec3(50.f, -1.0f * roomSizeY / 2.0f + 17.5f, -80.f),
      glm::vec3(60, 5, 5));

  initSingleBoxCollider(
      glm::vec3(80.f, -1.0f * roomSizeY / 2.0f + 12.5f, -60.f),
      glm::vec3(5, 5, 40));

  initSingleBoxCollider(glm::vec3(60.f, -1.0f * roomSizeY / 2.0f + 7.5f, -40.f),
                        glm::vec3(40, 5, 5));

  initSingleBoxCollider(glm::vec3(40.f, -1.0f * roomSizeY / 2.0f + 12.5f, 10.f),
                        glm::vec3(5, 5, 100));

  initSingleBoxCollider(glm::vec3(20.f, -1.0f * roomSizeY / 2.0f + 17.5f, 60.f),
                        glm::vec3(40, 5, 5));

  initSingleBoxCollider(
      glm::vec3(-20.f, -1.0f * roomSizeY / 2.0f + 12.5f, 60.f),
      glm::vec3(40, 5, 5));

  initSingleBoxCollider(glm::vec3(0.f, -1.0f * roomSizeY / 2.0f + 22.5f, 70.f),
                        glm::vec3(5, 5, 20));

  initSingleBoxCollider(glm::vec3(40.f, -1.0f * roomSizeY / 2.0f + 27.5f, 80.f),
                        glm::vec3(80, 5, 5));

  initSingleBoxCollider(glm::vec3(80.f, -1.0f * roomSizeY / 2.0f + 32.5f, 40.f),
                        glm::vec3(5, 5, 80));
}

void setBoxColliderBoundary(FPSCamera *cam) {
  for (int i = 0; i < boxPosition.size(); i++) {
    cam->setSceneInnerBoundary(boxPosition[i].x - boxScale[i].x / 2.f,
                               boxPosition[i].y - boxScale[i].y / 2.f,
                               boxPosition[i].z - boxScale[i].z / 2.f,
                               boxPosition[i].x + boxScale[i].x / 2.f,
                               boxPosition[i].y + boxScale[i].y / 2.f,
                               boxPosition[i].z + boxScale[i].z / 2.f);
  }
}

void drawBoxColliders(Shader shader, GLuint diffuse, GLuint bump, GLuint spec,
                      FPSCamera *cam) {

  glUniform3fv(glGetUniformLocation(shader.Program, "lightPos1"), 1,
               LightPosition);

  glUniform3fv(glGetUniformLocation(shader.Program, "lightPos2"), 1,
               LightPosition2);

  glUniform3fv(glGetUniformLocation(shader.Program, "lightPos3"), 1,
               LightPosition3);

  glUniform3fv(glGetUniformLocation(shader.Program, "lightPos4"), 1,
               LightPosition4);

  glUniform3fv(glGetUniformLocation(shader.Program, "viewPos"), 1,
               glm::value_ptr(cam->cameraPos));

  float P[16];
  glGetFloatv(GL_PROJECTION_MATRIX, P);

  glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1,
                     GL_FALSE,

                     P);

  for (int i = 0; i < boxPosition.size(); i++) {

    glm::mat4 boxPos = glm::translate(
        glm::mat4(1.0),
        glm::vec3(boxPosition[i].x, boxPosition[i].y, boxPosition[i].z));
    glm::mat4 boxScl = glm::scale(
        glm::mat4(1.0), glm::vec3(boxScale[i].x, boxScale[i].y, boxScale[i].z));

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1,
                       GL_FALSE, glm::value_ptr(boxPos * boxScl));

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1,
                       GL_FALSE, glm::value_ptr(cam->viewMatrix));

    drawCube(shader, diffuse, bump, spec);
  }
}

void drawBoxColliders(GLuint *texture) {
  for (int i = 0; i < boxPosition.size(); i++) {
    glStencilMask(0x00);
    glPushMatrix();
    glTranslatef(boxPosition[i].x, boxPosition[i].y, boxPosition[i].z);
    glScalef(boxScale[i].x, boxScale[i].y, boxScale[i].z);
    drawCube(texture[0]);
    glPopMatrix();
  }
}

void initBreadModels() {
  int breadSum = boxSum;

  Model *myBreadModel = new Model;
  if (!myBreadModel->importModel("ModelRes/Bread/Bread.obj"))
    cout << "Import model error!" << endl;

  for (int i = 0; i < breadSum; i++) {
    breadSet.push_back(myBreadModel);
  }

  for (int i = 0; i < breadSum; i++) {
    ParticleSystem eaten(80, GravityAcceler);
    eaten.init();
    breadEatenEffectSet.push_back(eaten);
  }
}

void drawBreadModels() {
  for (int i = 0; i < breadSet.size(); i++) {
    if (!isBreadEatenSet[i]) {
      glPushMatrix();

      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilMask(0xFF);

      glTranslatef(boxPosition[i].x, boxPosition[i].y + 10.f, boxPosition[i].z);
      glRotatef(angle, 0.f, 1.f, 0.f);
      glScalef(5.f, 5.f, 5.f);
      breadSet[i]->renderTheModel(0.5f, false);
      glPopMatrix();
    }

    if (i == closeToBreadIndex) {
      glPushMatrix();

      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      glDisable(GL_DEPTH_TEST);

      glTranslatef(boxPosition[i].x, boxPosition[i].y + 9.92f,
                   boxPosition[i].z);
      glRotatef(angle, 0.f, 1.f, 0.f);
      glScalef(6.2f, 6.2f, 6.2f);
      breadSet[i]->renderTheModel(0.5f, true);

      glStencilMask(0xFF);
      glEnable(GL_DEPTH_TEST);

      glPopMatrix();
    }
  }
  angle += 1.5f;
}

void playBreadEatenEffect(FPSCamera *cam) {
  for (int i = 0; i < breadSet.size(); i++) {
    if (isBreadEatenSet[i]) {
      glPushMatrix();
      glm::vec3 pPos = boxPosition[i] + cam->getForward() * 10.f;
      glTranslatef(pPos.x, pPos.y + 7.f, pPos.z);
      breadEatenEffectSet[i].simulate(0.01);
      breadEatenEffectSet[i].render();
      glPopMatrix();
    }
  }
}

void deleteBreadModels() {
  for (int i = 0; i < breadSet.size(); i++)
    delete breadSet[i];
}

void detectBreadBeingEaten(FPSCamera *cam) {
  for (int i = 0; i < breadSet.size(); i++) {
    if (!isBreadEatenSet[i]) {
      glm::vec3 breadPos(boxPosition[i].x, boxPosition[i].y + 10.f,
                         boxPosition[i].z);

      if (cam->detectPlayerCloseToBread(breadPos, CloseToBreadDistance)) {
        closeToBreadIndex = i;

        if (cam->detectPlayerEatingBread(breadPos, EatBreadDistance)) {
          isBreadEatenSet[i] = true;
          eatenBreadNum++;
          closeToBreadIndex = -1;
          break;
        }
        break;
      } else
        closeToBreadIndex = -1;
    }
  }
}

float redAmbient[4] = {1, 0, 0, 1};
float redDiffuse[4] = {1, 0, 0, 1};
float redSpecular[4] = {1, 0, 0, 1};
float redEmission[4] = {1, 0, 0, 1};

void applyRedMaterial() {
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, redAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, redDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, redSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, redEmission);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
}

float whiteAmbient[4] = {1, 1, 1, 1};
float whiteDiffuse[4] = {1, 1, 1, 1};
float whiteSpecular[4] = {1, 1, 1, 1};
float whiteEmission[4] = {1, 1, 1, 1};

void applyWhiteMaterial() {
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, whiteAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, whiteDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, whiteSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, whiteEmission);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
}

float blackAmbient[4] = {0, 0, 0, 1};
float blackDiffuse[4] = {0, 0, 0, 1};
float blackSpecular[4] = {0, 0, 0, 1};
float blackEmission[4] = {0, 0, 0, 1};

void applyBlackMaterial() {
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, blackAmbient);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, blackDiffuse);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blackSpecular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, blackEmission);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0);
}

#define FONT_ComicSansMS "Comic Sans MS"
#define FONT_KaiTi "¿¬Ìå"
#define FONT_FangSong "·ÂËÎ"

string GameTitle = "DOOM";
string GameRule = "Game Rule: ";
string GameRuleCtrl = "Control:\n   1. Press 'w/a/s/d' to move.\n   2. Press "
                      "'space' to jump.\n   3. Use 'mouse' to look around.";
string GameRuleTarget = "Target:\n    Collect more totems and slay doom!";
string GameStartTitle = "Start";
string GameMaker = "Made by bhavya and samay";
string GameVictory = "You Won!!";

void selectFont(int size, int charset, const char *face) {
  HFONT hFont = CreateFontA(size, 0, 0, 0, FW_MEDIUM, 0, 0, 0, charset,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, face);
  HFONT hOldFont = (HFONT)SelectObject(wglGetCurrentDC(), hFont);
  DeleteObject(hOldFont);
}

vector<GLuint> listOfTexts;
void initEnString(const char *str) {}

#define MAX_CHAR 128
void drawEnString(const char *str) {
  static int isFirstCall = 1;
  GLuint lists;

  lists = glGenLists(MAX_CHAR);
  wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);

  for (; *str != '\0'; ++str)
    glCallList(lists + *str);

  glDeleteLists(lists, MAX_CHAR);
}

void drawCNString(const char *str) {
  int len, i;
  wchar_t *wstring;
  HDC hDC = wglGetCurrentDC();
  GLuint list = glGenLists(1);

  len = 0;
  for (i = 0; str[i] != '\0'; ++i) {
    if (IsDBCSLeadByte(str[i]))
      ++i;
    ++len;
  }

  wstring = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
  MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, str, -1, wstring, len);
  wstring[len] = L'\0';

  for (i = 0; i < len; ++i) {
    wglUseFontBitmapsW(hDC, wstring[i], 1, list);
    glCallList(list);
  }

  free(wstring);
  glDeleteLists(list, 1);
}

void drawMenuSceneUIText(FPSCamera *cam) {
  char strBuffer[200];

  glPushMatrix();
  applyWhiteMaterial();
  glBegin(GL_POLYGON);
  glVertex2f(cam->cameraPos.x - 18.f, cam->cameraPos.y + 25.f);
  glVertex2f(cam->cameraPos.x - 18.f, cam->cameraPos.y + 31.f);
  glVertex2f(cam->cameraPos.x + 18.f, cam->cameraPos.y + 31.f);
  glVertex2f(cam->cameraPos.x + 18.f, cam->cameraPos.y + 25.f);
  glEnd();
  glPopMatrix();

  selectFont(40, ANSI_CHARSET, FONT_ComicSansMS);

  glPushMatrix();
  applyBlackMaterial();
  glRasterPos3f(cam->cameraPos.x - 0.86f, cam->cameraPos.y + 1.48f,
                cam->cameraPos.z - 5.f);
  drawEnString(GameTitle.c_str());
  glPopMatrix();

  applyWhiteMaterial();

  selectFont(36, ANSI_CHARSET, FONT_ComicSansMS);

  glPushMatrix();
  glRasterPos3f(cam->cameraPos.x - 1.2f, cam->cameraPos.y + 0.8f,
                cam->cameraPos.z - 5.f);
  drawEnString(GameRule.c_str());
  glPopMatrix();

  glPushMatrix();
  glRasterPos3f(cam->cameraPos.x - 1.f, cam->cameraPos.y + 0.5f,
                cam->cameraPos.z - 5.f);
  const char *GameRuleCtrlc = GameRuleCtrl.c_str();
  sprintf(strBuffer, "%s", GameRuleCtrlc);
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)strBuffer);
  glPopMatrix();

  glPushMatrix();
  glRasterPos3f(cam->cameraPos.x - 1.f, cam->cameraPos.y - 0.3f,
                cam->cameraPos.z - 5.f);
  const char *GameRuleTargetc = GameRuleTarget.c_str();
  sprintf(strBuffer, "%s", GameRuleTargetc);
  glutBitmapString(GLUT_BITMAP_HELVETICA_18, (const unsigned char *)strBuffer);
  glPopMatrix();

  glPushMatrix();
  applyWhiteMaterial();
  glBegin(GL_POLYGON);
  glVertex2f(cam->cameraPos.x - 7.f, cam->cameraPos.y - 18.f);
  glVertex2f(cam->cameraPos.x - 7.f, cam->cameraPos.y - 24.f);
  glVertex2f(cam->cameraPos.x + 7.f, cam->cameraPos.y - 24.f);
  glVertex2f(cam->cameraPos.x + 7.f, cam->cameraPos.y - 18.f);
  glEnd();
  glPopMatrix();

  selectFont(40, ANSI_CHARSET, FONT_ComicSansMS);

  glPushMatrix();
  applyBlackMaterial();
  glRasterPos3f(cam->cameraPos.x - 0.25f, cam->cameraPos.y - 1.24f,
                cam->cameraPos.z - 5.f);
  drawEnString(GameStartTitle.c_str());
  glPopMatrix();

  glPushMatrix();
  selectFont(24, GB2312_CHARSET, FONT_KaiTi);

  applyWhiteMaterial();
  glRasterPos3f(cam->cameraPos.x + 0.5f, cam->cameraPos.y - 1.7f,
                cam->cameraPos.z - 5.f);
  drawCNString(GameMaker.c_str());
  glPopMatrix();
}

#define VictoryTextSizeLow 60
#define VictoryTextSizeUp 200
#define VictoryTextSizeGap 6

string GameSceneUIText = "Totems: ";
static int victoryTextSize = VictoryTextSizeLow;
int dSize = VictoryTextSizeGap;

#include <glm/gtx/string_cast.hpp>
bool once = false;
void debugOnce(bool &once, glm::mat3 d) {
  if (!once) {
    cout << glm::to_string(d) << endl;
    once = true;
  }
}
void drawGameSceneUIText(FPSCamera *cam, int x, int y) {

  glm::mat3 vpMatI(300, 0, 300, 0, 300, 300, 0, 0, 1);

  vpMatI = glm::inverse(glm::transpose(vpMatI));
  glm::vec3 world = (vpMatI)*glm::vec3(x, y, 1);
  debugOnce(once, vpMatI);

  drawEnString("tt");

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-1, 1, -1, 1, 0, 2);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  selectFont(36, ANSI_CHARSET, FONT_ComicSansMS);
  applyBlackMaterial();
  glRasterPos3f(world[0], world[1], 0);

  char strBuffer[80];
  const char *UIText1c = GameSceneUIText.c_str();
  string UIText2 = " / ";
  const char *UIText2c = UIText2.c_str();
  sprintf(strBuffer, "%s%d%s%d", UIText1c, eatenBreadNum, UIText2c, boxSum);

  drawEnString(strBuffer);

  if (eatenBreadNum == boxSum) {
    victoryTextSize += dSize;
    selectFont(victoryTextSize, ANSI_CHARSET, FONT_ComicSansMS);
    glPushMatrix();
    applyRedMaterial();

    glRasterPos3f(-0.5, 0, 0);
    drawEnString(GameVictory.c_str());
    glPopMatrix();

    if (victoryTextSize >= VictoryTextSizeUp)
      dSize = -VictoryTextSizeGap;
    else if (victoryTextSize <= VictoryTextSizeLow)
      dSize = VictoryTextSizeGap;
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void setupLights() {
  glClearDepth(1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  glEnable(GL_LIGHTING);

  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable(GL_LIGHT1);
  glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);

  glEnable(GL_LIGHT2);
  glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT2, GL_POSITION, LightPosition2);

  glEnable(GL_LIGHT3);
  glLightfv(GL_LIGHT3, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT3, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT3, GL_POSITION, LightPosition3);

  glEnable(GL_LIGHT4);
  glLightfv(GL_LIGHT4, GL_AMBIENT, LightAmbient);
  glLightfv(GL_LIGHT4, GL_DIFFUSE, LightDiffuse);
  glLightfv(GL_LIGHT4, GL_POSITION, LightPosition4);
}
