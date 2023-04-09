//
// Created by intel on 29/3/2023.
//

// The MIT License (MIT)
//
// Copyright (c) 2013 Dan Ginsburg, Budirijanto Purnomo
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

//
// Book:      OpenGL(R) ES 3.0 Programming Guide, 2nd Edition
// Authors:   Dan Ginsburg, Budirijanto Purnomo, Dave Shreiner, Aaftab Munshi
// ISBN-10:   0-321-93388-5
// ISBN-13:   978-0-321-93388-1
// Publisher: Addison-Wesley Professional
// URLs:      http://www.opengles-book.com
//            http://my.safaribooksonline.com/book/animation-and-3d/9780133440133
//
// MultiTexture.c
//
//    This is an example that draws a quad with a basemap and
//    lightmap to demonstrate multitexturing.
//
#include <stdlib.h>
#include "esUtil.h"
#include "outBuffer.h"
#include "socket.h"
#include <pthread.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Sampler locations
    GLint baseMapLoc;
    GLint lightMapLoc;

    // Texture handle
    GLuint baseMapTexId;
    GLuint lightMapTexId;

    //GLTexture


} UserData;

///
// Load texture from disk
//
GLuint LoadTexture(void *ioContext, char *fileName) {
    int width,
            height;

    char *buffer = esLoadTGA(ioContext, fileName, &width, &height);
    GLuint texId;

    if (buffer == NULL) {
        esLogMessage("Error loading (%s) image.\n", fileName);
        return 0;
    }

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(buffer);

    return texId;
}


///
// Initialize the shader and program object
//
int Init(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;
#if 0
    char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "layout(location = 1) in vec2 a_texCoord;   \n"
            "out vec2 v_texCoord;                       \n"
            "void main()                                \n"
            "{                                          \n"
            "   gl_Position = a_position;               \n"
            "   v_texCoord = a_texCoord;                \n"
            "}                                          \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
            "precision mediump float;                            \n"
            "in vec2 v_texCoord;                                 \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "uniform sampler2D s_baseMap;                        \n"
            "uniform sampler2D s_lightMap;                       \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  vec4 baseColor;                                   \n"
            "  vec4 lightColor;                                  \n"
            "                                                    \n"
            "  baseColor = texture( s_baseMap, v_texCoord );     \n"
            "  lightColor = texture( s_lightMap, v_texCoord );   \n"
            "  outColor = baseColor * (lightColor + 0.25);       \n"
            "}                                                   \n";
#else
    char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "layout(location = 1) in vec2 a_texCoord;   \n"
            "out vec2 v_texCoord;                       \n"
            "void main()                                \n"
            "{                                          \n"
            "   gl_Position = a_position;               \n"
            "   v_texCoord = a_texCoord;                \n"
            "}                                          \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
            "#extension GL_OES_EGL_image_external : require      \n"
            "precision mediump float;                            \n"
            "layout(location = 0) out vec4 outColor;             \n"
            "in vec2 v_texCoord;                                 \n"
            "uniform samplerExternalOES uTexture;                \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  outColor = texture2D(uTexture, v_texCoord);   \n"
            "}                                                   \n";
#endif
    // Load the shaders and get a linked program object
    userData->programObject = esLoadProgram(vShaderStr, fShaderStr);

    // Get the sampler location
    //userData->baseMapLoc = glGetUniformLocation ( userData->programObject, "s_baseMap" );
    //userData->lightMapLoc = glGetUniformLocation ( userData->programObject, "s_lightMap" );

    // Load the textures
    //userData->baseMapTexId = LoadTexture ( esContext->platformData, "basemap.tga" );
    //userData->lightMapTexId = LoadTexture ( esContext->platformData, "lightmap.tga" );
    setupAHardwareBuffer01(esContext);
    userData->baseMapTexId = LoadOutTexture(esContext);

    int size = 3;
    esContext->eglDmaImage_size = size;
    memset(esContext->eglDmaImage, 0, sizeof(struct EGL_DMA_Image) * size);
    for (int i = 0; i < size; i++) {
        setupAHardwareBuffer02(esContext, i);
    }

    if (userData->baseMapTexId == 0) {
        return FALSE;
    } else {
        pthread_t server_thread;
        pthread_create(&server_thread, NULL, reinterpret_cast<void *(*)(void *)>(setupServer),
                       (void *) esContext);
    }

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return TRUE;
}

void Draw02(ESContext *esContext) {

    UserData *userData = (UserData *) esContext->userData;
    GLfloat vVertices[] = {-0.5f, 0.5f, 0.0f,  // Position 0
                           0.0f, 0.0f,        // TexCoord 0
                           -0.5f, -0.5f, 0.0f,  // Position 1
                           0.0f, 1.0f,        // TexCoord 1
                           0.5f, -0.5f, 0.0f,  // Position 2
                           1.0f, 1.0f,        // TexCoord 2
                           0.5f, 0.5f, 0.0f,  // Position 3
                           1.0f, 0.0f         // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind the base map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, userData->baseMapTexId);

    // Set the base map sampler to texture unit to 0
    glUniform1i(userData->baseMapLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}


void ShutDown02(ESContext *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    // Delete texture object
    glDeleteTextures(1, &userData->baseMapTexId);

    // Delete program object
    glDeleteProgram(userData->programObject);
}

extern "C"
int esMain(ESContext *esContext) {
    esContext->userData = malloc(sizeof(UserData));

    esCreateWindow(esContext, "MultiTexture", 320, 240, ES_WINDOW_RGB);

    if (!Init(esContext)) {
        return GL_FALSE;
    }

    esRegisterDrawFunc(esContext, Draw02);
    esRegisterShutdownFunc(esContext, ShutDown02);

    return GL_TRUE;
}