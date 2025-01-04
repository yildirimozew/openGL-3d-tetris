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
#include <random>
#define BUFFER_OFFSET(i) ((char*)NULL + (i))

using namespace std;

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

glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix = glm::translate(glm::mat4(1.f), glm::vec3(-0.5, -0.5, -0.5));
glm::vec3 eyePos = glm::vec3(0, 0, 24);
glm::vec3 lightPos = glm::vec3(0, 0, 7);





std::vector<std::vector<glm::vec3>>  movingObjectList;
std::vector<glm::vec3> movingObject;
glm::vec3 movingCubePos = glm::vec3(0, 0, 1);




bool isMoving = true;
glm::int16 speed = 1;
int direction = 1;
float animationTime = 0.5f; 
float elapsedTime = 0.0f;   
bool isAnimating = false;   
glm::mat4 startViewingMatrix; 
glm::mat4 targetViewingMatrix; 
glm::mat4 rotationMatrix;
float startAngle = 0.0f;    
float targetAngle = 90.0f;   
int look_direction = 0;
//glm::vec3 kdGround(0.334, 0.288, 0.635); // this is the ground color in the demo
glm::vec3 kdCubes(0.86, 0.11, 0.31);
std::vector<glm::vec3> settledCubes;
int activeProgramIndex = 0;
std::map<int,int> counter;
string direction_text[4] = {"Front","Right","Back","Left"};
int score;
bool justStarted = true;



//Start random generatr
std::random_device rd;
std::mt19937 gen(rd()); 
std::uniform_int_distribution<> dis;





// Holds all state information relevant to a character as loaded using FreeType
struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};
std::map<GLchar, Character> Characters;

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

