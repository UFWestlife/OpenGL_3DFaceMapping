// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <array>
#include <stack>
#include <sstream>
#include <fstream>

// Include GLEW
#include <GL/glew.h>
// Include GLFW
#include <glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace glm;
// Include AntTweakBar
#include <AntTweakBar.h>

#include <common/shader.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>
#include <common/tga.h>
#include <common/ray_casting.h>

const int window_width = 600, window_height = 600;

typedef struct Vertex {
    float Position[4];
    float Color[4];
    float Normal[3];
    float UV[2];
    void SetPosition(float *coords) {
        Position[0] = coords[0];
        Position[1] = coords[1];
        Position[2] = coords[2];
        Position[3] = 1.0;
    }
    void SetColor(float *color) {
        Color[0] = color[0];
        Color[1] = color[1];
        Color[2] = color[2];
        Color[3] = color[3];
    }
    void SetNormal(float *coords) {
        Normal[0] = coords[0];
        Normal[1] = coords[1];
        Normal[2] = coords[2];
    }
    void SetUV(float *coords) {
        UV[0] = coords[0];
        UV[1] = coords[1];
    }
} Vertex;

// function prototypes
int initWindow(void);
void initOpenGL(void);
void loadObject(char*, glm::vec4, Vertex * &, GLuint* &, int);
void createVAOs(Vertex[], GLuint[], int);
void createObjects(void);
void pickObject(void);
void renderScene(void);
void cleanup(void);
static void keyCallback(GLFWwindow*, int, int, int, int);
static void mouseCallback(GLFWwindow*, int, int, int);
void subdivision(void);
void subtriangle(void);

// GLOBAL VARIABLES
GLFWwindow* window;

glm::mat4 gProjectionMatrix;
glm::mat4 gViewMatrix;

GLuint programID;
GLuint pickingProgramID;

const GLuint NumObjects = 10;	// ATTN: THIS NEEDS TO CHANGE AS YOU ADD NEW OBJECTS
/*
 0 = XYZ axes
 1 = Face model
 2 = XZ grid
 3 = Ctl Mesh Points
 4 = Ctl Mesh Lines
 5 = Triangles of the green grids
 6 = Yellow points
 7 = smotthed surface
 8 = Hair
 */

GLuint VertexArrayId[NumObjects] = { 0 };
GLuint VertexBufferId[NumObjects] = { 0 };
GLuint IndexBufferId[NumObjects] = { 0 };

size_t NumIndices[NumObjects] = { 0 };
size_t VertexBufferSize[NumObjects] = { 0 };
size_t IndexBufferSize[NumObjects] = { 0 };

GLuint MatrixID;
GLuint ModelMatrixID;
GLuint ViewMatrixID;
GLuint ProjMatrixID;
GLuint PickingMatrixID;
GLuint pickingColorArrayID;
GLuint LightID;
GLuint useLightModelID;
GLuint useTextureID;
GLuint TextureID;//wangzun codes//

void loadObject(char* file, glm::vec4 color, Vertex * &out_Vertices, GLuint* &out_Indices, int ObjectId)
{
    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    if (!loadOBJ(file, vertices, normals)) return;
    
    std::vector<GLuint> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    indexVBO(vertices, normals, indices, indexed_vertices, indexed_normals);
    
    const size_t vertCount = indexed_vertices.size();
    const size_t idxCount = indices.size();
    
    // populate output arrays
    out_Vertices = new Vertex[vertCount];
    for (int i = 0; i < vertCount; i++) {
        out_Vertices[i].SetPosition(&indexed_vertices[i].x);
        out_Vertices[i].SetNormal(&indexed_normals[i].x);
        out_Vertices[i].SetColor(&color[0]);
    }
    out_Indices = new GLuint[idxCount];
    for (int i = 0; i < idxCount; i++) {
        out_Indices[i] = indices[i];
    }
    
    // set global variables!!
    NumIndices[ObjectId] = idxCount;
    VertexBufferSize[ObjectId] = sizeof(out_Vertices[0]) * vertCount;
    IndexBufferSize[ObjectId] = sizeof(GLuint) * idxCount;
}

// Added Global Variables
// For setting which face model to use
int faceModelID = 0;
char* faceModelName[2] = { "Jin_Ke.obj", "Wang_Zun.obj" };
char* faceTexName[2] = { "Jin_Ke.tga", "Wang_Zun.tga" };
char* faceHairName[2] = { "Jin_Ke_Hair.obj", "Wang_Zun_Hair.obj" };
char* faceSmileName[2] = { "Jin_Ke_Smile.p3", "Wang_Zun_Smile.p3" };
char* faceFrownName[2] = { "Jin_Ke_Frown.p3", "Wang_Zun_LOL.p3" };

// For camera rotating
double CAngleXZ=0.0;
double CAngleXY=0.0;
double CRadius=0.0;
glm::vec3 cameraCoords;
float cameraUp;
int direction=0;
// For rendering face model
bool showFace = true;

// For drawing control mesh
void generateCylinderCtlMesh(float rad);
int showControlMesh = 0;
float meshOffset[2][3] = {0, -0.28, -0.07, -0.25, -0.25, 0};
float radius[2] = { 0.5, 0.5 };
float height[2] = { 10.25, 10 };
Vertex CtlVerts[441];
Vertex CtlMeshVerts[2400];
GLuint texture;

// For refresh/init control mesh
void updateControlMesh(bool isInit);

