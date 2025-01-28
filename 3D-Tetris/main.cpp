//
// Author: Ahmet Oguz Akyuz
// 
// This is a sample code that draws a single block piece at the center
// of the window. It does many boilerplate work for you -- but no
// guarantees are given about the optimality and bug-freeness of the
// code. You can use it to get started or you can simply ignore its
// presence. You can modify it in any way that you like.
//
//


#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <GL/glew.h>
//#include <OpenGL/gl3.h>   // The GL Header File
#include <GLFW/glfw3.h> // The GLFW header
#include <glm/glm.hpp> // GL Math library header
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <ft2build.h>
#include FT_FREETYPE_H

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

/*Declerations*/ ////////////////////
void draw3x3x3Cube(glm::vec3 position);


//////////////////////////////////


GLuint gProgram[3];
int gWidth = 600, gHeight = 1000;
GLuint gVertexAttribBuffer, gTextVBO, gIndexBuffer;
GLuint gTex2D;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;
int gTriangleIndexDataSizeInBytes, gLineIndexDataSizeInBytes;

GLint modelingMatrixLoc[2];
GLint viewingMatrixLoc[2];
GLint projectionMatrixLoc[2];
GLint eyePosLoc[2];
GLint lightPosLoc[2];
GLint kdLoc[2];
GLint textProjectionMatrixLoc;

glm::mat4 projectionMatrix;
glm::mat4 textProjectionMatrix; // Add text projection matrix
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix = glm::translate(glm::mat4(1.f), glm::vec3(-0.5, -0.5, -0.5));
glm::vec3 eyePos = glm::vec3(0, 0, 24);
glm::vec3 lightPos = glm::vec3(0, 0, 7);

glm::vec3 kdGround(0.334, 0.288, 0.635); // this is the ground color in the demo
glm::vec3 kdCubes(0.86, 0.11, 0.31);

int activeProgramIndex = 0;

// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

/*Our data*/ ////////////////////////////////////
float initCenterCube = 6.25;
float centerGround = -7.5;

float gameBoundMinX = -4.5;
float gameBoundMaxX = 4.5;
float gameBoundMinY = centerGround + 0.25;
float gameBoundMinZ = -4.5;
float gameBoundMaxZ = 4.5;

struct Cube {
    glm::vec3 position{0,0,0};
    Cube(glm::vec3 position) : position(position) {}
};

std::vector<Cube> allCubes;
Cube activeCube(glm::vec3{0, initCenterCube, 0});
// bool doesActiveExist = true;

float stepInterval = 1.0f;   
float timeAccumulator = 0.0f; 
float stepLength = 1.0f;
float stepLengthSliding = 1.0f;
float lastFrame = 0;
float currentFrame = 0;
float deltaTime = 0;
float totalRotation = 0;

enum class MoveDirection : uint8_t {
    LEFT = 0,
    RIGHT = 1,
    DOWN = 2,
};

bool moveLeft = false;
bool moveRight = false;
bool finishRotation = true;

enum class RotationDirection : uint8_t {
    C_WISE = 0,
    CC_WISE = 1
};

int viewStatus = 0; // FRONT = 0, RIGHT = 1, BACK = 2, LEFT = 3

RotationDirection rotationDirection = RotationDirection::C_WISE;

glm::vec3 eyePosOriginal = eyePos;
glm::vec3 lightPosOriginal = lightPos;

std::vector<int> rowCubeCount {0,0,0,0};

const int maxRowCubeNo = 9;

int point = 0;

std::string lastKeyPressed = "";
bool showKey = false;
std::chrono::time_point<std::chrono::steady_clock> keyPressTime;
const float displayDuration = 0.5f;
bool gameStarted = false;
bool gameOver = false;

/////////////////////////////////////////////////

// For reading GLSL files
bool ReadDataFromFile(
    const string& fileName, ///< [in]  Name of the shader file
    string&       data)     ///< [out] The contents of the file
{
    fstream myfile;

    // Open the input 
    myfile.open(fileName.c_str(), std::ios::in);

    if (myfile.is_open())
    {
        string curLine;

        while (getline(myfile, curLine))
        {
            data += curLine;
            if (!myfile.eof())
            {
                data += "\n";
            }
        }

        myfile.close();
    }
    else
    {
        return false;
    }

    return true;
}

