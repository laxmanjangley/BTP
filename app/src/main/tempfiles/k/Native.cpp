#include <cstdio>
#include <cstdlib>
// #include "PCUtils.h"
#include "Draw.h"
// #include <GL/glew.h>
#include <string>
#include <cmath>

// using namespace std;

// int main(){
// 	setupWindowAndGLContext();
// 	setupGraphics(1366, 768);
// 	while(1){
// 		renderFrame();
// 		SwapBuffer();
// 		if(PollForESC()){
// 			cleanup();
// 			break;
// 		}
// 	}
// 	return 0;
// }

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