// For picking
float pickingColors[441][4];
GLuint pickedIndex = -1;
bool holdingShift = false;
bool enableDrag = false;
double lastXPos = -1, lastYPos = -1;

//For subdivision
bool showSubd = false;
Vertex subVerts[3249];
Vertex subMeshVerts[17496];

// For showing hair model;
bool showHair = false;

// For save and load
void saveVerts(char* filename);
void loadVerts(char* filename, Vertex* v);

// For animation (smile, frown)
Vertex delta[441];
int animationCount = 0, animationLimit = 120;
bool animation = false;

void createObjects(void)
{
    //-- COORDINATE AXES --//
    Vertex CoordVerts[] =
    {
        { { 0.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
        { { 5.0, 0.0, 0.0, 1.0 }, { 1.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
        { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
        { { 0.0, 5.0, 0.0, 1.0 }, { 0.0, 1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0 } },
        { { 0.0, 0.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
        { { 0.0, 0.0, 5.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 }, { 0.0, 0.0, 1.0 } },
    };
    
    VertexBufferSize[0] = sizeof(CoordVerts);	// ATTN: this needs to be done for each hand-made object with the ObjectID (subscript)
    createVAOs(CoordVerts, NULL, 0);
    
    //-- GRID --//
    
    // ATTN: create your grid vertices here!
    // wangzun start
    // draw XZ grid
    Vertex GridVerts[84];
    int GridVertsCount=0;
    float tempCordGrid[4]={0.0, 0.0, 0.0, 1.0};
    float tempColorGrid[4]={1.0, 1.0, 1.0, 1.0};
    float tempNormalGrid[3]={0.0, 1.0, 0.0};
    for (float tempX=-5; tempX<=5; tempX=tempX+0.5) {
        tempCordGrid[0] = tempX;
        tempCordGrid[2] = 5.0;
        
        // Add the first point
        GridVerts[GridVertsCount].SetPosition(tempCordGrid);
        GridVerts[GridVertsCount].SetColor(tempColorGrid);
        GridVerts[GridVertsCount].SetNormal(tempNormalGrid);
        GridVertsCount++;
        
        // Add the second point
        tempCordGrid[2] = -tempCordGrid[2];
        GridVerts[GridVertsCount].SetPosition(tempCordGrid);
        GridVerts[GridVertsCount].SetColor(tempColorGrid);
        GridVerts[GridVertsCount].SetNormal(tempNormalGrid);
        GridVertsCount++;
    }
    
    for (float tempZ=-5; tempZ<=5; tempZ=tempZ+0.5) {
        tempCordGrid[2] = tempZ;
        tempCordGrid[0] = 5.0;
        
        // Add the first point
        GridVerts[GridVertsCount].SetPosition(tempCordGrid);
        GridVerts[GridVertsCount].SetColor(tempColorGrid);
        GridVerts[GridVertsCount].SetNormal(tempNormalGrid);
        GridVertsCount++;
        
        // Add the second point
        tempCordGrid[0] = -tempCordGrid[0];
        GridVerts[GridVertsCount].SetPosition(tempCordGrid);
        GridVerts[GridVertsCount].SetColor(tempColorGrid);
        GridVerts[GridVertsCount].SetNormal(tempNormalGrid);
        GridVertsCount++;
    }
    VertexBufferSize[2] = sizeof(GridVerts);
    createVAOs(GridVerts, NULL, 2);
    
    // draw Control Mesh
    float tempPos[4]={0.0, 0.0, meshOffset[faceModelID][2], 1.0};
    float tempColor[4]={0.0, 1.0, 0.0, 1.0};
    float tempNormal[3]={0.0, 0.0, 1.0};
    float tempUV[2];
    int tempIndex;
    
    for (int i=0; i<21; i++) {
        tempPos[0] = -5.0 + i*0.5 + meshOffset[faceModelID][0];
        tempUV[0] = i/20.0;
        for (int j=0; j<21; j++) {
            tempPos[1] = j*0.5 + meshOffset[faceModelID][1];
            tempUV[1] = j/20.0;
            tempIndex = i*21+j;
            CtlVerts[tempIndex].SetPosition(tempPos);
            CtlVerts[tempIndex].SetColor(tempColor);
            CtlVerts[tempIndex].SetNormal(tempNormal);
            CtlVerts[tempIndex].SetUV(tempUV);
            
            if (tempIndex<256) {
                pickingColors[tempIndex][0] = tempIndex/255.0;
                pickingColors[tempIndex][1] = pickingColors[tempIndex][2] = 0;
            }
            else {
                pickingColors[tempIndex][0] = 1.0;
                pickingColors[tempIndex][1] = (tempIndex-255.0)/255.0;
                pickingColors[tempIndex][2] = 0;
            }
            pickingColors[tempIndex][3] = 1.0;
        }
    }
    //Add subdivision points
    VertexBufferSize[6] = sizeof(subVerts);
    createVAOs(subVerts, NULL, 6);
    
    //Add subdivision triangles
    VertexBufferSize[7] = sizeof(subMeshVerts);
    createVAOs(subMeshVerts, NULL, 7);
    
    updateControlMesh(true);
    
    // TGA Import Functions
    long twidth = 0;
    long theight = 0;
    texture = load_texture_TGA(faceTexName[faceModelID], &twidth, &theight, GL_CLAMP, GL_CLAMP);
    
    //wangzun end //
    
    //-- .OBJs --//
    // ATTN: load your models here
    Vertex* Verts;
    GLuint* Idcs;
    //loadObject("models/base.obj", glm::vec4(1.0, 0.0, 0.0, 1.0), Verts, Idcs, ObjectID);
    //createVAOs(Verts, Idcs, ObjectID);
    
    loadObject(faceModelName[faceModelID], glm::vec4(235.0f/255.0f, 180.0f/255.0f, 140.0f/255.0f, 1.0), Verts, Idcs, 1);
    createVAOs(Verts, Idcs, 1);
    
    loadObject(faceHairName[faceModelID], glm::vec4(40.0f/255.0f, 30.0f/255.0f, 30.0f/255.0f, 1.0), Verts, Idcs, 8);
    createVAOs(Verts, Idcs, 8);
    
    delete[] Verts;
    delete[] Idcs;
}

void updateControlMesh(bool isInit) {
    if (isInit) {
        VertexBufferSize[3] = sizeof(CtlVerts);
        createVAOs(CtlVerts, NULL, 3);
    }
    else {
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CtlVerts), CtlVerts);
        
    }
    //horizontal mesh lines
    Vertex GreenGridVerts[1680];
    for (int i = 0; i < 21; i++) {
        for (int j = 0; j < 20; j++) {
            GreenGridVerts[i * 40 + j*2] = CtlVerts[i * 21 + j];
            GreenGridVerts[i * 40 + j*2 + 1] = CtlVerts[i * 21 + j+1];
        }
    }
    //vertical mesh lines
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 21; j++) {
            GreenGridVerts[840 + i * 42 + j * 2] = CtlVerts[j + (i * 21)];
            GreenGridVerts[840 + i * 42 + j * 2 + 1] = CtlVerts[j + ((i + 1) * 21)];
        }
    }
    if (isInit) {
        VertexBufferSize[4] = sizeof(GreenGridVerts);
        createVAOs(GreenGridVerts, NULL, 4);
    }
    else {
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[4]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GreenGridVerts), GreenGridVerts);
    }
    
    //wangzun start
    int count = 0;
    int vert = 0;
    int hor = 0;
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            
            vert = i * 21;
            hor = j;
            
            /*
             // Reverse texture
             //triangle 1
             CtlMeshVerts[count] = CtlVerts[vert + hor + 21];
             CtlMeshVerts[count + 1] = CtlVerts[vert + hor];
             CtlMeshVerts[count + 2] = CtlVerts[vert + hor + 22];
             
             //triangle 2
             CtlMeshVerts[count + 3] = CtlVerts[vert + hor + 1];
             CtlMeshVerts[count + 4] = CtlVerts[vert + hor + 22];
             CtlMeshVerts[count + 5] = CtlVerts[vert + hor];
             */
            
            //triangle 1
            CtlMeshVerts[count] = CtlVerts[vert + hor + 0];
            CtlMeshVerts[count + 1] = CtlVerts[vert + hor + 21];
            CtlMeshVerts[count + 2] = CtlVerts[vert + hor + 1];
            
            //triangle 2
            CtlMeshVerts[count + 3] = CtlVerts[vert + hor +22];
            CtlMeshVerts[count + 4] = CtlVerts[vert + hor + 1];
            CtlMeshVerts[count + 5] = CtlVerts[vert + hor + 21];
            
            
            count +=6;
        }
    }
    if (isInit) {
        VertexBufferSize[5] = sizeof(CtlMeshVerts);
        createVAOs(CtlMeshVerts, NULL, 5);
    }
    else {
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[5]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CtlMeshVerts), CtlMeshVerts);
    }

	if (showSubd) {
		subdivision();
		subtriangle();
	}

}