GLuint createVS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &shader, &length);
    glCompileShader(vs);

    char output[1024] = {0};
    glGetShaderInfoLog(vs, 1024, &length, output);
    printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
    string shaderSource;

    string filename(shaderName);
    if (!ReadDataFromFile(filename, shaderSource))
    {
        cout << "Cannot find file name: " + filename << endl;
        exit(-1);
    }

    GLint length = shaderSource.length();
    const GLchar* shader = (const GLchar*) shaderSource.c_str();

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &shader, &length);
    glCompileShader(fs);

    char output[1024] = {0};
    glGetShaderInfoLog(fs, 1024, &length, output);
    printf("FS compile log: %s\n", output);

	return fs;
}

void initFonts(int windowWidth, int windowHeight)
{
    // Set OpenGL options
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(windowWidth), 0.0f, static_cast<GLfloat>(windowHeight));
    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(glGetUniformLocation(gProgram[2], "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // FreeType
    FT_Library ft;
    // All functions return a value different than 0 whenever an error occurred
    if (FT_Init_FreeType(&ft))
    {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }

    // Load font as face
    FT_Face face;
    if (FT_New_Face(ft, "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 0, &face))
    //if (FT_New_Face(ft, "/usr/share/fonts/truetype/gentium-basic/GenBkBasR.ttf", 0, &face)) // you can use different fonts
    {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
                );
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (GLuint) face->glyph->advance.x
        };
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    //
    // Configure VBO for texture quads
    //
    glGenBuffers(1, &gTextVBO);
    glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initShaders()
{
	// Create the programs

    gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
	gProgram[2] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs1 = createVS("vert.glsl"); // for cube shading
    GLuint fs1 = createFS("frag.glsl");

	GLuint vs2 = createVS("vert2.glsl"); // for border shading
	GLuint fs2 = createFS("frag2.glsl");

	GLuint vs3 = createVS("vert_text.glsl");  // for text shading
	GLuint fs3 = createFS("frag_text.glsl");

	// Attach the shaders to the programs

	glAttachShader(gProgram[0], vs1);
	glAttachShader(gProgram[0], fs1);

	glAttachShader(gProgram[1], vs2);
	glAttachShader(gProgram[1], fs2);

	glAttachShader(gProgram[2], vs3);
	glAttachShader(gProgram[2], fs3);

	// Link the programs

    for (int i = 0; i < 3; ++i)
    {
        glLinkProgram(gProgram[i]);
        GLint status;
        glGetProgramiv(gProgram[i], GL_LINK_STATUS, &status);

        if (status != GL_TRUE)
        {
            cout << "Program link failed: " << i << endl;
            exit(-1);
        }
    }


	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 2; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
		lightPosLoc[i] = glGetUniformLocation(gProgram[i], "lightPos");
		kdLoc[i] = glGetUniformLocation(gProgram[i], "kd");

        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(modelingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
        glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
        glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
        glUniform3fv(kdLoc[i], 1, glm::value_ptr(kdCubes));

    }
}

// VBO setup for drawing a cube and its borders
void initVBO()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	assert(glGetError() == GL_NONE);

	glGenBuffers(1, &gVertexAttribBuffer);
	glGenBuffers(1, &gIndexBuffer);

	assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

	glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    GLuint indices[] = {
        0, 1, 2, // front
        3, 0, 2, // front
        4, 7, 6, // back
        5, 4, 6, // back
        0, 3, 4, // left
        3, 7, 4, // left
        2, 1, 5, // right
        6, 2, 5, // right
        3, 2, 7, // top
        2, 6, 7, // top
        0, 4, 1, // bottom
        4, 5, 1  // bottom
    };

    GLuint indicesLines[] = {
        7, 3, 2, 6, // top
        4, 5, 1, 0, // bottom
        2, 1, 5, 6, // right
        5, 4, 7, 6, // back
        0, 1, 2, 3, // front
        0, 3, 7, 4, // left
    };

    GLfloat vertexPos[] = {
        0, 0, 1, // 0: bottom-left-front
        1, 0, 1, // 1: bottom-right-front
        1, 1, 1, // 2: top-right-front
        0, 1, 1, // 3: top-left-front
        0, 0, 0, // 0: bottom-left-back
        1, 0, 0, // 1: bottom-right-back
        1, 1, 0, // 2: top-right-back
        0, 1, 0, // 3: top-left-back
    };

    GLfloat vertexNor[] = {
         1.0,  1.0,  1.0, // 0: unused
         0.0, -1.0,  0.0, // 1: bottom
         0.0,  0.0,  1.0, // 2: front
         1.0,  1.0,  1.0, // 3: unused
        -1.0,  0.0,  0.0, // 4: left
         1.0,  0.0,  0.0, // 5: right
         0.0,  0.0, -1.0, // 6: back 
         0.0,  1.0,  0.0, // 7: top
    };

	gVertexDataSizeInBytes = sizeof(vertexPos);
	gNormalDataSizeInBytes = sizeof(vertexNor);
    gTriangleIndexDataSizeInBytes = sizeof(indices);
    gLineIndexDataSizeInBytes = sizeof(indicesLines);
    int allIndexSize = gTriangleIndexDataSizeInBytes + gLineIndexDataSizeInBytes;

	glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexPos);
	glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, vertexNor);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, allIndexSize, 0, GL_STATIC_DRAW);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, gTriangleIndexDataSizeInBytes, indices);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, gTriangleIndexDataSizeInBytes, gLineIndexDataSizeInBytes, indicesLines);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init() 
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // polygon offset is used to prevent z-fighting between the cube and its borders
    glPolygonOffset(0.5, 0.5);
    glEnable(GL_POLYGON_OFFSET_FILL);

    initShaders();
    initVBO();
    initFonts(gWidth, gHeight);
}

