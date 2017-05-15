#ifndef DRAW_H
#define DRAW_H


#include <cstdio>
#include <cstdlib>

#include "Shader.h"
#include "Timer.h"
#include "Matrix.h"
#include "ShaderCode.h"
#include "TriangulationTable.h"

#include <jni.h>
#include <android/log.h>
#include "GLES3/gl3.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

// #include <GL/glew.h>

#include <string>
#include <cmath>
//**********************
#include <iostream>
//**********************
#define LOG_TAG "libNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// #define LOGI(...) printf("%s\n",__VA_ARGS__)
// #define LOGE(...) printf("%s\n",__VA_ARGS__)

using std::string;
using namespace MaliSDK;

void calc_mvp(Matrix& mvp);
void setupGraphics(int width, int height);
void renderFrame(void);
void cleanup();

#endif  /* DRAW_H */