// Moving Camera
void moveCamera( double delta ) {
    switch (direction) {
        case GLFW_KEY_LEFT:
            CAngleXZ -= delta;
            break;
        case GLFW_KEY_RIGHT:
            CAngleXZ += delta;
            break;
        case GLFW_KEY_UP:
            CAngleXY += delta;
            break;
        case GLFW_KEY_DOWN:
            CAngleXY -= delta;
            break;
        default:
            break;
    }
    cameraUp = 1.0;
    // Horizontal limit
    if (CAngleXZ > 2*pi<double>())
        CAngleXZ -= 2*pi<double>();
    if (CAngleXZ < -2*pi<double>())
        CAngleXZ += 2*pi<double>();
    
    // Vertical limit
    if (CAngleXY > 2*pi<double>())
        CAngleXY -= 2*pi<double>();
    if (CAngleXY < -2*pi<double>())
        CAngleXY += 2*pi<double>();
    
    if (CAngleXY > pi<double>()/2 && CAngleXY < 3*pi<double>()/2) cameraUp = -1.0;  // For W
    if (CAngleXY < -pi<double>()/2 && CAngleXY > -3*pi<double>()/2) cameraUp = -1.0;  // For S
    
    cameraCoords.x = CRadius * cos(CAngleXY) * cos(CAngleXZ);
    cameraCoords.y = CRadius * sin(CAngleXY);
    cameraCoords.z = CRadius * cos(CAngleXY) * sin(CAngleXZ);
    
    gViewMatrix = glm::lookAt(cameraCoords,	// eye
                              glm::vec3(0.0, 0.0, 0.0),	// center
                              glm::vec3(0.0, cameraUp, 0.0));	// up
}

