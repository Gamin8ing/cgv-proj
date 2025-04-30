#include "Model.h"

string getBasePath(const string &path) {
  size_t pos = path.find_last_of("\\/");
  return (string::npos == pos) ? "" : path.substr(0, pos + 1);
}

Model::Model() {}

Model::~Model() {

  textureIdMap.clear();

  if (textureIds) {
    delete[] textureIds;
    textureIds = NULL;
  }

  aiReleaseImport(scene);
}

bool Model::importModel(const string &pFile) {

  ifstream fin(pFile.c_str());
  if (!fin.fail())
    fin.close();
  else
    return false;

  scene =
      aiImportFile(pFile.c_str(), aiProcessPreset_TargetRealtime_MaxQuality);

  if (!scene)
    return false;
  else {
    cout << "Import successfully!" << endl;
    if (!loadTextures(scene, pFile))
      return false;
  }
  return true;
}

bool Model::loadTextures(const aiScene *scene, const string modelPath) {
  ILboolean success;

  if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION) {

    string err_msg = "Wrong DevIL version. Old devil.dll in system32/SysWow64?";
    cout << err_msg << endl;
    return false;
  }

  ilInit();

  for (unsigned int m = 0; m < scene->mNumMaterials; m++) {
    int texIndex = 0;
    aiReturn texFound = AI_SUCCESS;

    aiString path;

    while (texFound == AI_SUCCESS) {
      texFound = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE,
                                                  texIndex, &path);
      textureIdMap[path.data] = NULL;
      texIndex++;
    }
  }

  int numTextures = textureIdMap.size();

  ILuint *imageIds = NULL;
  imageIds = new ILuint[numTextures];

  ilGenImages(numTextures, imageIds);

  textureIds = new GLuint[numTextures];
  for (int i = 0; i < numTextures; i++) {
    textureIds[i] = 20 + i;
  }

  map<string, GLuint *>::iterator itr = textureIdMap.begin();

  string basepath = getBasePath(modelPath);

  for (int i = 0; i < numTextures; i++) {

    string filename = (*itr).first;
    cout << "filename " << filename << endl;
    (*itr).second = &textureIds[i];
    itr++;

    ilBindImage(imageIds[i]);
    string fileloc = basepath + filename;
    success = ilLoadImage(fileloc.c_str());

    if (success) {

      success = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
      if (!success) {

        return false;
      }

      glBindTexture(GL_TEXTURE_2D, textureIds[i]);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

      glTexImage2D(GL_TEXTURE_2D, 0, ilGetInteger(IL_IMAGE_BPP),
                   ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
                   0, ilGetInteger(IL_IMAGE_FORMAT), GL_UNSIGNED_BYTE,
                   ilGetData());

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
      glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
      glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    }
  }

  ilDeleteImages(numTextures, imageIds);

  delete[] imageIds;
  imageIds = NULL;

  return true;
}

void Model::renderTheModel(float scale = 1.0f, bool isAmbient = true) {
  glEnable(GL_TEXTURE_2D);
  recursiveRender(scene, scene->mRootNode, scale, isAmbient);
  glDisable(GL_TEXTURE_2D);
}
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode *nd,
                            float scale = 1.0f, bool isAmbient = true) {
  aiMatrix4x4 mTrans = nd->mTransformation;
  aiMatrix4x4 m2;
  aiMatrix4x4::Scaling(aiVector3D(scale, scale, scale), m2);
  mTrans = mTrans * m2;

  mTrans.Transpose();
  glPushMatrix();
  glMultMatrixf((float *)&mTrans);

  for (int m = 0; m < nd->mNumMeshes; m++) {
    const struct aiMesh *mesh = scene->mMeshes[nd->mMeshes[m]];

    applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);

    if (mesh->mNormals == NULL)
      glDisable(GL_LIGHTING);
    else
      glEnable(GL_LIGHTING);

    for (int f = 0; f < mesh->mNumFaces; f++) {
      const struct aiFace *face = &(mesh->mFaces[f]);

      GLenum face_mode;

      switch (face->mNumIndices) {
      case 1:
        face_mode = GL_POINTS;
        break;
      case 2:
        face_mode = GL_LINES;
        break;
      case 3:
        face_mode = GL_TRIANGLES;
        break;
      default:
        face_mode = GL_POLYGON;
        break;
      }

      glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

      const GLfloat edgeColor[] = {0.f, 1.0f, 1.f, 1.0f};
      const GLfloat white[] = {1.0f, 1.0f, 1.0f, 1.0f};
      const GLfloat black[] = {0.0f, 0.0f, 0.0f, 1.0f};

      if (isAmbient) {
        glColor4fv(edgeColor);
        glEnable(GL_COLOR_MATERIAL);
      }

      glBegin(face_mode);

      int i;

      for (i = 0; i < face->mNumIndices; i++) {
        int index = face->mIndices[i];

        if (mesh->mNormals != NULL && !isAmbient) {
          if (mesh->HasTextureCoords(0)) {
            glTexCoord2f(mesh->mTextureCoords[0][index].x,
                         1 - mesh->mTextureCoords[0][index].y);
          }
          glNormal3fv(&mesh->mNormals[index].x);
        }
        glVertex3fv(&(mesh->mVertices[index].x));
      }
      glEnd();

      if (isAmbient)
        glDisable(GL_COLOR_MATERIAL);
    }
  }

  for (int n = 0; n < nd->mNumChildren; ++n) {
    recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
  }

  glPopMatrix();
}

