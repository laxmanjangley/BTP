#ifndef SHADER_H
#define SHADER_H

#include <cstdio>
#include <cstdlib>

#ifdef PCIMPL
#include <GL/glew.h>
#define LOGE(...) fprintf(stderr,__VA_ARGS__)
#define GL_CHECK(x) \
    x; \
    { \
        GLenum glError = glGetError(); \
        if(glError != GL_NO_ERROR && glError != GL_INVALID_ENUM) { \
            LOGE("glGetError() = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__); \
            exit(1); \
        } \
    }
#else
#define LOG_TAG "libNative"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define GL_CHECK(x) \
    x; \
    { \
        GLenum glError = glGetError(); \
        if(glError != GL_NO_ERROR) { \
            LOGE("glGetError() = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__); \
            exit(1); \
        } \
    }
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>
#endif

namespace MaliSDK
{
    /**
     * \brief Functions for working with OpenGL ES shaders.
     */
    class Shader
    {
    public:
        /**
         * \brief Create shader, load in source, compile, and dump debug if necessary.
         *
         * Creates a shader using with the required shaderType using glCreateShader(shaderType) and then compiles it using glCompileShader.
         * The output from the compilation is checked for success and a log of the compilation errors is printed in the case of failure.
         *
         * \param[out] shaderPtr      The shader ID of the newly compiled shader. Cannot be NULL.
         * \param[in] shaderSourcePtr Contains OpenGL ES SL source code. Cannot be NULL.
         * \param[in] shaderType      Passed to glCreateShader to define the type of shader being processed.
         *                            Must be GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
         */
        static void processShader(GLuint* shaderPtr, const char* shaderSourcePtr, GLint shaderType);
    };
}
#endif /* SHADER_H */