void resetState() {
    
    // Clear rotation
    cameraCoords = glm::vec3(10.0, 10.0, 10.0f);
    cameraUp = 1.0;
    // Init camera angle & radius for later rotation
    CRadius = sqrt(3 * (10 * 10));
    CAngleXZ = atan(1);
    CAngleXY = atan(1 / sqrt(2));
    
    gViewMatrix = glm::lookAt(cameraCoords,	// eye
                              glm::vec3(0.0, 0.0, 0.0),	// center
                              glm::vec3(0.0, cameraUp, 0.0));	// up
    
    showFace = true;
    showControlMesh = 0;
	animation = false;
    showHair = false;

    createObjects();
    
}

void renderScene(void)
{
    //ATTN: DRAW YOUR SCENE HERE. MODIFY/ADAPT WHERE NECESSARY!
    
    
    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.2f, 0.0f);
    // Re-clear the screen for real rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(programID);
    {
        glPointSize(7.0);
        glm::vec3 lightPos = glm::vec3(0, 10, 10);
        glm::mat4x4 ModelMatrix = glm::mat4(1.0);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &gViewMatrix[0][0]);
        glUniformMatrix4fv(ProjMatrixID, 1, GL_FALSE, &gProjectionMatrix[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        
        glUniform1i(useLightModelID, false);
        glUniform1i(useTextureID, false);
        glBindVertexArray(VertexArrayId[0]);	// draw CoordAxes
        glDrawArrays(GL_LINES, 0, 6);
        
        if (showFace) {
            glUniform1i(useLightModelID, true);
            glBindVertexArray(VertexArrayId[1]);
            glDrawElements(GL_TRIANGLES, VertexBufferSize[1], GL_UNSIGNED_INT, NULL);
        }
        //wangzun start//
        // draw grid
        glUniform1i(useLightModelID, false);
        glBindVertexArray(VertexArrayId[2]);
        glDrawArrays(GL_LINES, 0, 84);
        
        if (showControlMesh == 1) {
            // Draw Mesh Verticies
            glBindVertexArray(VertexArrayId[3]);
            glDrawArrays(GL_POINTS, 0, 441);
            
            // Draw lines of Mesh
            glBindVertexArray(VertexArrayId[4]);
            glDrawArrays(GL_LINES, 0, 1680);
            
        }
        
        if (showControlMesh >= 1) {
            // Draw Triangles for Mesh Using Texture
            glUniform1i(useTextureID, true);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            glUniform1i(TextureID, 0);
			if (!showSubd) {
				glBindVertexArray(VertexArrayId[5]);
				glDrawArrays(GL_TRIANGLES, 0, 2400);
			}
			else {
				// Draw Triangles for Mesh Using Texture
				glUniform1i(useTextureID, true);
				glBindVertexArray(VertexArrayId[7]);
				glDrawArrays(GL_TRIANGLES, 0, (972 * 18));
			}
        }
        
        if (showHair) {
            //Draw Hair
            glUniform1i(useTextureID, false);
            glUniform1i(useLightModelID, true);
            glBindVertexArray(VertexArrayId[8]);
            glDrawElements(GL_TRIANGLES, VertexBufferSize[8], GL_UNSIGNED_INT, NULL);
        }
        
        //wangzun end
        
        glBindVertexArray(0);
        
    }
    glUseProgram(0);
    
    // Draw GUI
    //TwDraw();
    
    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void pickObject(void)
{
    // Clear the screen in white
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(pickingProgramID);
    {
        glPointSize(7.5);
        glm::mat4 ModelMatrix = glm::mat4(1.0); // TranslationMatrix * RotationMatrix;
        glm::mat4 MVP = gProjectionMatrix * gViewMatrix * ModelMatrix;
        
        // Send our transformation to the currently bound shader, in the "MVP" uniform
        glUniformMatrix4fv(PickingMatrixID, 1, GL_FALSE, &MVP[0][0]);
        
        // ATTN: DRAW YOUR PICKING SCENE HERE. REMEMBER TO SEND IN A DIFFERENT PICKING COLOR FOR EACH OBJECT BEFOREHAND
        glUniform4fv(pickingColorArrayID, 441, &pickingColors[0][0]);	// here we pass in the picking marker array
        
        // Draw the points
        glBindVertexArray(VertexArrayId[3]);
        //glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
        //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CtlVerts), CtlVerts);	// update buffer data
        glDrawArrays(GL_POINTS, 0, 441);
        glBindVertexArray(0);
        
    }
    glUseProgram(0);
    // Wait until all the pending drawing commands are really done.
    // Ultra-mega-over slow !
    // There are usually a long time between glDrawElements() and
    // all the fragments completely rasterized.
    glFlush();
    glFinish();
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Read the pixel at the center of the screen.
    // You can also use glfwGetMousePos().
    // Ultra-mega-over slow too, even for 1 pixel,
    // because the framebuffer is on the GPU.
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    unsigned char data[4];
    glReadPixels(xpos, window_height - ypos, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data); // OpenGL renders with (0,0) on bottom, mouse reports with (0,0) on top
    
    // Convert the color back to an integer ID
    if (data[2] == 255) // Background
        pickedIndex = -1;
    else {
        if (data[0] < 255)
            pickedIndex = int(data[0]);
        else pickedIndex = int(data[0])+int(data[1]);
    }
    
    /*
     std::ostringstream oss;
     oss << "point " << gPickedIndex;
     gMessage = oss.str();
     */
    printf("%d\n", pickedIndex);
    
    // Uncomment these lines to see the picking shader in effect
    //glfwSwapBuffers(window);
}

void moveVertex(void)
{
    if (pickedIndex == -1)
        return;
    
    // retrieve your cursor position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    // move points
    //Change the position of both the original vertices and the vertices for drawing lines
    
    if (lastXPos == -1) {
        lastXPos = xpos;
        lastYPos = ypos;
        return;
    }
    double deltaX = xpos - lastXPos;
    double deltaY = ypos - lastYPos;
    if (holdingShift) {
        /*
         if (lastXPos == -255) lastXPos = Vertices[gPickedIndex].XYZW[0];
         float delta = newPos.x - lastXPos;
         Vertices[gPickedIndex].XYZW[2] += delta;
         lastXPos = newPos.x;
         */
        if (cameraCoords.x<0)
            CtlVerts[pickedIndex].Position[2] += cameraUp * (deltaX/60.0);
        else
            CtlVerts[pickedIndex].Position[2] -= cameraUp * (deltaX/60.0);
    }
    else {
        CtlVerts[pickedIndex].Position[0] += cameraUp * (deltaX/60.0);
        CtlVerts[pickedIndex].Position[1] -= cameraUp * (deltaY/60.0);
    }
    lastXPos = xpos;
    lastYPos = ypos;
    
    updateControlMesh(false);
    
}

void generateCylinderCtlMesh(float rad) {
    // draw Control Mesh
    float tempPos[4]={0.0, 0.0, meshOffset[faceModelID][2], 1.0};
    float tempColor[4]={0.0, 1.0, 0.0, 1.0};
    float tempUV[2];
    for (int i=0; i<21; i++) {
        tempPos[0] = -rad * sin(i*2*pi<float>()/20.0) + meshOffset[faceModelID][0];
        tempPos[2] = -rad * cos(i*2*pi<float>()/20.0) + meshOffset[faceModelID][2];
        tempUV[0] = i/20.0;
        for (int j=0; j<21; j++) {
            tempPos[1] = meshOffset[faceModelID][1] + height[faceModelID]*(j/20.0);
            tempUV[1] = j/20.0;
            CtlVerts[i*21+j].SetPosition(tempPos);
            CtlVerts[i*21+j].SetColor(tempColor);
            CtlVerts[i*21+j].SetUV(tempUV);
        }
    }
    VertexBufferSize[3] = sizeof(CtlVerts);
    //createVAOs(CtlVerts, NULL, 3);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[3]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(CtlVerts), CtlVerts);
}

void autoFit() {
    int triangleCount=0;
    float rayDirection[3] = {0.0, 0.0, 1.0};
    float bc[3];
    Vertex* Verts;
    GLuint* Idcs;
    vec3 vectorH, vectorV;
    loadObject(faceModelName[faceModelID], glm::vec4(1.0), Verts, Idcs, 1);
    
    // Get a cylinder control mesh
    generateCylinderCtlMesh(radius[faceModelID]);
    
    for (int ctlVertCount=5*21; ctlVertCount<16*21; ctlVertCount++) {
        for (triangleCount = 0; triangleCount < NumIndices[1]/3 ; triangleCount++) {
            
            rayDirection[0] = CtlVerts[ctlVertCount].Position[0] -meshOffset[faceModelID][0];
            rayDirection[2] = CtlVerts[ctlVertCount].Position[2] -meshOffset[faceModelID][2];
            
            ray_cast(Verts[Idcs[triangleCount*3]].Position, Verts[Idcs[triangleCount*3+1]].Position, Verts[Idcs[triangleCount*3+2]].Position, CtlVerts[ctlVertCount].Position, rayDirection, bc);
            if (bc[0] >= 0 && bc[1] >= 0 && bc[2] >= 0) {
                float tempPos[4];
                for (int i=0; i<3; i++) {
                    tempPos[i] =bc[0] * Verts[Idcs[triangleCount*3]].Position[i] +
                    bc[1] * Verts[Idcs[triangleCount*3+1]].Position[i] +
                    bc[2] * Verts[Idcs[triangleCount*3+2]].Position[i];
                }
                if (tempPos[2] < CtlVerts[ctlVertCount].Position[2] )
                    continue;
                float tempDis = tempPos[0]*tempPos[0] + tempPos[2]*tempPos[2];
                float oriDis = CtlVerts[ctlVertCount].Position[0]*CtlVerts[ctlVertCount].Position[0] + CtlVerts[ctlVertCount].Position[2] * CtlVerts[ctlVertCount].Position[2];
                if (tempDis < oriDis)
                    continue;
                tempPos[3] = 1.0;
                CtlVerts[ctlVertCount].SetPosition(tempPos);
            }
        }
    }
    
    updateControlMesh(false);
}

void subtriangle(){
    int count = 0;
    int hor = 0;
    int ver = 0;
    for (int offset = 0; offset < (171*18); offset=offset+171) {
        for (int i = 0; i <54; i++) {
            for (int j = 0; j < 3; j++) {
                hor = i*3;
                ver = j;
                if (j < 2) {
                    
                    subMeshVerts[count] = subVerts[ offset + hor + ver ];
                    subMeshVerts[count+1] = subVerts[ offset + hor + ver + 1];
                    subMeshVerts[count+2] = subVerts[ offset + hor+ ver  + 3];
                    
                    //triangle 2
                    subMeshVerts[count+3] = subVerts[ offset + hor + ver  + 4];
                    subMeshVerts[count+4] = subVerts[offset + hor + ver  + 3];
                    subMeshVerts[count+5] = subVerts[offset + hor + ver  + 1];
                    
                    count+=6;
                }
                else {
                    
                    subMeshVerts[count] = subVerts[ offset + hor + ver ];
                    subMeshVerts[count+1] = subVerts[ offset + hor + ver + 169];
                    subMeshVerts[count+2] = subVerts[offset + hor + ver + 2];
                    
                    //triangle 2
                    subMeshVerts[count+3] = subVerts[ offset + hor + ver + 172];
                    subMeshVerts[count+4] = subVerts[ offset + hor + ver + 2];
                    subMeshVerts[count+5] = subVerts[offset +  hor + ver + 169];
                    
                    count+=6;
                }
            }
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[7]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(subMeshVerts), subMeshVerts);
}

void subdivision(){
    float tempColor[4]={1.0, 1.0, 0.0, 1.0};
    float tempNormal[3]={0.0, 0.0, 1.0};
    int subCount = 0;
    int ctlCount = 22;
    
    for (ctlCount = 22; ctlCount < 418; ctlCount=ctlCount+2) {
        for (int j = 0; j < 19; j++) {
            for (int i = 0; i < 9; i++) {
                subVerts[subCount].SetColor(tempColor);
                subVerts[subCount].SetNormal(tempNormal);
                float tempPos[4];
                float tempUV[2];
                if (i == 0) {
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (16*CtlVerts[ctlCount].Position[m]+4*(CtlVerts[ctlCount+1].Position[m]+CtlVerts[ctlCount-1].Position[m]+CtlVerts[ctlCount+21].Position[m]+CtlVerts[ctlCount-21].Position[m])+(CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount-22].Position[m]+CtlVerts[ctlCount+20].Position[m])+CtlVerts[ctlCount-20].Position[m])/36;
                        if (m < 2) {
                            tempUV[m] = (16*CtlVerts[ctlCount].UV[m]+4*(CtlVerts[ctlCount+1].UV[m]+CtlVerts[ctlCount-1].UV[m]+CtlVerts[ctlCount+21].UV[m]+CtlVerts[ctlCount-21].UV[m])+(CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount-22].UV[m]+CtlVerts[ctlCount+20].UV[m])+CtlVerts[ctlCount-20].UV[m])/36;
                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 1){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (8*CtlVerts[ctlCount].Position[m]+2*(CtlVerts[ctlCount+1].Position[m]+CtlVerts[ctlCount-1].Position[m])+4*CtlVerts[ctlCount+21].Position[m]+CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount+20].Position[m])/18;
                        if (m < 2) {
                            tempUV[m] = (8*CtlVerts[ctlCount].UV[m]+2*(CtlVerts[ctlCount+1].UV[m]+CtlVerts[ctlCount-1].UV[m])+4*CtlVerts[ctlCount+21].UV[m]+CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount+20].UV[m])/18;
                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                    
                }
                else if(i == 2){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (8*CtlVerts[ctlCount+21].Position[m]+2*(CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount+20].Position[m])+4*CtlVerts[ctlCount].Position[m]+CtlVerts[ctlCount+1].Position[m]+CtlVerts[ctlCount-1].Position[m])/18;
                        if (m < 2) {
                            tempUV[m] = (8*CtlVerts[ctlCount+21].UV[m]+2*(CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount+20].UV[m])+4*CtlVerts[ctlCount].UV[m]+CtlVerts[ctlCount+1].UV[m]+CtlVerts[ctlCount-1].UV[m])/18;
                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 3){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (8*CtlVerts[ctlCount].Position[m]+2*(CtlVerts[ctlCount+21].Position[m]+CtlVerts[ctlCount-21].Position[m])+4*CtlVerts[ctlCount+1].Position[m]+CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount-20].Position[m])/18;
                        if (m < 2) {
                            tempUV[m] = (8*CtlVerts[ctlCount].UV[m]+2*(CtlVerts[ctlCount+21].UV[m]+CtlVerts[ctlCount-21].UV[m])+4*CtlVerts[ctlCount+1].UV[m]+CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount-20].UV[m])/18;
                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 4){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (4*CtlVerts[ctlCount].Position[m]+2*(CtlVerts[ctlCount+21].Position[m]+CtlVerts[ctlCount+1].Position[m])+CtlVerts[ctlCount+22].Position[m])/9;
                        if (m < 2) {
                            tempUV[m] = (4*CtlVerts[ctlCount].UV[m]+2*(CtlVerts[ctlCount+21].UV[m]+CtlVerts[ctlCount+1].UV[m])+CtlVerts[ctlCount+22].UV[m])/9;                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 5){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (4*CtlVerts[ctlCount+21].Position[m]+2*(CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount].Position[m])+CtlVerts[ctlCount+1].Position[m])/9;
                        if (m < 2) {
                            tempUV[m] = (4*CtlVerts[ctlCount+21].UV[m]+2*(CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount].UV[m])+CtlVerts[ctlCount+1].UV[m])/9;
                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 6){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (8*CtlVerts[ctlCount+1].Position[m]+2*(CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount-20].Position[m])+4*CtlVerts[ctlCount].Position[m]+CtlVerts[ctlCount+21].Position[m]+CtlVerts[ctlCount-21].Position[m])/18;
                        if (m < 2) {
                            tempUV[m] = (8*CtlVerts[ctlCount+1].UV[m]+2*(CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount-20].UV[m])+4*CtlVerts[ctlCount].UV[m]+CtlVerts[ctlCount+21].UV[m]+CtlVerts[ctlCount-21].UV[m])/18;                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 7){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (4*CtlVerts[ctlCount+1].Position[m]+2*(CtlVerts[ctlCount+22].Position[m]+CtlVerts[ctlCount].Position[m])+CtlVerts[ctlCount+21].Position[m])/9;
                        if (m < 2) {
                            tempUV[m] = (4*CtlVerts[ctlCount+1].UV[m]+2*(CtlVerts[ctlCount+22].UV[m]+CtlVerts[ctlCount].UV[m])+CtlVerts[ctlCount+21].UV[m])/9;                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                else if(i == 8){
                    for (int m = 0; m < 3; m++) {
                        tempPos[m] = (4*CtlVerts[ctlCount+22].Position[m]+2*(CtlVerts[ctlCount+1].Position[m]+CtlVerts[ctlCount+21].Position[m])+CtlVerts[ctlCount].Position[m])/9;
                        if (m < 2) {
                            tempUV[m] = (4*CtlVerts[ctlCount+22].UV[m]+2*(CtlVerts[ctlCount+1].UV[m]+CtlVerts[ctlCount+21].UV[m])+CtlVerts[ctlCount].UV[m])/9;                        }
                    }
                    subVerts[subCount].SetPosition(tempPos);
                    subVerts[subCount].SetUV(tempUV);
                }
                subCount++;
            }
            ctlCount++;
        }
    }
}

