//
// Created by intel on 10/4/2023.
//

#include <string.h>
#include "offscreenRender.h"
#include "esUtil.h"
#include "android_log.h"

const char vShadowMapShaderStr[] =
        "#version 300 es                                  \n"
        "uniform mat4 u_mvpLightMatrix;                   \n"
        "layout(location = 0) in vec4 a_position;         \n"
        "out vec4 v_color;                                \n"
        "void main()                                      \n"
        "{                                                \n"
        "   gl_Position = u_mvpLightMatrix * a_position;  \n"
        "}                                                \n";

const char fShadowMapShaderStr[] =
        "#version 300 es                                  \n"
        "precision lowp float;                            \n"
        "void main()                                      \n"
        "{                                                \n"
        "}                                                \n";


const char vDirectionallightsStr[] =
        "#version 300 es\n"
        "\n"
        "precision mediump float;\n"
        "\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "layout (location = 2) in vec3 aNormal;\n"
        "\n"
        "out vec3 fragPos;\n"
        "out vec2 texCoord;\n"
        "out vec3 normal;\n"
        "\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    texCoord = aTexCoord;\n"
        "    fragPos = vec3(model * vec4(aPos, 1.0));\n"
        "    normal = mat3(transpose(inverse(model))) * aNormal;\n"
        "    gl_Position = projection * view * vec4(fragPos, 1.0);\n"
        "}";

const char fDirectionallightsStr[] =
        "#version 300 es\n"
        "\n"
        "precision mediump float;\n"
        "\n"
        "layout(location = 0) out vec4 fragColor;\n"
        "\n"
        "in vec2 texCoord;\n"
        "in vec3 fragPos;\n"
        "in vec3 normal;\n"
        "\n"
        "uniform sampler2D imageTex;\n"
        "uniform sampler2D normalTex;\n"
        "uniform vec3 lightDirection;\n"
        "uniform vec3 viewPos;\n"
        "uniform vec3 lightColor;\n"
        "uniform vec3 objectColor;\n"
        "\n"
        "float myPow(float x, int r) {\n"
        "    float result = 1.0;\n"
        "    for (int i = 0; i < r; i = i + 1) {\n"
        "        result = result * x;\n"
        "    }\n"
        "    return result;\n"
        "}\n"
        "\n"
        "void main()\n"
        "{\n"
        "\n"
        "    vec3 texColor = texture(imageTex, texCoord).rgb;\n"
        "\n"
        "    // ambient\n"
        "    float ambientStrength = 0.5;\n"
        "    vec3 ambient = ambientStrength * lightColor;\n"
        "\n"
        "    // diffuse\n"
        "    vec3 norm = normalize(normal);\n"
        "    vec3 lightDir = normalize(-lightDirection);\n"
        "    float diff = max(dot(norm, lightDir), 0.0);\n"
        "    vec3 diffuse = diff * lightColor;\n"
        "\n"
        "    // specular\n"
        "    float specularStrength = 2.0;\n"
        "    vec3 viewDir = normalize(viewPos - fragPos);\n"
        "    vec3 reflectDir = reflect(-lightDir, norm);\n"
        "    float spec = myPow(max(dot(viewDir, reflectDir), 0.0), 16);\n"
        "    vec3 specular = specularStrength * spec * lightColor;\n"
        "\n"
        "    vec3 result = (ambient + diffuse + specular) * texColor;\n"
        "    fragColor = vec4(result, 1.0);\n"
        "}";

int glSurfaceViewWidth = 0;
int glSurfaceViewHeight = 0;

float vertexData[] = {
        // 正面
        // front face
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        // 背面
        // back face
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        // 顶面
        // Top face
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        // 底面
        // Bottom face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        // 左面
        // Left face
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        // 右面
        // Right face
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f
};

const int VERTEX_COMPONENT_COUNT = 3;
float vertexDataBuffer[108] = {};

