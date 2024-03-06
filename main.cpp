//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"

#include <iostream>
# include <windows.h>
#include <mmsystem.h>

int glWindowWidth = 1280;
int glWindowHeight = 720;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;

const unsigned int SHADOW_WIDTH = 32000;//16384;//2048;
const unsigned int SHADOW_HEIGHT = 32000;//16384;//2048;


glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
glm::vec3 lightDirSun = glm::vec3(-1.0f, -1.0f, 0.0f);
GLuint lightDirLoc;
GLuint lightDirSunLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 2.0f, 5.5f),
	glm::vec3(0.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));

float cameraSpeed = 5.03f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

gps::Model3D earth;
glm::vec3 earthCoord;
gps::Model3D sun;
glm::vec3 sunCoord;
gps::Model3D saturn;
glm::vec3 saturnCoord;
gps::Model3D jupiter;
glm::vec3 jupiterCoord;
gps::Model3D mars;
glm::vec3 marsCoord;
gps::Model3D venus;
glm::vec3 venusCoord;
gps::Model3D mercury;
glm::vec3 mercuryCoord;
gps::Model3D uranus;
glm::vec3 uranusCoord;
gps::Model3D neptune;
glm::vec3 neptuneCoord;
gps::Model3D moon;
glm::vec3 moonCoord;
gps::Model3D pluto;
glm::vec3 plutoCoord;
gps::Model3D asteroizi;
gps::Model3D phobos;
gps::Model3D deimos;
gps::Model3D europa;

#define mercuryRadius 7.0f
#define venusRadius 15.0f
#define earthRadius 20.0f 
#define marsRadius 14.0f
#define jupiterRadius 135.0f
#define saturnRadius 110.0f
#define uranusRadius 67.0f
#define neptuneRadius 60.0f
#define plutoRadius 5.0f
#define moonRadius 5.0f

gps::Model3D screenQuad;

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;

gps::Shader depthShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

GLint lightMode = 1;
GLuint lightModeLoc;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;


GLuint fogDensityLoc;
GLfloat fogDensity;

#define AstronomicalUnit 250
#define SunPaging 325

static float rev = 0, revVar = 0;
static float rot = 0, rotVar = 0;

glm::vec3 lightDirNEW;

// light parameters
glm::vec3 pointLightPosition;
glm::vec3 pointLightColor;
GLuint pointLightColorLoc;
GLuint pointLightPositionLoc;

#pragma comment(lib, "winmm.lib")

GLenum glCheckError_(const char* file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)


void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

GLboolean firstMouse = true;
float lastX = 400, lastY = 300;
double yaw = -90.0f, pitch;

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	myCamera.rotate(pitch, yaw);
}


bool colisionCalcCamMove(glm::vec3 planetCoord, float planetRadius)
{
	glm::vec3 cameraPostion = myCamera.getPosition();

	if ((cameraPostion.x - planetCoord.x) * (cameraPostion.x - planetCoord.x) + (cameraPostion.y - planetCoord.y) * (cameraPostion.y - planetCoord.y)
		+ (cameraPostion.z - planetCoord.z) * (cameraPostion.z - planetCoord.z) <= planetRadius * planetRadius) {
		return true;
	}

	return false;
}

void colisionCalcPlanetMove(glm::vec3 planetCoord, float planetRadius)
{
	glm::vec3 cameraPosition = myCamera.getPosition();

	if ((cameraPosition.x - planetCoord.x) * (cameraPosition.x - planetCoord.x) + (cameraPosition.y - planetCoord.y) * (cameraPosition.y - planetCoord.y)
		+ (cameraPosition.z - planetCoord.z) * (cameraPosition.z - planetCoord.z) <= planetRadius * planetRadius)
	{
		myCamera.setPosition(glm::vec3(cameraPosition.x + 1, cameraPosition.y + 1, cameraPosition.z + 1));
	}
}

bool planetColision()
{
	if (colisionCalcCamMove(sunCoord, SunPaging))
		return true;
	if (colisionCalcCamMove(mercuryCoord, mercuryRadius))
		return true;
	if (colisionCalcCamMove(venusCoord, venusRadius))
		return true;
	if (colisionCalcCamMove(earthCoord, earthRadius))
		return true;
	if (colisionCalcCamMove(moonCoord, moonRadius))
		return true;
	if (colisionCalcCamMove(marsCoord, marsRadius))
		return true;
	if (colisionCalcCamMove(jupiterCoord, jupiterRadius))
		return true;
	if (colisionCalcCamMove(saturnCoord, saturnRadius))
		return true;
	if (colisionCalcCamMove(uranusCoord, uranusRadius))
		return true;
	if (colisionCalcCamMove(neptuneCoord, neptuneRadius))
		return true;
	if (colisionCalcCamMove(plutoCoord,plutoRadius))
		return true;

	return false;
}