void drawCube()
{
	glUseProgram(gProgram[0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void drawCubeEdges()
{
    glLineWidth(3);

	glUseProgram(gProgram[1]);

    for (int i = 0; i < 6; ++i)
    {
	    glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT, BUFFER_OFFSET(gTriangleIndexDataSizeInBytes + i * 4 * sizeof(GLuint)));
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

bool does_hit(const glm::vec3& pos,const glm::vec3& tmp_movingCubePos){
    for(const auto& offset: movingObject){
        glm::vec3 cubePos = tmp_movingCubePos + offset;
        if (glm::distance(cubePos, pos) < 1) {
            return true;
        }
    }
    return false;
}
bool out_of_bounds(const glm::vec3& tmp_movingCubePos){
    for(const auto& offset: movingObject){
        glm::vec3 cubePos = tmp_movingCubePos + offset;
        if (cubePos.x < -5 || cubePos.x > 4 || cubePos.z < -4 || cubePos.z > 5)
            return true;
    }
    return false;
}

void display()
{
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::vec3 floorPos = glm::vec3(0, -1, 0);
    glm::mat4 movingCubeModelingMatrix = glm::translate(glm::mat4(1), movingCubePos);
    static 
    glm::mat4 floorModelingMatrix = glm::translate(glm::mat4(1), floorPos);
    
    for (const auto& pos : settledCubes) {
        glm::mat4 settledModelingMatrix = glm::translate(glm::mat4(1), pos);
        for (int i = 0; i < 2; i++) {
            glUseProgram(gProgram[i]);
            glUniformMatrix4fv(modelingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(settledModelingMatrix));
            glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
            glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
            glUniform3fv(kdLoc[i], 1, glm::value_ptr(kdCubes));
        }
        drawCube();
        drawCubeEdges();
    }

    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    if (currentTime - lastTime >= 0.3 && isMoving) {
        movingCubePos.y -= speed;
        lastTime = currentTime;
    }

    if(isMoving){
        for(int i = 0; i < 2; i++){
        for (const auto& offset: movingObject){
            glm::mat4 cubeModelingMatrix = glm::translate(movingCubeModelingMatrix, offset);
            glUseProgram(gProgram[i]);
            glUniformMatrix4fv(modelingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(cubeModelingMatrix));
            glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
            glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
            glUniform3fv(kdLoc[i], 1, glm::value_ptr(kdCubes));
            drawCube();
            drawCubeEdges();
        }
    }
    }

    for(float k = -4.5; k < 4; k++){
        for(float j = -4.5; j < 4; j++){
            glm::vec3 cubePos = glm::vec3(j, -8, k + 1);
            glm::mat4 modelingMatrix = glm::translate(glm::mat4(1), cubePos);
            for(int i = 0; i < 2; i++){
                glUseProgram(gProgram[i]);
                glUniformMatrix4fv(modelingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
                glUniform3fv(eyePosLoc[i], 1, glm::value_ptr(eyePos));
                glUniform3fv(lightPosLoc[i], 1, glm::value_ptr(lightPos));
                glUniform3fv(kdLoc[i], 1, glm::value_ptr(kdCubes));
            }
            drawCube();
            drawCubeEdges();
        }
    }

    if (isMoving) {
        for (const auto& pos : settledCubes) {
            for (const auto& offset: movingObject) {
                glm::vec3 cubePos = movingCubePos + offset;
                if (glm::distance(cubePos, pos) < 1) {
                    for (const auto& offset2: movingObject) {
                        settledCubes.push_back(movingCubePos + offset2 + glm::vec3(0,1,0));
                        counter[int(movingCubePos.y + offset2.y)]++;
                    }
                    movingCubePos.y = 6;
                    movingCubePos.x = 0;
                    movingCubePos.z = 0;
                    int index = dis(gen);
                    std::cout << index << std::endl;
                    movingObject = movingObjectList[index];
                    break;
                }
            }
        }
    }

    for(auto& pos : settledCubes){
        if(int(pos.y) == 7){
            isMoving = false;
        }
    }


    if (isMoving && movingCubePos.y < -5) {
        for(const auto& offset: movingObject){
            settledCubes.push_back(movingCubePos + offset);
            counter[int(movingCubePos.y + offset.y)]++;
        }
        movingCubePos.y = 6;
        int index = dis(gen);
        std::cout << index << std::endl;
        movingObject = movingObjectList[index];
        }

        if (isAnimating) {
        static double lastTime2 = glfwGetTime();
        if(justStarted) {
            justStarted = false;
            lastTime2 = glfwGetTime();
        }
        double currentTime2 = glfwGetTime();
        float deltaTime2 = static_cast<float>(currentTime2 - lastTime2);
        lastTime2 = currentTime2;

        elapsedTime += deltaTime2;

        if (elapsedTime < animationTime) {
            // Calculate the current angle based on elapsed time
            float t = elapsedTime / animationTime;
            float currentAngle = glm::radians(startAngle + t * (targetAngle - startAngle));

            // Create an incremental rotation matrix
            glm::mat4 incrementalRotation = glm::rotate(glm::mat4(1.0f), currentAngle, glm::vec3(0, 1, 0));

            // Combine the starting viewing matrix with the incremental rotation
            viewingMatrix = startViewingMatrix * incrementalRotation;

            // Update the shader with the interpolated matrix
            for (int i = 0; i < 2; ++i) {
            glUseProgram(gProgram[i]);
            glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
            }
        } else {
            // End the animation
            isAnimating = false;
            viewingMatrix = startViewingMatrix * rotationMatrix;

            // Final update to the shader
            for (int i = 0; i < 2; ++i) {
            glUseProgram(gProgram[i]);
            glUniformMatrix4fv(viewingMatrixLoc[i], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
            }
        }
        }
    if(isMoving == false){
        renderText("Game Over", 0, 500, 1, glm::vec3(1, 1, 0));
    }
    renderText("Score: " + std::to_string(score/3), 0, 950, 0.75, glm::vec3(1, 1, 0));
    renderText(direction_text[look_direction], 0, 920, 0.75, glm::vec3(1, 1, 0));
    assert(glGetError() == GL_NO_ERROR);
}

void reshape(GLFWwindow* window, int w, int h)
{
    w = w < 1 ? 1 : w;
    h = h < 1 ? 1 : h;
    look_direction = 0;
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
}

void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if ((key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    int base_index = look_direction * 4;
    float min_val = -3, max_val = 3;
    float z_min_val = -2, z_max_val = 4;
    glm::vec3 tmp_movingCubePos = movingCubePos;
    if(key == GLFW_KEY_A && action == GLFW_PRESS){
        switch(look_direction){
            case 0:
                tmp_movingCubePos.x = movingCubePos.x-1; break;
            case 1:
                tmp_movingCubePos.z = movingCubePos.z+1; break; 
            case 2:
                tmp_movingCubePos.x = movingCubePos.x+1; break;
            default:
                tmp_movingCubePos.z = movingCubePos.z-1; break;
        }
        
    }
    if(key == GLFW_KEY_D && action == GLFW_PRESS){
        switch(look_direction){
            case 0:
                tmp_movingCubePos.x = movingCubePos.x+1; break;
            case 1:
                tmp_movingCubePos.z = movingCubePos.z-1; break;
            case 2:
                tmp_movingCubePos.x = movingCubePos.x-1; break;
            default:
                tmp_movingCubePos.z = movingCubePos.z+1; break;
        }
    }
    bool not_hit = true;
    bool is_out_of_bounds = out_of_bounds(tmp_movingCubePos);
    if (!is_out_of_bounds){
        for(const auto& pos : settledCubes){
            if(does_hit(pos,tmp_movingCubePos)){
                not_hit = false;
                break;
            }
        }
        if(not_hit){
            movingCubePos = tmp_movingCubePos;
        }
    }
    if(key == GLFW_KEY_W && action == GLFW_PRESS){
        if(speed == 1){
            speed = 0;
        }
    }
    if(key == GLFW_KEY_S && action == GLFW_PRESS){
        if(speed == 0){
            speed = 1;
        }
    }

    // Key event to start the animation
    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        if(isAnimating){
            return;
        }
        isAnimating = true;
        elapsedTime = 0.0f;
        look_direction = (look_direction + 3) % 4; 
        // Save the current viewing matrix
        startViewingMatrix = viewingMatrix;

        // Precompute the final rotation matrix for reference
        rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
        targetAngle = 90.0f;
        justStarted = true;
    }

    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        if(isAnimating){
            return;
        }
        isAnimating = true;
        elapsedTime = 0.0f;
        look_direction = (look_direction + 1) % 4; 

        startViewingMatrix = viewingMatrix;

        rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        targetAngle = -90.0f;
        justStarted = true;
    }
    //std::cout<<"look direction: "<<look_direction<<std::endl;
    // Update animation in the render loop

}

void destroy_cubes(){
    for(int i = -10 ; i < 10; i++){
        if(counter[i] == 81){
            //erase the cubes
            for(int j = 0; j < settledCubes.size(); j++){
                if(int(settledCubes[j].y) == i){
                    settledCubes.erase(settledCubes.begin() + j);
                    j--;
                }
            }
            //move the cubes
            for(int j = 0; j < settledCubes.size(); j++){
                if(int(settledCubes[j].y) > i){
                    settledCubes[j].y -= 1;
                }
            }
            //update the counter
            for(int j = i; j < 10; j++){
                counter[j] = counter[j+1];
            }
            counter[9] = 0;
            i--;
            score++;
        }
    }
}


void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window))
    {
        display();
        destroy_cubes();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

int main(int argc, char** argv)   // Create Main Function For Bringing It All Together
{
    for(int i = -10 ; i < 10; i++){
        counter[i] = 0;
    }
    GLFWwindow* window;
    if (!glfwInit())
    {
        exit(-1);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    std::vector<glm::vec3> cube,double_cube;
    for(float x = -1.5; x <=1 ; x++)
        for(float y = -1; y <=1 ; y++)
            for(float z = -1.5; z <=1 ; z++)
                cube.push_back(glm::vec3(x,y,z));
    movingObjectList.push_back(cube);
    for(float x = -2.5; x <=3 ; x++)
        for(float y = -1; y <=1 ; y++)
            for(float z = -1.5; z <=1 ; z++)
                double_cube.push_back(glm::vec3(x,y,z));
    movingObjectList.push_back(double_cube);
    double_cube.clear();
    for(float x = -1.5; x <=1 ; x++)
        for(int y = -1; y <=4 ; y++)
            for(float z = -1.5; z <=1 ; z++)
                double_cube.push_back(glm::vec3(x,y,z));
    movingObjectList.push_back(double_cube);
    double_cube.clear();
    for(float x = -1.5; x <=1 ; x++)
        for(int y = -1; y <=1 ; y++)
            for(int z = -2.5; z <=3 ; z++)
                double_cube.push_back(glm::vec3(x,y,z));
    movingObjectList.push_back(double_cube);
    double_cube.clear();

    std::uniform_int_distribution<> tmp_dis(0,movingObjectList.size()-1);
    dis = tmp_dis;
    movingObject = movingObjectList[dis(gen)];


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
    mainLoop(window); // this does not return unless the window is closeds

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