void saveVerts(char* filename) {
    std::fstream fs;
    fs.open(filename, std::ios::out);
    if (fs.is_open())
    {
        for (int i = 0; i < 441; i++) {
            fs << CtlVerts[i].Position[0] << " ";
            fs << CtlVerts[i].Position[1] << " ";
            fs << CtlVerts[i].Position[2] << " ";
            fs << CtlVerts[i].UV[0] << " ";
            fs << CtlVerts[i].UV[1] << "\n";
        }
    }
    fs.close();
}

void loadVerts(char* filename, Vertex* v) {
    std::fstream fs;
    fs.open(filename, std::ios::in);
    if (fs.is_open()) {
        for (int i = 0; i < 441; i++) {
            fs >> v[i].Position[0];
            fs >> v[i].Position[1];
            fs >> v[i].Position[2];
            fs >> v[i].UV[0];
            fs >> v[i].UV[1];
        }
    }
    fs.close();
    updateControlMesh(false);
}

void animate(char* fileName) {
	if (animation)
		return;

	// load smile file
	Vertex tempVertex[441];
	loadVerts(fileName, tempVertex);

	float tempPos[4] = {1.0};
	// calculate delta
	//for (int i = 5*21; i < 16*21; i++) {
	for (int i = 0; i < 441; i++) {
		for (int j = 0; j < 3; j++)
			tempPos[j] = (tempVertex[i].Position[j] - CtlVerts[i].Position[j]) / animationLimit;
		delta[i].SetPosition(tempPos);
	}

	animationCount = 0;
	animation = true;
}