bool startMusic = true;

void processMovement()
{

	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

	if (pressedKeys[GLFW_KEY_P]) {
		startMusic = false;
	}

	if (pressedKeys[GLFW_KEY_J]) {
		lightAngle -= 1.0f;
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle += 1.0f;
	}

	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		if (planetColision()) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
			return;
		}
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		if (planetColision()) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
			return;
		}
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		if (planetColision()) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
			return;
		}
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		if (planetColision()) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed);
			return;
		}
	}

	if (pressedKeys[GLFW_KEY_F]) {
		fogDensity += 0.00002f;
		if (fogDensity >= 0.3f)
			fogDensity = 0.3f;
		myCustomShader.useShaderProgram();
		glUniform1fv(fogDensityLoc, 1, &fogDensity);
	}

	if (pressedKeys[GLFW_KEY_G]) {
		fogDensity -= 0.00002f;
		if (fogDensity <= 0.0f)
			fogDensity = 0.0f;
		myCustomShader.useShaderProgram();
		glUniform1fv(fogDensityLoc, 1, &fogDensity);
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {
		rot += 0.01f;
		if (rot >= 1.0f)
			rot = 1.0f;
	}

	if (pressedKeys[GLFW_KEY_LEFT]) {
		rot -= 0.01f;
		if (rot <= 0.0f)
			rot = 0.0f;
	}

	if (pressedKeys[GLFW_KEY_UP]) {
		rev += 0.01f;
		if (rev >= 1.0f)
			rev = 1.0f;
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		rev -= 0.01f;
		if (rev <= 0.0f)
			rev = 0.0f;
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		lightMode = 0;
		lightModeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightMode");
		glUniform1i(lightModeLoc, lightMode);
	}

	if (pressedKeys[GLFW_KEY_X]) {
		lightMode = 1;
		lightModeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightMode");
		glUniform1i(lightModeLoc, lightMode);
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//window scaling for HiDPI displays
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

	//for sRBG framebuffer
	glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	//for antialising
	glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() {
	earth.LoadModel("objects/spatiu/Earth.obj");
	sun.LoadModel("objects/spatiu/sun2.obj");
	saturn.LoadModel("objects/spatiu/saturn.obj");
	jupiter.LoadModel("objects/spatiu/jupiter.obj");
	mars.LoadModel("objects/spatiu/mars.obj");
	uranus.LoadModel("objects/spatiu/uranus.obj");
	neptune.LoadModel("objects/spatiu/neptune.obj");
	venus.LoadModel("objects/spatiu/venus.obj");
	mercury.LoadModel("objects/spatiu/mercury.obj");
	moon.LoadModel("objects/spatiu/moon.obj");
	pluto.LoadModel("objects/spatiu/pluto.obj");
	asteroizi.LoadModel("objects/spatiu/asteroids.obj");
	phobos.LoadModel("objects/spatiu/phobos.obj");
	deimos.LoadModel("objects/spatiu/deimos.obj");
	europa.LoadModel("objects/spatiu/europa.obj");

	screenQuad.LoadModel("objects/quad/quad.obj");

}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();

	depthShader.loadShader("shaders/depth.vert", "shaders/depth.frag");
	depthShader.useShaderProgram();

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
}

void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 10000000000000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 0.0f, 1.0f);

	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");

	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightColorLoc = glGetUniformLocation(lightShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightDirSunLoc = glGetUniformLocation(lightShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirSunLoc, 1, glm::value_ptr(lightDirSun));

	pointLightColor = glm::vec3(1.0f, 1.0f, 1.0f); 
	pointLightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "pointLightColor");
	glUniform3fv(pointLightColorLoc, 1, glm::value_ptr(pointLightColor));

	pointLightPosition = glm::vec3(0.0f, 0.0f, 0.0f);
	pointLightPositionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "position");
	glUniform3fv(pointLightPositionLoc, 1, glm::value_ptr(pointLightPosition));

	lightModeLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightMode");
	glUniform1i(lightModeLoc, lightMode);

	//fog density
	fogDensityLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	glUniform1fv(fogDensityLoc, 1, &fogDensity);

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


}

