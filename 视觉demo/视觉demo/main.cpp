//
//  main.cpp
//  视觉demo
//

#include "GLTools.h"
#include "GLShaderManager.h"
#include "GLFrustum.h"
#include "GLBatch.h"
#include "GLMatrixStack.h"
#include "GLGeometryTransform.h"
#include "StopWatch.h"

#include <math.h>
#include <stdio.h>

#ifdef __APPLE__
#include <glut/glut.h>
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>
#endif
#include "GLTools.h"
#include <glut/glut.h>

GLShaderManager shaderManager;
GLMatrixStack modelViewMatrix;
GLMatrixStack projectionMatrix;
GLFrustum viewFrustum;
GLGeometryTransform transformPipeline;

GLBatch floorBatch;
GLBatch ceilingBatch;
GLBatch leftWallBatch;
GLBatch rightWallBatch;

GLFrame cameraFrame;

GLfloat viewZ = -65.0;

#define TEXTURE_BRICK 0 //墙面
#define TEXTURE_FLOOR 1 //地板
#define TEXTURE_CEILING 2 //天花板
#define TEXTURE_COUNT 3 //纹理个数

GLuint textures[TEXTURE_COUNT];
const char *szTextureFiles[TEXTURE_COUNT] = {"brick.tga", "floor.tga", "ceiling.tga"};

//窗口大小改变时接受新的宽度和高度，其中0，0代表窗口中视图的左下角坐标，w，h代表像素
void ChangeSize(int w,int h)
{
    glViewport(0, 0, w, h);
    
    viewFrustum.SetPerspective(80, float(w) / float(h), 1.0, 120);
    projectionMatrix.LoadMatrix(viewFrustum.GetProjectionMatrix());
    transformPipeline.SetMatrixStacks(modelViewMatrix, projectionMatrix);
}

//为程序做一次性的设置
void SetupRC()
{
    //设置背景色
    glClearColor(0.0f,0.0f,0.0f,1.0f);
    
    //初始化着色管理器
    shaderManager.InitializeStockShaders();
    
    //1.定义纹理需要的相关变量
    GLbyte *pBytes;
    GLint iWidth, iHeight, iComponents;
    GLenum eFormat;
    GLint iLoop;
    
    //2.申请TEXTURE_COUNT个纹理对象
    glGenTextures(TEXTURE_COUNT, textures);
    
    //3.设置纹理属性
    for (iLoop = 0; iLoop < TEXTURE_COUNT; iLoop++) {
        //3.1绑定纹理
        glBindTexture(GL_TEXTURE_2D, textures[iLoop]);
        //3.2加载纹理
        pBytes = gltReadTGABits(szTextureFiles[iLoop], &iWidth, &iHeight, &iComponents, &eFormat);
        
        //3.3放大缩小过滤方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        
        //3.4环绕方式
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        //3.5设定纹理
        glTexImage2D(GL_TEXTURE_2D, 0, iComponents, iWidth, iHeight, 0, eFormat, GL_UNSIGNED_BYTE, pBytes);
        //3.6设置mipmap
        glGenerateMipmap(GL_TEXTURE_2D);
        free(pBytes);
    }
    
    GLfloat z;
    //4.地板批次类设置
    floorBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0; z-=10) {
        
        floorBatch.MultiTexCoord2f(0, 0, 0);
        floorBatch.Vertex3f(-10, -10, z);

        floorBatch.MultiTexCoord2f(0, 1, 0);
        floorBatch.Vertex3f(10, -10, z);

        floorBatch.MultiTexCoord2f(0, 0, 1);
        floorBatch.Vertex3f(-10, -10, z-10);

        floorBatch.MultiTexCoord2f(0, 1, 1);
        floorBatch.Vertex3f(10, -10, z-10);
    }
    floorBatch.End();
    
    //5.天花板批次类设置
    ceilingBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0; z-=10) {
        ceilingBatch.MultiTexCoord2f(0, 0, 0);
        ceilingBatch.Vertex3f(-10, 10, z);
        
        ceilingBatch.MultiTexCoord2f(0, 0, 1);
        ceilingBatch.Vertex3f(-10, 10, z - 10);
        
        ceilingBatch.MultiTexCoord2f(0, 1, 0);
        ceilingBatch.Vertex3f(10, 10, z);
        
        ceilingBatch.MultiTexCoord2f(0, 1, 1);
        ceilingBatch.Vertex3f(10, 10, z - 10);
    }
    ceilingBatch.End();
    
    //6.左侧墙壁批次类设置
    leftWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0; z-=10) {
        leftWallBatch.MultiTexCoord2f(0, 0, 0);
        leftWallBatch.Vertex3f(-10, -10, z);
        
        leftWallBatch.MultiTexCoord2f(0, 0, 1);
        leftWallBatch.Vertex3f(-10, 10, z);
        
        leftWallBatch.MultiTexCoord2f(0, 1, 0);
        leftWallBatch.Vertex3f(-10, -10, z - 10);
        
        leftWallBatch.MultiTexCoord2f(0, 1, 1);
        leftWallBatch.Vertex3f(-10, 10, z - 10);
    }
    leftWallBatch.End();
    
    //7.右侧墙壁批次类设置
    rightWallBatch.Begin(GL_TRIANGLE_STRIP, 28, 1);
    for (z = 60; z >= 0; z-=10) {
        rightWallBatch.MultiTexCoord2f(0, 0, 0);
        rightWallBatch.Vertex3f(10, -10, z);
        
        rightWallBatch.MultiTexCoord2f(0, 0, 1);
        rightWallBatch.Vertex3f(10, 10, z);
        
        rightWallBatch.MultiTexCoord2f(0, 1, 0);
        rightWallBatch.Vertex3f(10, -10, z - 10);
        
        rightWallBatch.MultiTexCoord2f(0, 1, 1);
        rightWallBatch.Vertex3f(10, 10, z - 10);
    }
    rightWallBatch.End();
    
}

