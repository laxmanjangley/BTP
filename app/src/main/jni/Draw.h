#ifndef DRAW_H
#define DRAW_H

#ifdef PCIMPL
#define LOGI(...) fprintf(std,__VA_ARGS__)
#define LOGE(...) fprintf(stderr,__VA_ARGS__)
#else
#define LOG_TAG "libNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#include "ShaderCode.h"

using std::string;
using namespace MaliSDK;

void calc_mvp(Matrix& mvp);
void setupGraphics(int width, int height);
void renderFrame(void);
void cleanup(void);

#endif  /* DRAW_H */