float normalData[] = {
        // 正面
        // front face
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        // 背面
        // back face
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        // 顶面
        // Top face
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        // 底面
        // Bottom face
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        // 左面
        // Left face
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        // 右面
        // Right face
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f
};

const int NORMAL_COMPONENT_COUNT = 3;
float normalDataBuffer[108] = {};

float textureCoordinateData[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
};
const int TEXTURE_COORDINATE_COMPONENT_COUNT = 2;
float textureCoordinateDataBuffer[72] = {};

GLuint imageTexture = 0;

GLuint programId = 0;

const int LOCATION_ATTRIBUTE_POSITION = 0;
const int LOCATION_ATTRIBUTE_TEXTURE_COORDINATE = 1;
const int LOCATION_ATTRIBUTE_NORMAL = 2;
const int LOCATION_UNIFORM_TEXTURE = 0;

float translateX = 0.0f;
float translateY = 0.0f;
float translateZ = 0.0f;
float rotateX = 30.0f;
float rotateY = -45.0f;
float rotateZ = 0.0f;
float scaleX = 1.0f;
float scaleY = 1.0f;
float scaleZ = 1.0;
float cameraPositionX = 0.0f;
float cameraPositionY = 0.0f;
float cameraPositionZ = 7.0f;
float lookAtX = 0.0f;
float lookAtY = 0.0f;
float lookAtZ = 0.0f;
float cameraUpX = 0.0f;
float cameraUpY = 1.0f;
float cameraUpZ = 0.0f;
float nearPlaneLeft = -1.0f;
float nearPlaneRight = 1.0f;
float nearPlaneBottom = -1.0f;
float nearPlaneTop = 1.0f;
float nearPlane = 2.0f;
float farPlane = 100.0f;


void getIdentity(ESMatrix *matrix) {
    esMatrixLoadIdentity(matrix);
}