//开始渲染
void RenderScene(void) {
    //清楚一个或一组特定的缓冲区
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    
    M3DMatrix44f mCamera;
    cameraFrame.GetCameraMatrix(mCamera);
    modelViewMatrix.MultMatrix(mCamera);
    
    modelViewMatrix.PushMatrix(mCamera);
    modelViewMatrix.Translate(0, 0, viewZ);
    
    shaderManager.UseStockShader(GLT_SHADER_TEXTURE_REPLACE, transformPipeline.GetModelViewProjectionMatrix(), 0);
    
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_FLOOR]);
    floorBatch.Draw();
    
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_CEILING]);
    ceilingBatch.Draw();
    
    glBindTexture(GL_TEXTURE_2D, textures[TEXTURE_BRICK]);
    leftWallBatch.Draw();
    rightWallBatch.Draw();
    
    modelViewMatrix.PopMatrix();
    
    //将在后台缓冲区进行渲染，然后在结束时交换到前台
    glutSwapBuffers();
}

void SpecialKeys(int key, int x, int y) {
    
    float angular = float(m3dDegToRad(5));
    if (key == GLUT_KEY_UP) {
        viewZ += 0.5;
    }
    if (key == GLUT_KEY_DOWN) {
        viewZ -= 0.5;
    }
    if (key == GLUT_KEY_LEFT) {
        cameraFrame.RotateWorld(angular, 0, 1, 0);
    }
        
    if (key == GLUT_KEY_RIGHT) {
        cameraFrame.RotateWorld(-angular, 0, 1, 0);
    }
    
    glutPostRedisplay();
}

void ProcessMenu(int value) {
    GLint index;
    for (index = 0; index < TEXTURE_COUNT; index++) {
        glBindTexture(GL_TEXTURE_2D, textures[index]);
        
        switch (value) {
            case 0:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                break;
                
            case 1:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                break;
                
            case 2:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
                break;
                
            case 3:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
                
            case 4:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
                break;
                
            case 5:
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                break;
                
            case 6:
            
                //设置各向异性过滤
                GLfloat fLargest;
                //获取各向异性过滤的最大数量
                glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
                //设置纹理参数(各向异性采样)
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
                break;
        
            case 7:
                //设置各向同性过滤，数量为1.0表示(各向同性采样)
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
                break;
        }
    }
    
    glutPostRedisplay();
}

void ShutdownRC(void) {
    glDeleteTextures(TEXTURE_COUNT, textures);
}

int main(int argc,char* argv[]) {

    //设置当前工作目录，针对MAC OS X
    gltSetWorkingDirectory(argv[0]);
    
    //初始化GLUT库
    glutInit(&argc, argv);
    /*初始化双缓冲窗口，其中标志GLUT_DOUBLE、GLUT_RGBA、GLUT_DEPTH、GLUT_STENCIL分别指
     双缓冲窗口、RGBA颜色模式、深度测试、模板缓冲区
     */
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH|GLUT_STENCIL);
    
    //GLUT窗口大小
    glutInitWindowSize(800,600);
    glutCreateWindow("Tunnel");
    
    //注册回调函数
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    glutSpecialFunc(SpecialKeys);
    
    //menu
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("GL_NEAREST", 0);
    glutAddMenuEntry("GL_LINEAR", 1);
    glutAddMenuEntry("GL_NEAREST_MIPMAP_NEAREST", 2);
    glutAddMenuEntry("GL_NEARSET_MIPMAP_LINEAR", 3);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_NEARSET", 4);
    glutAddMenuEntry("GL_LINEAR_MIPMAP_LINEAR", 5);
    glutAddMenuEntry("Anisotropic Filter", 6);
    glutAddMenuEntry("Anisotropic Off", 7);
    
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    //驱动程序的初始化中没有出现任何问题
    GLenum err = glewInit();
    if(GLEW_OK != err) {
        fprintf(stderr,"glew error:%s\n",glewGetErrorString(err));
        return 1;
    }

    //调用SetupRC
    SetupRC();
    glutMainLoop();
    
    ShutdownRC();
    
    return 0;
}