void initCube() {
    activeCube.position = glm::vec3{0, initCenterCube, 0};
}

void draw3x3x3Cube(glm::vec3 position) {
    glLineWidth(3);
    for(int z = -1; z < 2; ++z) {
        for(int y = -1; y < 2; ++y) {
            for(int x = -1; x < 2; ++x) {
                glUseProgram(gProgram[0]);
	            modelingMatrix = glm::translate(glm::mat4(1.f), glm::vec3(position.x + x - 0.5, position.y + y - 0.5, position.z + z - 0.5));
                GLuint modelLoc = glGetUniformLocation(gProgram[0], "modelingMatrix");
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));

                glDrawElements(GL_TRIANGLES, gTriangleIndexDataSizeInBytes/4, GL_UNSIGNED_INT, 0);

                glUseProgram(gProgram[1]);
                for (int i = 0; i < 6; ++i)
                {
                    GLuint modelLocE = glGetUniformLocation(gProgram[1], "modelingMatrix");
                    glUniformMatrix4fv(modelLocE, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
                    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, BUFFER_OFFSET(gTriangleIndexDataSizeInBytes + i * 4 * sizeof(GLuint)));
                }
            }
        }
    }
}

void drawGround(glm::vec3 position) {
    // set the ground color
    glUseProgram(gProgram[0]);
    GLuint kdLoc = glGetUniformLocation(gProgram[0], "kd");
    glUniform3fv(kdLoc, 1, glm::value_ptr(kdGround)); // Send kdGround to the shader

    glLineWidth(3);
    for(int z = -4; z < 5; ++z) {
        for(int x = -4; x < 5; ++x) {
            glUseProgram(gProgram[0]);

            modelingMatrix = glm::scale(glm::translate(glm::mat4(1.f), glm::vec3(position.x + x - 0.5, position.y - 0.25, position.z + z - 0.5)), 
                                glm::vec3(1, 0.5, 1));

            GLuint modelLoc = glGetUniformLocation(gProgram[0], "modelingMatrix");
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
            glDrawElements(GL_TRIANGLES, gTriangleIndexDataSizeInBytes/4, GL_UNSIGNED_INT, 0);

            glUseProgram(gProgram[1]);
            for (int i = 0; i < 6; ++i)
            {
                GLuint modelLocE = glGetUniformLocation(gProgram[1], "modelingMatrix");
                glUniformMatrix4fv(modelLocE, 1, GL_FALSE, glm::value_ptr(modelingMatrix));
                glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, BUFFER_OFFSET(gTriangleIndexDataSizeInBytes + i * 4 * sizeof(GLuint)));
            }
        }
    }

    // reset the color to cube color
    glUseProgram(gProgram[0]);
    glUniform3fv(kdLoc, 1, glm::value_ptr(kdCubes));
}

void drawAllCubes() {
    draw3x3x3Cube(activeCube.position);
    for(Cube c : allCubes) {
        draw3x3x3Cube(c.position);
    }
}