void onDrawFrame(ESContext *esContext) {
    rotateY += 1.0f;
    glUseProgram(programId);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glClear(GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, glSurfaceViewWidth, glSurfaceViewHeight);

    glEnableVertexAttribArray(LOCATION_ATTRIBUTE_POSITION);
    glVertexAttribPointer(LOCATION_ATTRIBUTE_POSITION, VERTEX_COMPONENT_COUNT, GL_FLOAT, false, 0,
                          vertexDataBuffer);
    glEnableVertexAttribArray(LOCATION_ATTRIBUTE_TEXTURE_COORDINATE);
    glVertexAttribPointer(LOCATION_ATTRIBUTE_TEXTURE_COORDINATE, TEXTURE_COORDINATE_COMPONENT_COUNT,
                          GL_FLOAT, false, 0, textureCoordinateDataBuffer);
    glEnableVertexAttribArray(LOCATION_ATTRIBUTE_NORMAL);
    glVertexAttribPointer(LOCATION_ATTRIBUTE_NORMAL, NORMAL_COMPONENT_COUNT, GL_FLOAT, false, 0,
                          normalDataBuffer);

    ESMatrix mvpMatrix;
    getIdentity(&mvpMatrix);
    ESMatrix translateMatrix;
    getIdentity(&mvpMatrix);
    ESMatrix rotateMatrix;
    getIdentity(&rotateMatrix);
    ESMatrix scaleMatrix;
    getIdentity(&scaleMatrix);
    ESMatrix modelMatrix;
    getIdentity(&modelMatrix);
    ESMatrix viewMatrix;
    getIdentity(&viewMatrix);
    ESMatrix projectMatrix;
    getIdentity(&projectMatrix);
    esTranslate(&translateMatrix, translateX, translateY, translateZ);
    esRotate(&rotateMatrix, rotateZ, 1.0f, 0.0f, 0.0f);
    esRotate(&rotateMatrix, rotateY, 0.0f, 1.0f, 0.0f);
    esRotate(&rotateMatrix, rotateX, 0.0f, 0.0f, 1.0f);
    esScale(&scaleMatrix, scaleX, scaleY, scaleZ);
    esMatrixMultiply(&modelMatrix, &rotateMatrix, &scaleMatrix);
    esMatrixMultiply(&modelMatrix, &modelMatrix, &translateMatrix);
    esMatrixLookAt(&viewMatrix, cameraPositionX, cameraPositionY, cameraPositionZ, lookAtX, lookAtY,
                   lookAtZ, cameraUpX, cameraUpY, cameraUpZ);

    esFrustum(&projectMatrix, nearPlaneLeft, nearPlaneRight, nearPlaneBottom, nearPlaneTop,
              nearPlane, farPlane);
    esMatrixMultiply(&mvpMatrix, &viewMatrix, &modelMatrix);
    esMatrixMultiply(&mvpMatrix, &projectMatrix, &mvpMatrix);

    glUniformMatrix4fv(glGetUniformLocation(programId, "model"), 1, false, &modelMatrix.m[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(programId, "view"), 1, false, &viewMatrix.m[0][0]);

    glUniformMatrix4fv(glGetUniformLocation(programId, "projection"), 1, false,
                       &projectMatrix.m[0][0]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imageTexture);
    glUniform1i(glGetUniformLocation(programId, "imageTex"), 0);
    glUniform3f(glGetUniformLocation(programId, "viewPos"), 0.0f, 0.0f, 5.0f);
    glUniform3f(glGetUniformLocation(programId, "lightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(programId, "objectColor"), 1.0f, 1.0f, 0.0f);

    glDrawArrays(
            GL_TRIANGLES, 0, sizeof (vertexData)/sizeof (float )/ VERTEX_COMPONENT_COUNT);
    if(glGetError() != 0) {
        LOGE("kanli glGetError");
    }
}

void onSurfaceChanged(ESContext *esContext, int width, int height) {
    glSurfaceViewWidth = width;
    glSurfaceViewHeight = height;
    nearPlaneBottom = -(float) height / (float) width;
    nearPlaneTop = (float) height / (float) width;
}


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

void onSurfaceCreated(ESContext *esContext, EGLConfig eglConfig) {
    programId = esLoadProgram(vDirectionallightsStr, fDirectionallightsStr);

    memcpy(vertexDataBuffer, vertexData, sizeof(vertexData));
    glEnableVertexAttribArray(LOCATION_ATTRIBUTE_POSITION);
    glVertexAttribPointer(LOCATION_ATTRIBUTE_POSITION, VERTEX_COMPONENT_COUNT,
                          GL_FLOAT, false,
                          0, vertexDataBuffer);

    memcpy(normalDataBuffer, normalData, sizeof(normalData));
    glEnableVertexAttribArray(LOCATION_ATTRIBUTE_NORMAL);
    glVertexAttribPointer(LOCATION_ATTRIBUTE_NORMAL, NORMAL_COMPONENT_COUNT,
                          GL_FLOAT, false,
                          0, normalDataBuffer);

    memcpy(textureCoordinateDataBuffer, textureCoordinateData, sizeof(textureCoordinateData));
    glEnableVertexAttribArray(LOCATION_ATTRIBUTE_TEXTURE_COORDINATE);
    glVertexAttribPointer(LOCATION_ATTRIBUTE_TEXTURE_COORDINATE, TEXTURE_COORDINATE_COMPONENT_COUNT,
                          GL_FLOAT, false,
                          0, textureCoordinateDataBuffer);


    glActiveTexture(GL_TEXTURE0);
    imageTexture = LoadTexture(esContext->platformData, "lighting/brick.png");

    glUniform1i(LOCATION_UNIFORM_TEXTURE, 0);
    glEnable(GL_DEPTH_TEST);
}

int defaultFramebuffer;

void offscreenRender(ESContext *esContext) {
    GLfloat *positions;
    GLuint *indices;

}