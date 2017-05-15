#include <cstdio>
#include <cstdlib>
#include <string>
#include <cmath>

#include "Draw.h"
#ifdef PCIMPL
#include "PCUtils.h"
#endif

using namespace std;

#ifdef PCIMPL

int main(){
    SetupWindowAndGLContext();
    setupGraphics(1366, 768);
    while(1){
        renderFrame();
        SwapBuffer();
        if(PollForESC()){
            cleanup();
            break;
        }
    }
    return 0;
}

#else

extern "C"
{
    JNIEXPORT void JNICALL Java_com_arm_malideveloper_openglessdk_metaballs_NativeLibrary_init
    (JNIEnv *env, jclass jcls, jint width, jint height)
    {
        /* Initialze OpenGL ES and model environment for metaballs: allocate buffers and textures, bind them, etc. */
        setupGraphics(width, height);
    }

    JNIEXPORT void JNICALL Java_com_arm_malideveloper_openglessdk_metaballs_NativeLibrary_step
    (JNIEnv *env, jclass jcls)
    {
        /* Render a frame */
        renderFrame();
    }

    JNIEXPORT void JNICALL Java_com_arm_malideveloper_openglessdk_metaballs_NativeLibrary_uninit
    (JNIEnv *, jclass)
    {
        cleanup();
    }
}
#endif