void addDelta() {
	float tempPos[4] = {1.0};
	//for (int i = 5*21; i < 16*21; i++) {
	for (int i = 0; i < 441; i++) {
		for (int j=0; j<3; j++)
			tempPos[j] = CtlVerts[i].Position[j] + delta[i].Position[j];

		CtlVerts[i].SetPosition(tempPos);
	}
}

int initWindow(void)
{
    // Initialise GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Open a window and create its OpenGL context
    window = glfwCreateWindow(window_width, window_height, "Wang,Zun (6151-0196)", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }
    
    /*
     // Initialize the GUI
     TwInit(TW_OPENGL_CORE, NULL);
     TwWindowSize(window_width, window_height);
     TwBar * GUI = TwNewBar("Picking");
     TwSetParam(GUI, NULL, "refresh", TW_PARAM_CSTRING, 1, "0.1");
     TwAddVarRW(GUI, "Last picked object", TW_TYPE_STDSTRING, &gMessage, NULL);
     */
    
    // Set up inputs
    glfwSetCursorPos(window, window_width / 2, window_height / 2);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);
    
    return 0;
}

void initOpenGL(void)
{
    
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);
    
    // Projection matrix : 45∞ Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    gProjectionMatrix = glm::perspective(45.0f, 1.0f, 0.1f, 100.0f);
    // Or, for an ortho camera :
    //gProjectionMatrix = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.0f, 100.0f); // In world coordinates
    
    // Camera matrix
    cameraCoords = glm::vec3(10.0, 10.0, 10.0f);
    cameraUp=1.0;
    // Init camera angle & radius for later rotation
    CRadius = sqrt(3*(10*10));
    CAngleXZ = atan(1);
    CAngleXY = atan(1/sqrt(2));
    
    gViewMatrix = glm::lookAt(cameraCoords,	// eye
                              glm::vec3(0.0, 0.0, 0.0),	// center
                              glm::vec3(0.0, cameraUp, 0.0));	// up
    
    
    // Create and compile our GLSL program from the shaders
    programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
    pickingProgramID = LoadShaders("Picking.vertexshader", "Picking.fragmentshader");
    
    // Get a handle for our "MVP" uniform
    MatrixID = glGetUniformLocation(programID, "MVP");
    ModelMatrixID = glGetUniformLocation(programID, "M");
    ViewMatrixID = glGetUniformLocation(programID, "V");
    ProjMatrixID = glGetUniformLocation(programID, "P");
    
    PickingMatrixID = glGetUniformLocation(pickingProgramID, "MVP");
    // Get a handle for our "pickingColorID" uniform
    //pickingColorID = glGetUniformLocation(pickingProgramID, "PickingColor");
    
    //Get a handle for our "pickingColorArrayID" uniform
    pickingColorArrayID = glGetUniformLocation(pickingProgramID, "PickingColorArray");
    
    // Get a handle for our "LightPosition" uniform
    LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    
    // Get a handle for our "useLightModel" uniform
    useLightModelID = glGetUniformLocation(programID, "useLightModel");
    useTextureID = glGetUniformLocation(programID, "useTexture");
    TextureID = glGetUniformLocation(programID, "textureSampler");//wangzun codes//
    
    
    createObjects();
}