void renderText(const std::string& text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
{
    // Activate corresponding render state	
    glUseProgram(gProgram[2]);
    glUniform3f(glGetUniformLocation(gProgram[2], "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) 
    {
        Character ch = Characters[*c];

        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;

        // Update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos,     ypos + h,   0.0, 0.0 },            
            { xpos,     ypos,       0.0, 1.0 },
            { xpos + w, ypos,       1.0, 1.0 },

            { xpos,     ypos + h,   0.0, 0.0 },
            { xpos + w, ypos,       1.0, 1.0 },
            { xpos + w, ypos + h,   1.0, 0.0 }           
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, gTextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData

        //glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)

        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    drawGround(glm::vec3(0, centerGround, 0));
    drawAllCubes();

    // RENDER TEXTS
    std::string face;
    switch(viewStatus) {
    case 0:
        face = "Front";
        break;
    case 1:
        face = "Right";
        break;
    case 2:
        face = "Back";
        break;
    case 3:
        face = "Left";
        break;
    }

    renderText(face, 20, gHeight-25, 0.6, glm::vec3(1, 1, 0));
    renderText("Points: " + std::to_string(point), gWidth-160, gHeight-25, 0.6, glm::vec3(1, 1, 0));
    if(showKey) {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - keyPressTime).count();

        if (elapsed > displayDuration) {
            showKey = false;
        } else {
            if (face == "Left")
                renderText(lastKeyPressed, 20+15, gHeight - 60, 0.6f, glm::vec3(1.0f, 0.0f, 0.0f));
            else
                renderText(lastKeyPressed, 20+22, gHeight - 60, 0.6f, glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }

    if(gameOver) {
        renderText("Game Over!", gWidth/2-250, gHeight/2, 2.0f, glm::vec3(1, 1, 0));
    }

    assert(glGetError() == GL_NO_ERROR);
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;

    gWidth = w;
    gHeight = h;

    glViewport(0, 0, w, h);

	// Use perspective projection

	float fovyRad = (float) (45.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, gWidth / (float) gHeight, 1.0f, 100.0f);

    // always look toward (0, 0, 0)
	viewingMatrix = glm::lookAt(eyePos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

    for (int i = 0; i < 2; ++i)
    {
        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(projectionMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
    }

    textProjectionMatrix = glm::ortho(0.0f, (float)gWidth, 0.0f, (float)gHeight);

    glUseProgram(gProgram[2]);
    glUniformMatrix4fv(textProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr(textProjectionMatrix));
}

void restartGame() {
    gameStarted = false;
    gameOver = false;
    allCubes.clear();
    point = 0;
    timeAccumulator = 0;
    currentFrame = 0;
    lastFrame = 0;
    deltaTime = 0;
    stepInterval = 1.0f;
    rowCubeCount = std::vector<int> {0,0,0,0};
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (action == GLFW_PRESS) {
        showKey = true;
        keyPressTime = std::chrono::steady_clock::now();
    }

    if(key == GLFW_KEY_A && action == GLFW_PRESS) {
        moveLeft = true;
        lastKeyPressed = "A";
    }

    if(key == GLFW_KEY_D && action == GLFW_PRESS) {
        moveRight = true;
        lastKeyPressed = "D";
    }

    if(key == GLFW_KEY_W && action == GLFW_PRESS) {
        if (stepInterval == 1) stepLengthSliding = 0;
        if (stepInterval != 1) stepInterval *= 2;
        lastKeyPressed = "W";
    }

    if(key == GLFW_KEY_S && action == GLFW_PRESS) {
        lastKeyPressed = "S";
        if(!gameStarted) {
            gameStarted = true;
            return;
        }

        if(gameOver) {
            restartGame();
            return;
        }
        if (stepLengthSliding == 0) stepLengthSliding = 1;
        else if (stepInterval != 0.25) stepInterval /= 2;
    }

    if(key == GLFW_KEY_K && action == GLFW_PRESS) {
        //clockwise
        lastKeyPressed = "K";
        if(finishRotation) {
            rotationDirection = RotationDirection::C_WISE;
            finishRotation = false;
            viewStatus = ((viewStatus + 1) % 4 + 4) % 4;
        }
    }

    if(key == GLFW_KEY_H && action == GLFW_PRESS) {
        //counter-clockwise
        lastKeyPressed = "H";
        if(finishRotation) {
            rotationDirection = RotationDirection::CC_WISE;
            finishRotation = false;
            viewStatus = ((viewStatus - 1) % 4 + 4) % 4;
        }
    }


}

glm::vec3 calculateNextPosition(MoveDirection moveDir) {
    glm::vec3 new_position = activeCube.position;
    switch(viewStatus) {
    case 0: // FRONT
        if(moveDir == MoveDirection::LEFT) {
            new_position.x -= stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            new_position.x += stepLength;
        }
        break;
    case 1: // RIGHT
        if(moveDir == MoveDirection::LEFT) {
            new_position.z += stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            new_position.z -= stepLength;
        }
        break;
    case 2: // BACK
        if(moveDir == MoveDirection::LEFT) {
            new_position.x += stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            new_position.x -= stepLength;
        }
        break;
    case 3: // LEFT
        if(moveDir == MoveDirection::LEFT) {
            new_position.z -= stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            new_position.z += stepLength;
        }
        break;
    }

    if(moveDir == MoveDirection::DOWN) {
        new_position.y -= stepLengthSliding; //Sliding here
    }

    return new_position;
}

bool isThereIntersection(MoveDirection moveDir) {
    glm::vec3 new_position = calculateNextPosition(moveDir);

    float active_x_min = new_position.x - 1.5;
    float active_x_max = new_position.x + 1.5;
    float active_y_min = new_position.y - 1.5;
    float active_y_max = new_position.y + 1.5;
    float active_z_min = new_position.z - 1.5;
    float active_z_max = new_position.z + 1.5;
    
    for(Cube targetCube : allCubes) {
        float target_x_min = targetCube.position.x - 1.5;
        float target_x_max = targetCube.position.x + 1.5;
        float target_y_min = targetCube.position.y - 1.5;
        float target_y_max = targetCube.position.y + 1.5;
        float target_z_min = targetCube.position.z - 1.5;
        float target_z_max = targetCube.position.z + 1.5;
    
        bool overlap_x = (active_x_max > target_x_min && active_x_min < target_x_max);
        bool overlap_y = (active_y_max > target_y_min && active_y_min < target_y_max);
        bool overlap_z = (active_z_max > target_z_min && active_z_min < target_z_max);

        // If there's overlap in all three dimensions, there's an intersection
        if (overlap_x && overlap_y && overlap_z) {
            return true; // Intersection found
        }
        
    }
    return false;
}

bool doesExceedGameBoundary(MoveDirection moveDir) {
    glm::vec3 new_position = calculateNextPosition(moveDir);

    float active_x_min = new_position.x - 1.5;
    float active_x_max = new_position.x + 1.5;
    float active_y_min = new_position.y - 1.5;
    float active_y_max = new_position.y + 1.5;
    float active_z_min = new_position.z - 1.5;
    float active_z_max = new_position.z + 1.5;

    bool exceed_X = active_x_max > gameBoundMaxX || active_x_min < gameBoundMinX;
    bool exceed_Y = active_y_min < gameBoundMinY;
    bool exceed_Z = active_z_max > gameBoundMaxZ || active_z_min < gameBoundMinZ;


    return (exceed_X || exceed_Y || exceed_Z);
}

void deleteRow(float y_value, int rowNo) {
    size_t allCubesSize = allCubes.size();
    std::vector<Cube> keptCubes;
    for (int i = 0 ; i < allCubesSize; i++){
        if (allCubes[i].position.y != y_value) {

            Cube c {allCubes[i].position};
            if(c.position.y > y_value) {
                c.position.y -= 3;
            }
            keptCubes.push_back(c);
        }
    }
    
    rowCubeCount.erase(rowCubeCount.begin() + rowNo);
    rowCubeCount.push_back(0);

    allCubes = keptCubes;
    point += 243;
}

void slideActiveCube() {
    if(!gameStarted || gameOver) {
        return;
    }
    // Accumulate the time
    timeAccumulator += deltaTime; // change

    if (timeAccumulator >= stepInterval) {
        // if(activeCube.position.y <= centerGround + 1.75) { // touches ground
        if(doesExceedGameBoundary(MoveDirection::DOWN)) { // touches ground
            allCubes.push_back(activeCube);
            
            if(++rowCubeCount[0] >= maxRowCubeNo) {
                deleteRow(activeCube.position.y, 0);
            }
            initCube();
        }

        else if(isThereIntersection(MoveDirection::DOWN)) { // touches another cube
            allCubes.push_back(activeCube);
            
            int rowNo = (activeCube.position.y - centerGround - 1.75)/3; 

            if(rowNo == 4) {
                gameOver = true;
                return;
            }

            if(++rowCubeCount[rowNo] >= maxRowCubeNo) {
                deleteRow(activeCube.position.y, rowNo);
            }

            initCube();
        }

        else { // it can move
            activeCube.position.y -= stepLengthSliding; //Sliding here
        }
        
        timeAccumulator = 0.0f;
    }
}

void moveActiveCube(MoveDirection moveDir) {
    switch(viewStatus) {
    case 0: // FRONT
        if(moveDir == MoveDirection::LEFT) {
            activeCube.position.x -= stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            activeCube.position.x += stepLength;
        }
        break;
    case 1: // RIGHT
        if(moveDir == MoveDirection::LEFT) {
            activeCube.position.z += stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            activeCube.position.z -= stepLength;
        }
        break;
    case 2: // BACK
        if(moveDir == MoveDirection::LEFT) {
            activeCube.position.x += stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            activeCube.position.x -= stepLength;
        }
        break;
    case 3: // LEFT
        if(moveDir == MoveDirection::LEFT) {
            activeCube.position.z -= stepLength;
        }
        else if(moveDir == MoveDirection::RIGHT) {
            activeCube.position.z += stepLength;
        }
        break;
    }
}

void checkMovements() {
    if(gameOver) {
        return;
    }

    if(moveLeft == true) {
        moveLeft = false;
        if(!doesExceedGameBoundary(MoveDirection::LEFT) && !isThereIntersection(MoveDirection::LEFT)) {
            moveActiveCube(MoveDirection::LEFT);
        }
    }

    if(moveRight == true) {
        moveRight = false;
        if(!doesExceedGameBoundary(MoveDirection::RIGHT) && !isThereIntersection(MoveDirection::RIGHT)) {
            moveActiveCube(MoveDirection::RIGHT);
        }
    }
}

void rotateGame() {
    // MODIFY THIS FOR CONTINUOUS ROTATION
    const float rotationSpeed = 270.0f;
    float rotationAngle;
    float rotationStep = rotationSpeed * deltaTime;

    if(rotationDirection == RotationDirection::C_WISE) {
        totalRotation += rotationStep;
        rotationAngle = 90.0f;
    }
    else if(rotationDirection == RotationDirection::CC_WISE) {
        rotationStep = -rotationStep;
        totalRotation += rotationStep;
        rotationAngle = -90.0f;
        // cout << "totalRotation " << totalRotation << endl;
    }

    if(std::abs(totalRotation) >= std::abs(rotationAngle)) {
        finishRotation = true;

        if(rotationDirection == RotationDirection::C_WISE) { 
            totalRotation = rotationAngle;
        }
        else if(rotationDirection == RotationDirection::CC_WISE) {
            totalRotation = rotationAngle;
        }
    }
    
    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(totalRotation), glm::vec3(0.0f, 1.0f, 0.0f));

    eyePos = glm::vec3(rotationMatrix * glm::vec4(eyePosOriginal, 1.0f));
    lightPos = glm::vec3(rotationMatrix * glm::vec4(lightPosOriginal, 1.0f));

    viewingMatrix = glm::lookAt(eyePos, glm::vec3(0,0,0), glm::vec3(0,1,0));
    for(int i = 0; i < 2; i++) {
        glUseProgram(gProgram[i]);
        glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
        glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
        glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
    }

    if(finishRotation) {
        totalRotation = 0;
        eyePosOriginal = eyePos;
        lightPosOriginal = lightPos;
    }
}

void checkRotation() {
    if(!finishRotation) {
        rotateGame();
    }
}

void setFrameTimes() {
    lastFrame = currentFrame;
    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {   
        checkRotation();
        checkMovements();
        slideActiveCube();
        setFrameTimes();
        display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    //Getting the user's current window size
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode)
    {
        gWidth = mode->width;
        gHeight = mode->height;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(gWidth, gHeight, "tetrisGL", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        exit(-1);
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Initialize GLEW to setup the OpenGL Function pointers
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    char rendererInfo[512] = {0};
    strcpy(rendererInfo, (const char*) glGetString(GL_RENDERER));
    strcat(rendererInfo, " - ");
    strcat(rendererInfo, (const char*) glGetString(GL_VERSION));
    glfwSetWindowTitle(window, rendererInfo);

    init();

    glfwSetKeyCallback(window, keyboard);
    glfwSetWindowSizeCallback(window, reshape);
    reshape(window, gWidth, gHeight); // need to call this once ourselves


    mainLoop(window); // this does not return unless the window is closed

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
