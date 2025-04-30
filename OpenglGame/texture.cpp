#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <windows.h>
#include "SceneController.h"
#include <soil\SOIL.h>
#define BITMAP_ID 0x4D42

static unsigned char *LoadBitmapFile(char *filename,
                                     BITMAPINFOHEADER *bitmapInfoHeader) {

  FILE *filePtr;
  BITMAPFILEHEADER bitmapFileHeader;
  unsigned char *bitmapImage;
  int imageIdx = 0;
  unsigned char tempRGB;

  filePtr = fopen(filename, "rb");
  if (filePtr == NULL) {
    printf("file not open\n");
    return NULL;
  }

  fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);

  if (bitmapFileHeader.bfType != BITMAP_ID) {
    fprintf(stderr, "Error in LoadBitmapFile: the file is not a bitmap file\n");
    return NULL;
  }

  fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);

  fseek(filePtr, bitmapFileHeader.bfOffBits, SEEK_SET);

  bitmapImage = new unsigned char[bitmapInfoHeader->biSizeImage];

  if (!bitmapImage) {
    fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
    return NULL;
  }

  fread(bitmapImage, 1, bitmapInfoHeader->biSizeImage, filePtr);

  if (bitmapImage == NULL) {
    fprintf(stderr, "Error in LoadBitmapFile: memory error\n");
    return NULL;
  }

  for (imageIdx = 0; imageIdx < bitmapInfoHeader->biSizeImage; imageIdx += 3) {
    tempRGB = bitmapImage[imageIdx];
    bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
    bitmapImage[imageIdx + 2] = tempRGB;
  }

  fclose(filePtr);

  return bitmapImage;
}

void loadTex(int i, char *filename, GLuint *texture) {

  unsigned char *bitmapData;

  int width, height;
  bitmapData = SOIL_load_image(filename, &width, &height, 0, SOIL_LOAD_RGB);
  glBindTexture(GL_TEXTURE_2D, texture[i]);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, bitmapData);

  SOIL_free_image_data(bitmapData);

  glBindTexture(GL_TEXTURE_2D, 0);
}
