/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2014 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef SHADER_H
#define SHADER_H

#include <GLES3/gl3.h>
#include <cstdio>
#include <cstdlib>
#include <android/log.h>

#define GL_CHECK(x) \
    x; \
    { \
        GLenum glError = glGetError(); \
        if(glError != GL_NO_ERROR) { \
            LOGE("glGetError() = %i (0x%.8x) at %s:%i\n", glError, glError, __FILE__, __LINE__); \
            exit(1); \
        } \
    }

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
