#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <map>
#include <string>
#include <stdlib.h>
#include <ctime>

#include <Windows.h>
#include <Mmsystem.h>
// game starts in time
#define START_COUNTDOWN 2
#define GAME_COUNTDOWN 60


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
GLboolean CheckCollision(int objNum);
GLboolean CheckExitCollision();
void check_camera_direction();
unsigned int loadTexture(char const * path);
void restartGame();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

/// Holds all state information relevant to a character as loaded using FreeType
struct Character {
	GLuint TextureID;   // ID handle of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

// camera
Camera camera(glm::vec3(0.0f, -4.5f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// translation
glm::vec3 lightPos(5.0f, 2.0f, 0.0f);
glm::vec3 ceilPos(0.0f, 5.5f, 0.0f);
glm::vec3 floorPos(0.0f, -5.5f, 0.0f);
glm::vec3 wallPos[] = {
	glm::vec3(-5.5f, 0.0f, 0.0f),	// left
	glm::vec3(5.5f, 0.0f, 0.0f),	// right
	glm::vec3(0.0f, 0.0f, -5.5f),	// top
	glm::vec3(0.0f, 0.0f, 5.5f),	// dowm
};
glm::vec3 obstructPos[] = {
	glm::vec3(-1.5f, -1.0f, -3.5f),
	glm::vec3(1.5f, -1.0f, -3.5f),
	glm::vec3(-1.5f, -1.0f, -1.5f),
	glm::vec3(1.5f, -1.0f, -1.5f),

	glm::vec3(-1.5f, -1.0f, 1.5f),
	glm::vec3(1.5f, -1.0f, 1.5f),
	glm::vec3(-1.5f, -1.0f, 3.5f),
	glm::vec3(1.5f, -1.0f, 3.5f),

	glm::vec3(-3.5f, -1.0f, -0.5f),
	glm::vec3(3.5f, -1.0f, 0.5f),
	glm::vec3(3.5f, -1.0f, -4.5f),

	glm::vec3(0.0f, -1.0f, -2.5f),
	glm::vec3(0.0f, -1.0f, 2.5f),
};
glm::vec3 exitPos(-5.0f, -3.0f, -3.0f);


// scaling
glm::vec3 ceilScale(10.0f, 1.0f, 10.0f);
glm::vec3 floorScale(10.0f, 1.0f, 10.0f);
glm::vec3 wallScale[] = {
	glm::vec3(1.0f, 10.0f, 10.0f),	// left
	glm::vec3(1.0f, 10.0f, 10.0f),	// right
	glm::vec3(10.0f, 10.0f, 1.0f),	// top
	glm::vec3(10.0f, 10.0f, 1.0f),	// dowm
};
glm::vec3 obstructScale[] = {
	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),

	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),

	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),
	glm::vec3(1.0f, 8.0f, 1.0f),

	glm::vec3(6.0f, 8.0f, 1.0f),
	glm::vec3(6.0f, 8.0f, 1.0f),
};
glm::vec3 exitScale(0.5f, 1.0f, 0.5f);
glm::vec3 exitRotate(1.0f, 0.0f, 0.0f);



// camera movement direction
float lastCameraX = 0.0f;
float lastCameraZ = 0.0f;
bool cameraRtoL = true;
bool cameraBtoF = true;

// collision
glm::vec4 BR(0.5, -0.5, 0.5, 1.0);
glm::vec4 UL(-0.5, -0.5, -0.5, 1.0);
glm::vec4 cubeCollisionVertexBR;
glm::vec4 cubeCollisionVertexUL;
int objectNum = -1;

// exit collision
glm::vec4 exitCollisionVertexBR;
glm::vec4 exitCollisionVertexUL;
bool isFirstTimeExitCollision = true;

// clock
clock_t accomplish_clock;
clock_t game_start_clock;