void createVAOs(Vertex Vertices[], unsigned int Indices[], int ObjectId) {
    
    GLenum ErrorCheckValue = glGetError();
    const size_t VertexSize = sizeof(Vertices[0]);
    const size_t RgbOffset = sizeof(Vertices[0].Position);
    const size_t Normaloffset = sizeof(Vertices[0].Color) + RgbOffset;
    const size_t UVoffset = sizeof(Vertices[0].Normal)+ Normaloffset;//wangzun codes//
    
    // Create Vertex Array Object
    glGenVertexArrays(1, &VertexArrayId[ObjectId]);	//
    glBindVertexArray(VertexArrayId[ObjectId]);		//
    
    // Create Buffer for vertex data
    glGenBuffers(1, &VertexBufferId[ObjectId]);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferId[ObjectId]);
    glBufferData(GL_ARRAY_BUFFER, VertexBufferSize[ObjectId], Vertices, GL_STATIC_DRAW);
    
    // Create Buffer for indices
    if (Indices != NULL) {
        glGenBuffers(1, &IndexBufferId[ObjectId]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferId[ObjectId]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexBufferSize[ObjectId], Indices, GL_STATIC_DRAW);
    }
    
    // Assign vertex attributes
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, VertexSize, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)RgbOffset);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)Normaloffset);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, VertexSize, (GLvoid*)UVoffset);//wangzun codes//
    
    
    glEnableVertexAttribArray(0);	// position
    glEnableVertexAttribArray(1);	// color
    glEnableVertexAttribArray(2);	// normal
    glEnableVertexAttribArray(3);	// UVs //wangzun codes//
    
    
    // Disable our Vertex Buffer Object
    glBindVertexArray(0);
    
    ErrorCheckValue = glGetError();
    if (ErrorCheckValue != GL_NO_ERROR)
    {
        fprintf(
                stderr,
                "ERROR: Could not create a VBO: %s \n",
                gluErrorString(ErrorCheckValue)
                );
    }
}