void initFBO() {
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);


	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {

	// Apply rotation to the point
	glm::vec4 rotatedPoint = lightRotation * glm::vec4(lightDir, 1.0f); // Extend point to a vec4

	// Extract the new coordinates after rotation
	lightDirNEW = glm::vec3(rotatedPoint.x, rotatedPoint.y, rotatedPoint.z);

	glm::mat4 lightView = glm::lookAt(lightDirNEW, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = -16.4f * AstronomicalUnit - SunPaging, far_plane = 16.4f * AstronomicalUnit + SunPaging;
	glm::mat4 lightProjection = glm::ortho(-16.4f * AstronomicalUnit - SunPaging, 16.4f * AstronomicalUnit + SunPaging, -16.4f * AstronomicalUnit - SunPaging, 16.4f * AstronomicalUnit + SunPaging, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;


	return lightSpaceTrMatrix;
}

bool cameraAnimation = true;
int anim = 1;
int camTime = 0;

void playMusic() {
	PlaySound(TEXT("No Time for Caution.wav"), NULL,SND_FILENAME | SND_ASYNC);
}

void stopMusic() {
	if(startMusic == false)
		PlaySound(NULL, NULL, SND_ASYNC);
}

void handleCameraPosition() {
	if (cameraAnimation) {
		rot = 0.2;
		//cameraAnimation = false;
		if (anim == 1) {
			if (camTime > 700) {
				camTime = 0;
				anim = 2;
			}
			else
			{
				camTime += 2;
				myCamera = gps::Camera(glm::vec3(SunPaging + 1.4f * AstronomicalUnit + 100 + camTime, 50.0f, 100.0f), glm::vec3(SunPaging + AstronomicalUnit, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}
		if (anim == 2) {

			if (camTime > 800) {
				camTime = 0;
				anim = 3;
			}
			else
			{
				camTime += 2;

				float camX = sin(camTime / 100.0f) ;
				float camZ = cos(camTime / 100.0f);

				myCamera = gps::Camera(glm::vec3(SunPaging + 2.0 * AstronomicalUnit + camX, 0.0f, camZ), glm::vec3(SunPaging + 2.0 * AstronomicalUnit, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}
		if (anim == 3) {
			if (camTime > 6 * AstronomicalUnit) {
				camTime = 0;
				anim = 4;
			}
			else
			{
				camTime += 2;
				myCamera = gps::Camera(glm::vec3(3 * camTime, 0.0f, 2 * SunPaging), glm::vec3(3 * camTime, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

			}
		}
		if (anim == 4) {
			if (camTime > 500) {
				cameraAnimation = false;
			}
			else
			{
				camTime +=2;
				myCamera = gps::Camera(glm::vec3(SunPaging + 10.55f * AstronomicalUnit + camTime/2, 30.0f, 2.5f * AstronomicalUnit) , glm::vec3(2.0f * AstronomicalUnit, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			}
		}
	}
}

void dayPasser()
{
	revVar += rev;
	rotVar += rot;
}

void drawPlanet(gps::Model3D& planet, float revolutionSpeed, float rotationSpeed, float distanceAU, gps::Shader shader, bool depthPass, glm::vec3& planetCoord, float radius)
{
	float calculRev = 0, calculRot = 0;
	bool isSun = false;

	if (revolutionSpeed == 0 && rotationSpeed == 0)
		isSun = true;

	if (!isSun)
	{
		calculRev = revVar / revolutionSpeed * 360;
		calculRot = rotVar / rotationSpeed * 360;
	}

	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(calculRev), glm::vec3(0.0f, 1.0f, 0.0f));

	if (isSun)
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
	else
		model = glm::translate(model, glm::vec3(distanceAU * AstronomicalUnit + SunPaging, 0.0f, 0.0f));

	model = glm::rotate(model, glm::radians(calculRot), glm::vec3(0.0f, 1.0f, 0.0f));

	planetCoord = glm::vec3(model * glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
	colisionCalcPlanetMove(planetCoord, radius);

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	planet.Draw(shader);
}

void drawObjects(gps::Shader shader, bool depthPass) {

	shader.useShaderProgram();

	dayPasser();
			          /*revoution*/   /*rotation*/
	drawPlanet(mercury, 87.7,            59.0, 0.5, shader, depthPass, mercuryCoord, mercuryRadius);
	drawPlanet(venus,   224.7,           -243, 0.92, shader, depthPass, venusCoord, venusRadius);
	drawPlanet(earth,   360.0,           360.0, 1.4, shader, depthPass, earthCoord, earthRadius);
	drawPlanet(mars,    687,             369.1, 2.0f, shader, depthPass, marsCoord, marsRadius);
	drawPlanet(jupiter, 12 * 360.0,      150.0, 5.2, shader, depthPass, jupiterCoord, jupiterRadius);
	drawPlanet(saturn,  29.4 * 360.0,    160.5, 8.5, shader, depthPass, saturnCoord, saturnRadius);
	drawPlanet(uranus,  84 * 360.0,      255.0, 12.5, shader, depthPass, uranusCoord, uranusRadius);
	drawPlanet(neptune, 165 * 360.0,     240.0, 15.7, shader, depthPass, neptuneCoord, neptuneRadius);
	drawPlanet(pluto,   248 * 360.0,     6 * 360.0, 17.0, shader, depthPass, plutoCoord, plutoRadius);

	float calculRev = revVar / 360 * 360;
	float calculRot = rotVar / 150 * 360;
	float nouzaeci = 270.0;
	model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(calculRev), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(SunPaging + 1.4f * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(calculRot), glm::vec3(0.0f, 1.0f, 0.1f));
	if (!depthPass)
	{
		model = glm::translate(model, glm::vec3(0.15f * AstronomicalUnit, 0.0f, 0.0f));
		moonCoord = glm::vec3(model * glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
		colisionCalcPlanetMove(moonCoord, moonRadius);
	}
	else
		model = glm::translate(model, glm::vec3(0.225f * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(nouzaeci), glm::vec3(0.0f, 1.0f, 0.0f));

	moonCoord = glm::vec3(model * glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
	colisionCalcPlanetMove(moonCoord, moonRadius);

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	moon.Draw(shader);

	float calculRev2 = revVar / 687 * 360;
	float calculRot2 = rotVar / 100 * 360;
	model = glm::mat4(1.0f);
	
	model = glm::rotate(model, glm::radians(calculRev2), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(SunPaging + 2.0 * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(calculRot2), glm::vec3(0.0f, 1.0f, -0.5f));//-0.5f z
	model = glm::translate(model, glm::vec3(0.08 * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.6f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	phobos.Draw(shader);

	float calculRev3 = revVar / 687 * 360;
	float calculRot3 = rotVar / 370 * 360;
	model = glm::mat4(1.0f);
	
	model = glm::rotate(model, glm::radians(calculRev3), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(SunPaging + 2.0 * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(calculRot3), glm::vec3(0.0f, 1.0f, 0.5f));
	model = glm::translate(model, glm::vec3(0.12 * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.6f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}
	deimos.Draw(shader);


	float calculRev4 = revVar / (12 * 360.0) * 360;
	float calculRot4 = rotVar / 150 * 360;
	model = glm::mat4(1.0f);

	model = glm::rotate(model, glm::radians(calculRev4), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, glm::vec3(SunPaging + 5.2f * AstronomicalUnit, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(calculRot4), glm::vec3(0.5f, 1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.6f));

	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	europa.Draw(shader);


	if (!depthPass) {
		float calculRotAst = rotVar / 20000 * 360;
		model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(calculRotAst), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.6f));
		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		asteroizi.Draw(myCustomShader);
	}
}

void initSkybox()
{
	std::vector<const GLchar*> faces;
	faces.push_back("skybox/right.png");
	faces.push_back("skybox/left.png");
	faces.push_back("skybox/top.png");
	faces.push_back("skybox/bottom.png");
	faces.push_back("skybox/back.png");
	faces.push_back("skybox/front.png");
	mySkyBox.Load(faces);
}

void renderScene() {

	// render depth map on screen - toggled with the M key

	handleCameraPosition();
	//render the scene to the depth buffer
	depthShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthShader, true);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);

	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		

		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		drawPlanet(sun, 0.0, 0.0, 0.0, lightShader, true, sunCoord, SunPaging);

		mySkyBox.Draw(skyboxShader, view, projection);
	}
}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char* argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects();
	initShaders();
	initUniforms();
	initFBO();
	initSkybox();

	glCheckError();
	playMusic();

	while (!glfwWindowShouldClose(glWindow)) {
		processMovement();
		initUniforms();
		renderScene();
		dayPasser();
		stopMusic();
		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}
	cleanup();
	return 0;
}