void color4_to_float4(const aiColor4D *c, float f[4]) {
  f[0] = c->r;
  f[1] = c->g;
  f[2] = c->b;
  f[3] = c->a;
}

void set_float4(float f[4], float a, float b, float c, float d) {
  f[0] = a;
  f[1] = b;
  f[2] = c;
  f[3] = d;
}
void Model::applyMaterial(const aiMaterial *mtl, bool isAmbient) {
  float c[4];

  GLenum fill_mode;
  int ret1, ret2;
  aiColor4D diffuse;
  aiColor4D specular;
  aiColor4D ambient;
  aiColor4D emission;
  float shininess, strength;
  int two_sided;
  int wireframe;
  unsigned int max;

  int texIndex = 0;
  aiString texPath;

  if (AI_SUCCESS ==
      mtl->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath)) {

    unsigned int texId = *(textureIdMap[texPath.data]);
    if (isAmbient)
      glBindTexture(GL_TEXTURE_2D, 0);
    else
      glBindTexture(GL_TEXTURE_2D, texId);
  }

  set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
  if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
    color4_to_float4(&diffuse, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

  set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
  if (AI_SUCCESS ==
      aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular))
    color4_to_float4(&specular, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

  set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
  if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient))
    color4_to_float4(&ambient, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

  set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
  if (AI_SUCCESS ==
      aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission))
    color4_to_float4(&emission, c);
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

  max = 1;
  ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
  if (ret1 == AI_SUCCESS) {
    max = 1;
    ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength,
                                   &max);
    if (ret2 == AI_SUCCESS)
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
    else
      glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  } else {
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  max = 1;
  if ((AI_SUCCESS ==
       aiGetMaterialIntegerArray(mtl, AI_MATKEY_TWOSIDED, &two_sided, &max)) &&
      two_sided)
    glDisable(GL_CULL_FACE);
  else
    glEnable(GL_CULL_FACE);
}

/*
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
void Model::recursiveRender(const struct aiScene *sc, const struct aiNode* nd,
float scale = 1.0f, bool isAmbient = true) { aiMatrix4x4 mTrans =
nd->mTransformation; aiMatrix4x4 m2; aiMatrix4x4::Scaling(aiVector3D(scale,
scale, scale), m2); mTrans = mTrans * m2;


        mTrans.Transpose();
        glPushMatrix();
        glMultMatrixf((float*)&mTrans);


        for (int m = 0; m < nd->mNumMeshes; m++) {
                const struct aiMesh* mesh = scene->mMeshes[nd->mMeshes[m]];


                applyMaterial(sc->mMaterials[mesh->mMaterialIndex], isAmbient);


                if (mesh->mNormals == NULL)
                        glDisable(GL_LIGHTING);
                else
                        glEnable(GL_LIGHTING);


                for (int f = 0; f < mesh->mNumFaces; f++) {
                        const struct aiFace* face = &(mesh->mFaces[f]);

                        GLenum face_mode;

                        switch (face->mNumIndices) {
                        case 1:
                                face_mode = GL_POINTS;
                                break;
                        case 2:
                                face_mode = GL_LINES;
                                break;
                        case 3:
                                face_mode = GL_TRIANGLES;
                                break;
                        default:
                                face_mode = GL_POLYGON;
                                break;
                        }

                        glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

                        const GLfloat edgeColor[] = { 0.f, 1.0f, 1.f, 1.0f };
                        const GLfloat white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
                        const GLfloat black[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                        if (isAmbient) {
                                glColor4fv(edgeColor);
                                glEnable(GL_COLOR_MATERIAL);
                        }

                        glBegin(face_mode);

                        int i;

                        for (i = 0; i < face->mNumIndices; i++) {
                                int index = face->mIndices[i];


                                if (mesh->mNormals != NULL && !isAmbient) {
                                        if (mesh->HasTextureCoords(0)) {
                                                glTexCoord2f(mesh->mTextureCoords[0][index].x,
                                                        1 -
mesh->mTextureCoords[0][index].y);
                                        }
                                        glNormal3fv(&mesh->mNormals[index].x);
                                }
                                glVertex3fv(&(mesh->mVertices[index].x));
                        }
                        glEnd();

                        if (isAmbient)
                                glDisable(GL_COLOR_MATERIAL);
                }
        }


        for (int n = 0; n < nd->mNumChildren; ++n) {
                recursiveRender(sc, nd->mChildren[n], scale, isAmbient);
        }

        glPopMatrix();
}*/