void cleanup(void)
{
    // Cleanup VBO and shader
    for (int i = 0; i < NumObjects; i++) {
        glDeleteBuffers(1, &VertexBufferId[i]);
        glDeleteBuffers(1, &IndexBufferId[i]);
        glDeleteVertexArrays(1, &VertexArrayId[i]);
    }
    glDeleteProgram(programID);
    glDeleteProgram(pickingProgramID);
    
    // Close OpenGL window and terminate GLFW
    glfwTerminate();
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // ATTN: MODIFY AS APPROPRIATE
    if (action == GLFW_RELEASE) {
        direction = 0;
        holdingShift = false;
    }
    if (action == GLFW_PRESS) {
        switch (key)
        {
                // Direction Keys
            case GLFW_KEY_LEFT:
            case GLFW_KEY_RIGHT:
            case GLFW_KEY_UP:
            case GLFW_KEY_DOWN:
                direction = key;
                break;
                // Keys for activating functions
            case GLFW_KEY_R:
				resetState();
				break;
            case GLFW_KEY_A:
				autoFit();
				break;
            case GLFW_KEY_S:
				saveVerts("cm.p3");
				break;
            case GLFW_KEY_L:
				loadVerts("cm.p3", CtlVerts);
				break;
			case GLFW_KEY_COMMA:
				animationLimit = 120;
				animate(faceSmileName[faceModelID]);
				break;
			case GLFW_KEY_PERIOD:
				// frown
				animationLimit = 120;
				animate(faceFrownName[faceModelID]);
                break;
                // Status changing keys
            case GLFW_KEY_F:
                showFace = (showFace?false:true);
                break;
            case GLFW_KEY_C:
                showControlMesh = (showControlMesh + 1) % 3;
                break;
            case GLFW_KEY_H:
                showHair = (showHair?false:true);
                break;
			case GLFW_KEY_D:
				showSubd = (showSubd ? false : true);
				subdivision();
				subtriangle();
				break;
            case GLFW_KEY_LEFT_SHIFT:
            case GLFW_KEY_RIGHT_SHIFT:
                holdingShift = true;
                break;
                // For choosing which face to show
            case GLFW_KEY_1:
            case GLFW_KEY_2:
                faceModelID = key - GLFW_KEY_1 ;
                resetState();
                break;
            default:
                break;
        }
    }
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        pickObject();
        enableDrag = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        enableDrag = false;
        pickedIndex = -1;
        lastXPos = lastYPos = -1;
    }
}

int main(void)
{
    // initialize window
    int errorCode = initWindow();
    if (errorCode != 0)
        return errorCode;
    
    // initialize OpenGL pipeline
    initOpenGL();
    
    do {
        
        if (direction) moveCamera(glm::pi<float>()/72.0);
        
        if (enableDrag) {
            moveVertex();
        }
        
		if (animation) {
			addDelta();
			animationCount++;
			if (animationCount == animationLimit)
				animation = false;
			updateControlMesh(false);
		}
        
        // DRAWING POINTS
        renderScene();
        
        
    } // Check if the ESC key was pressed or the window was closed
    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0);
    
    cleanup();
    
    return 0;
}