// Phong spec
glm::vec3 materialAmbient(1.0f, 1.0f, 1.0f);
glm::vec3 materialDiffuse(1.0f, 1.0f, 1.0f);
glm::vec3 materialSpecular(1.0f, 1.0f, 1.0f);
GLfloat shininess = 32.0f;
glm::vec3 lightAmbient(0.2f, 0.2f, 0.2f);
glm::vec3 lightDiffuse(0.9f, 0.9f, 0.9f);
glm::vec3 lightSpecular(0.1f, 0.1f, 0.1f);
GLfloat lightCutOff = glm::cos(glm::radians(12.5f));
GLfloat lightOuterCutOff = glm::cos(glm::radians(17.5f));
GLfloat lightConstant = 1.0f;
GLfloat lightLinear = 0.09f;
GLfloat lightQuadratic = 0.032f;

// check game start flag
bool gameStartFlag = false;


int main()
{

	

    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "FinalProject", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
	// horizontal vertical
	glfwSetWindowPos(window, 700, 200);

  

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader obstructShader("obstruct.vs", "obstruct.fs");
    Shader lampShader("lamp.vs", "lamp.fs");
	Shader floorShader("floor.vs", "floor.fs");
	Shader ceilShader("ceil.vs", "ceil.fs");
	Shader exitShader("exit.vs", "exit.fs");
	Shader wallShader("wall.vs", "wall.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};
	
	// load texture
	unsigned int floorTexture = loadTexture("Texture/floor.bmp");
	unsigned int wallTexture = loadTexture("Texture/wall.jpg");
	unsigned int ceilTexture = loadTexture("Texture/ceiling.jpg");
	unsigned int obstructTexture = loadTexture("Texture/obstruct.jpg");
	unsigned int exitTexture = loadTexture("Texture/exit.jpg");

	// VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// cubeVAO -> obstacle
	unsigned int cubeVAO;
	glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


    // lightVAO
    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


	// floorVAO
	unsigned int floorVAO;
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);


	// ceilVAO
	unsigned int ceilVAO;
	glGenVertexArrays(1, &ceilVAO);
	glBindVertexArray(ceilVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// exitVAO
	unsigned int exitVAO;
	glGenVertexArrays(1, &exitVAO);
	glBindVertexArray(exitVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// wallVAO
	unsigned int wallVAO;
	glGenVertexArrays(1, &wallVAO);
	glBindVertexArray(wallVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	
	// set texture to shader only once
	wallShader.use();
	//wallShader.setInt("wall_texture", 0);
	wallShader.setInt("material.diffuse", 0);
	floorShader.use();
	//floorShader.setInt("floor_texture", 1);
	floorShader.setInt("material.diffuse", 1);
	ceilShader.use();
	//ceilShader.setInt("ceil_texture", 2);
	ceilShader.setInt("material.diffuse", 2);
	obstructShader.use();
	//obstructShader.setInt("obstruct_texture", 3);
	obstructShader.setInt("material.diffuse", 3);
	exitShader.use();
	exitShader.setInt("exit_texture", 4);


	// initial last camera position
	lastCameraX = 0.0f;
	lastCameraZ = 0.0f;


	// Game Starts in 3 secs
	/*clock_t now = clock();
	while (clock() < now + START_COUNTDOWN * CLOCKS_PER_SEC)
	{
		if ((clock() - now) % CLOCKS_PER_SEC == 0)
		{
			printf("START IN %d s\r", START_COUNTDOWN - (clock() - now) / CLOCKS_PER_SEC);
		}
	}*/

	std::string instruction = "簡介\n在火場中，通常會讓人措手不及，尤其是在煙霧中、伸手不見五指時更顯著，在這樣的情況下逃生，\n往往會在路途中迷失方向或錯失逃生門。\n因此在逃生時，若能沿著牆面行走並找到安全門，即可不會發生上述的現象。\n依據不同的項目可以做出對應的多媒體教材，在此我們以逃生避難狀況的沿牆面逃生做為火災逃生模擬的對象。\n";
	std::string keyInstruction = "\n使用說明\nWSAD為上下左右，滑鼠為控制視角，滾輪放大縮小視角\n。";

	printf("%s", (instruction+keyInstruction).c_str());
	// 卡住
	system("pause");
	PlaySound(TEXT("sound.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);



	// for game, 沒在時間內完成,關閉遊戲視窗,並顯示GAME OVER
	game_start_clock = clock();
	int leftTime = 0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window) )
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
		
		// for countdown
		if (!camera.exitCollision) {
			if ((clock() - game_start_clock) % CLOCKS_PER_SEC == 0)
			{
				leftTime = GAME_COUNTDOWN - (clock() - game_start_clock) / CLOCKS_PER_SEC;
				// 空白來覆蓋原本的字
				printf("REMAINING TIME %2d s                                          \r", leftTime);
			}
			if (leftTime <= 0)
			{
				break;
			}
		}
		


        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 30.0f);
		glm::mat4 view = camera.GetViewMatrix();
		// model declaration
		glm::mat4 model;


        // obstruct object
		for (unsigned int i = 0; i < 13; i++)
		{
			obstructShader.use();
			obstructShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
			obstructShader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			obstructShader.setMat4("projection", projection);
			obstructShader.setMat4("view", view);
			// calculate the model matrix for each object and pass it to shader before drawing
			model = glm::mat4();
			model = glm::translate(model, obstructPos[i]);
			model = glm::scale(model, obstructScale[i]);
			obstructShader.setMat4("model", model);
			cubeCollisionVertexBR = model * BR;
			cubeCollisionVertexUL = model * UL;
			// check collision
			CheckCollision(i);
			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, obstructTexture);
			// obstruct light 
			//obstructShader.setVec3("material.ambient", materialAmbient);
			//obstructShader.setVec3("material.diffuse", materialDiffuse);
			obstructShader.setVec3("material.specular", materialSpecular);
			obstructShader.setFloat("material.shininess", shininess);
			obstructShader.setVec3("light.ambient", lightAmbient);
			obstructShader.setVec3("light.diffuse", lightDiffuse);
			obstructShader.setVec3("light.specular", lightSpecular);
			obstructShader.setFloat("light.constant", lightConstant);
			obstructShader.setFloat("light.linear", lightLinear);
			obstructShader.setFloat("light.quadratic", lightQuadratic);
			obstructShader.setVec3("light.position", camera.Position);
			obstructShader.setVec3("light.direction", camera.Front);
			obstructShader.setFloat("light.cutOff", lightCutOff);
			obstructShader.setFloat("light.outerCutOff", lightOuterCutOff);
			obstructShader.setVec3("viewPos", camera.Position);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


        // lamp object
        //lampShader.use();
        //lampShader.setMat4("projection", projection);
        //lampShader.setMat4("view", view);
        //model = glm::mat4();
        //model = glm::translate(model, lightPos);
        ////model = glm::scale(model, glm::vec3(0.2f)); // a smaller cube
        //lampShader.setMat4("model", model);
        //glBindVertexArray(lightVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

		// floor object
		floorShader.use();
		floorShader.setMat4("projection", projection);
		floorShader.setMat4("view", view);
		model = glm::mat4();
		model = glm::translate(model, floorPos);
		model = glm::scale(model, floorScale);
		floorShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, floorTexture);
		// floor light 
		//floorShader.setVec3("material.ambient", materialAmbient);
		//floorShader.setVec3("material.diffuse", materialDiffuse);
		floorShader.setVec3("material.specular", materialSpecular);
		floorShader.setFloat("material.shininess", shininess);
		floorShader.setVec3("light.ambient", lightAmbient);
		floorShader.setVec3("light.diffuse", lightDiffuse);
		floorShader.setVec3("light.specular", lightSpecular);
		floorShader.setFloat("light.constant", lightConstant);
		floorShader.setFloat("light.linear", lightLinear);
		floorShader.setFloat("light.quadratic", lightQuadratic);
		floorShader.setVec3("light.position", camera.Position);
		floorShader.setVec3("light.direction", camera.Front);
		floorShader.setFloat("light.cutOff", lightCutOff);
		floorShader.setFloat("light.outerCutOff", lightOuterCutOff);
		floorShader.setVec3("viewPos", camera.Position);
		glBindVertexArray(floorVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// ceil object
		ceilShader.use();
		ceilShader.setMat4("projection", projection);
		ceilShader.setMat4("view", view);
		model = glm::mat4();
		model = glm::translate(model, ceilPos);
		model = glm::scale(model, ceilScale);
		ceilShader.setMat4("model", model);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, ceilTexture);
		// ceil light
		ceilShader.setVec3("material.specular", materialSpecular);
		ceilShader.setFloat("material.shininess", shininess);
		ceilShader.setVec3("light.ambient", lightAmbient);
		ceilShader.setVec3("light.diffuse", lightDiffuse);
		ceilShader.setVec3("light.specular", lightSpecular);
		ceilShader.setFloat("light.constant", lightConstant);
		ceilShader.setFloat("light.linear", lightLinear);
		ceilShader.setFloat("light.quadratic", lightQuadratic);
		ceilShader.setVec3("light.position", camera.Position);
		ceilShader.setVec3("light.direction", camera.Front);
		ceilShader.setFloat("light.cutOff", lightCutOff);
		ceilShader.setFloat("light.outerCutOff", lightOuterCutOff);
		ceilShader.setVec3("viewPos", camera.Position);
		glBindVertexArray(ceilVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		


		// wall object
		for (unsigned int i = 0; i < 4; i++)
		{
			wallShader.use();
			wallShader.setMat4("projection", projection);
			wallShader.setMat4("view", view);
			// calculate the model matrix for each object and pass it to shader before drawing
			model = glm::mat4();
			model = glm::translate(model, wallPos[i]);
			model = glm::scale(model, wallScale[i]);
			obstructShader.setMat4("model", model);
			cubeCollisionVertexBR = model * BR;
			cubeCollisionVertexUL = model * UL;
			// check collision
			CheckCollision(i+13);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, wallTexture);
			// wall light 
			//wallShader.setVec3("material.ambient", materialAmbient);
			//wallShader.setVec3("material.diffuse", materialDiffuse);
			wallShader.setVec3("material.specular", materialSpecular);
			wallShader.setFloat("material.shininess", shininess);
			wallShader.setVec3("light.ambient", lightAmbient);
			wallShader.setVec3("light.diffuse", lightDiffuse);
			wallShader.setVec3("light.specular", lightSpecular);
			wallShader.setFloat("light.constant", lightConstant);
			wallShader.setFloat("light.linear", lightLinear);
			wallShader.setFloat("light.quadratic", lightQuadratic);
			wallShader.setVec3("light.position", camera.Position);
			wallShader.setVec3("light.direction", camera.Front);
			wallShader.setFloat("light.cutOff", lightCutOff);
			wallShader.setFloat("light.outerCutOff", lightOuterCutOff);
			wallShader.setVec3("viewPos", camera.Position);
			glBindVertexArray(wallVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// exit object
		exitShader.use();
		exitShader.setMat4("projection", projection);
		exitShader.setMat4("view", view);
		model = glm::mat4();
		model = glm::translate(model, exitPos);
		model = glm::rotate(model, (GLfloat)-3.14/2, exitRotate);
		model = glm::scale(model, exitScale);
		exitShader.setMat4("model", model);
		glm::mat4 model_exit = glm::mat4();	// no rotation
		model_exit = glm::translate(model_exit, exitPos);
		model_exit = glm::scale(model_exit, exitScale);
		exitCollisionVertexBR = model_exit * BR;
		exitCollisionVertexUL = model_exit * UL;
		// check exit collision
		CheckExitCollision();
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, exitTexture);
		glBindVertexArray(exitVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		
		

		// print camera position
		//printf("x: %.2f y: %.2f z: %.2f\n", camera.Position.x, camera.Position.y, camera.Position.z);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
	glDeleteVertexArrays(1, &floorVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
	if(!camera.exitCollision) printf("\nGAME OVER!!!!\n\a");
	system("pause");
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		restartGame();
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS);
		gameStartFlag = true;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}


// suppose there are 20 objs
glm::vec2 state[20];
std::vector <bool>initialState(20, true);
// check collision for one object
GLboolean CheckCollision(int objNum)
{
	float offset = 0.2;
	
	float cameraPositionXR = camera.Position.x + offset;
	float cameraPositionXL = camera.Position.x - offset;
	float cameraPositionZU = camera.Position.z - offset;
	float cameraPositionZD = camera.Position.z + offset;


	// x
	bool collisionX = cubeCollisionVertexBR.x >= cameraPositionXL &&
		cameraPositionXR >= cubeCollisionVertexUL.x;
	
	// z
	bool collisionZ = cubeCollisionVertexBR.z >= cameraPositionZU &&
		cameraPositionZD >= cubeCollisionVertexUL.z;

	// init state
	// fix change obj change state !! call CheckCollision twice for init and locate??
	if (initialState[objNum]) {
		state[objNum].x = collisionX;
		state[objNum].y = collisionZ;
		initialState[objNum] = false;
	} else {
		int bitX = (int)collisionX - (int)state[objNum].x;
		int bitZ = (int)collisionZ - (int)state[objNum].y;
		// Back Front
		if (bitX == 0 && bitZ == 1) {
			camera.backFront = true;
			camera.rightLeft = false;
		}
		if (bitX == 0 && bitZ == -1) camera.cameraCollision = false;

		// Right left
		if (bitX == 1 && bitZ == 0) {
			camera.backFront = false;
			camera.rightLeft = true;
		}
		if (bitX == -1 && bitZ == 0) camera.cameraCollision = false;
		
		

		//printf("%d   %d\n", backFront, rightLeft);
		state[objNum].x = collisionX;
		state[objNum].y = collisionZ;
	}
	
	//printf("collisionX: %d  collisionZ: %d\n", collisionX, collisionZ);

	if (collisionX && collisionZ) {
		camera.cameraCollision = true;
	}

	
	// two collision occur
	return collisionX && collisionZ;
}

GLboolean CheckExitCollision()
{

	float offset = 0.2;

	float cameraPositionXR = camera.Position.x + offset;
	float cameraPositionXL = camera.Position.x - offset;
	float cameraPositionZU = camera.Position.z - offset;
	float cameraPositionZD = camera.Position.z + offset;


	// x
	bool collisionX = exitCollisionVertexBR.x >= cameraPositionXL &&
		cameraPositionXR >= exitCollisionVertexUL.x;

	// z
	bool collisionZ = exitCollisionVertexBR.z >= cameraPositionZU &&
		cameraPositionZD >= exitCollisionVertexUL.z;


	if (collisionX && collisionZ) {
		// 卡死
		if (isFirstTimeExitCollision) {
			camera.exitCollision = true;
			accomplish_clock = clock();
			printf("Congrats!!! -> You spent %2d seconds to accomplish.\n", (accomplish_clock - game_start_clock) / CLOCKS_PER_SEC);
		}
		isFirstTimeExitCollision = false;
	}


	// two collision occur
	return collisionX && collisionZ;
}



// utility function for loading a 2D texture from file
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


void check_camera_direction() {
	if (camera.Position.x - lastCameraX < 0) {
		cameraRtoL = true;
		lastCameraX = camera.Position.x;
	}
	else if (camera.Position.x - lastCameraX > 0) {
		cameraRtoL = false;
		lastCameraX = camera.Position.x;
	}
	else {
		cameraRtoL = cameraRtoL;
		lastCameraX = camera.Position.x;
	}

	if (camera.Position.z - lastCameraZ < 0) {
		cameraBtoF = true;
		lastCameraZ = camera.Position.z;
	}
	else if (camera.Position.z - lastCameraZ > 0) {
		cameraBtoF = false;
		lastCameraZ = camera.Position.z;
	}
	else {
		cameraBtoF = cameraBtoF;
		lastCameraZ = camera.Position.z;
	}
}


void restartGame() {
	camera.Position.x = 0.0f;
	camera.Position.y = -4.5f;
	camera.Position.z = 0.0f;
	camera.Front.x = 0.0f;
	camera.Front.y = 0.0f;
	camera.Front.z = -1.0f;
	camera.Up.x = 0.0f;
	camera.Up.y = 1.0f;
	camera.Up.z = 0.0f;
	camera.Yaw = -90.0f;
	camera.Pitch = 0.0f;
	camera.backFront = false;
	camera.rightLeft = false;
	camera.cameraCollision = false;
	camera.exitCollision = false;
	isFirstTimeExitCollision = true;
	game_start_clock = clock();
	for (int i = 0;i < 20; i++) initialState[i] = true;
}