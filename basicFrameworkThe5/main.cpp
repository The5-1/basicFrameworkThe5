#define GLEW_STATIC //Using the static lib, so we need to enable it
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <Ant/AntTweakBar.h>
#include <memory>
#include "helper.h"
#include "Shader.h"
#include "Model.h"
#include "Skybox.h"
#include "times.h"
#include "InstancedMesh.h"
#include "FBO.h"
#include "Texture.h"

#include "glm/gtx/string_cast.hpp"

//Time
Timer timer;
int frame;
long timeCounter, timebase;
char timeString[50];

//Light
struct Light {
public:
	glm::vec3 lightPosition;
	glm::vec3 lightTarget;
	glm::vec3 lightUp;
public:
	Light(glm::vec3 _lightPosition, glm::vec3 _lightTarget, glm::vec3 _lightUp) {
		lightPosition = _lightPosition;
		lightTarget = _lightTarget;
		lightUp = _lightUp;
	}
};

//Resolution (has to be changed in helper.h too)
glm::vec2 resolution = glm::vec2(1024, 768);

//Light
glm::vec3 defaultlightpos = glm::vec3(18.665f, 73.297f, -6.238f);
glm::vec3 defaultlightdir = glm::vec3(-0.194856f, -0.669356f, 0.0891604f);
Light light(defaultlightpos, defaultlightpos + defaultlightdir, glm::vec3(0.0f, 1.0f, 0.0f));

//Externals
cameraSystem cam(1.0f, 1.0f, glm::vec3(20.95f, 20.95f, -0.6f));
glm::mat4 projMatrix;
glm::mat4 viewMatrix;

//Light
glm::vec3 lightDir = glm::vec3(0.0f, 100.0f, 0.0f);

//Test-Object
simpleQuad * quad = 0;
simpleCube * cube = 0;
InstancedMesh *instancedSphere = 0;

Mesh quadMesh;
Mesh boxMesh;

//Skybox
Skybox skybox;
char* negz = "D:/Dev/Assets/SkyboxTextures/Yokohama2/negz.jpg";
char* posz = "D:/Dev/Assets/SkyboxTextures/Yokohama2/posz.jpg";
char* posy = "D:/Dev/Assets/SkyboxTextures/Yokohama2/posy.jpg";
char* negy = "D:/Dev/Assets/SkyboxTextures/Yokohama2/negy.jpg";
char* negx = "D:/Dev/Assets/SkyboxTextures/Yokohama2/negx.jpg";
char* posx = "D:/Dev/Assets/SkyboxTextures/Yokohama2/posx.jpg";

//Shaders
Shader basicShader;
Shader modelLoaderShader;
Shader modelLoaderShaderGbuffer;

Shader halfspaceFogTexturedShader;
Shader exponentialHalfspaceFogTextured;
Shader screenSpaceFogShader;
Shader screenSpaceNoiseFogShader;

//Skybox
Shader skyboxShader;

//FBO-Shader
Shader depthOnlyShader;

//Noise
Shader noiseFunctionShader;

//Shadow
Shader shadowMapShader;
int SHADOW_WIDTH = 1024;
int SHADOW_HEIGHT = 1024;
Shader msm_depthShader;
Shader msm_shadowShader;
Shader momentsShadowShader;
Shader variance_shadowShader;

//DepthOfField
Shader depthOfFieldShader;
Shader phongDofShader;
Shader satComputeShader;

//Game of Life
Shader gameOfLifeRandomNumberComputeShader;
Shader gameOfLifeComputeShader;

//Instanced Shader
Shader instancedMeshShader;

//Debug-Shader
Shader quadScreenSizedShader;
Shader standardMiniColorFboShader;
Shader standardMiniDepthFboShader;

//Particle Physic Shader
Shader particlePhysicComputeShader;

//Fub test shader
Shader fubTestShader_forward;
Shader fubTestShader_postproc;
float uMetallic = 0.0;
float uSubsurface = 0.0;
float uSpecular = 1.0;
float uRoughness = 0.1;
float uSpecularTint = 0.0;
float uAnisotropic = 0.0;
float uSheen = 0.0;
float uSheenTint = 0.0;
float uClearcoat = 0.0;
float uClearcoatGloss = 0.1;
float uLightIntensity = 3.0;


//Models
Model nanosuitModel;
Model sponzaModel;

//Textures
Texture *DoF_sat0 = 0, *DoF_sat1 = 0, *DoF_satResult = 0;
int gameOfLifeTextureHeight, gameOfLifeTextureWidth;
Texture *gameOfLifeInputTexture = 0, *gameOfLifeOutputTexture = 0;

int mandalaTextureHeight, mandalaTextureWidth;
Texture *mandalaInputTexture = 0, *mandalaOutputTexture = 0;

//Frame buffer object
FBO *fbo = 0;
FBO *fboDepthShadowMap = 0;
FBO *fboRgbDepthShadow= 0;
FBO *fboRgbDepth = 0;

//Tweakbar
TwBar *tweakBar;
enum SHADER_TYPE { STANDARD_SHADER,
					SHADOW_MAP_SHADER,
					VARIANCE_SHADOW_MAP_SHADER,
					MOMENTS_SHADOW_MAP_SHADER,
					DEBUG_SHADER, 
					FOG_SHADER, 
					EXP_FOG_SHADER, 
					FOG_SCREENSPACE_SHADER, 
					FOG_SCREENSPACE_NOISE_SHADER, 
					NOISE_SHADER,
					DEPTH_OF_FIELD_SHADER,
					EMPTY_SHADER,
					GAME_OF_LIFE_SHADER,
					MANDALA_SHADER,
					INSTANCED_MESH_SHADER,
					FUB_TEST_SHADER
};

SHADER_TYPE current_Shader = FUB_TEST_SHADER;

//Shadow
float depthBias_MomentShadow = 0.005f;
float momentBias_MomentShadow = 0.002f;

//Fog-Variables
float rho_Fog = 0.24;
glm::vec3 fogDirection_Fog = glm::vec3(0.0f, 1.0f, 0.0f);
float fogHeight_Fog = -2.0f;

//Exp-Fog-Fariables
float a_expFog = 0.24;
glm::vec3 fogDirection_expFog = glm::vec3(0.0f, 1.0f, 0.0f);
float fogHeight_expFog = -2.0f;

//Screen-Space-Fog
float fogDensity_ScreenSpace = 0.06;
float c_ScreenSpace = 1.0;
int fogType_ScreenSpace = 0;

//Noise-Function
float density_Noise = 8.0f;
int type_Noise = 0;

//Depth of Field
float focalPlane = 19.0f;
float blurFactor = 0.0f;
bool manualBlur = false;

//GameOfLife
bool nextFrame = false;

//Helper-Tweakbar-Button
bool helperBoolButton = false;
/* *********************************************************************************************************
TweakBar
********************************************************************************************************* */
void setupTweakBar() {
	// Tweakbar
	TwInit(TW_OPENGL_CORE, NULL);
	tweakBar = TwNewBar("Settings");

	// Array of drop down items
	TwEnumVal Shader_array[] = { { STANDARD_SHADER, "Standard" },
									{SHADOW_MAP_SHADER, "Shadow Map" },
									{VARIANCE_SHADOW_MAP_SHADER , "Variance Shadow Map" },
									{MOMENTS_SHADOW_MAP_SHADER , "Moments Shadow Map"},
									{DEBUG_SHADER, "Debug" },
									{FOG_SHADER, "Fog" },
									{EXP_FOG_SHADER, "Exponential fog" },
									{FOG_SCREENSPACE_SHADER, "Screenspace Fog"},
									{FOG_SCREENSPACE_NOISE_SHADER, "Screenspace Noise Fog" },
									{NOISE_SHADER, "Noise Textures"},
									{DEPTH_OF_FIELD_SHADER, "Depth of field"},
									{EMPTY_SHADER, "Empty"},
									{GAME_OF_LIFE_SHADER , "Game of life"},
									{MANDALA_SHADER, "Mandala"},
									{INSTANCED_MESH_SHADER, "Instanced Mesh"},
									{FUB_TEST_SHADER, "FUB_TEST_SHADER"}
	};


	// ATB identifier for the array
	TwType ShaderTwType = TwDefineEnum("Shader: ", Shader_array, 16); //Last number has to be the size of Shader_array!!

	// Link it to the tweak bar
	TwAddVarRW(tweakBar, "Shader", ShaderTwType, &current_Shader, NULL);

	//Seperator
	TwAddSeparator(tweakBar, "", NULL);

	//Light Dir
	TwAddVarRW(tweakBar, "lightDir", TW_TYPE_DIR3F, &lightDir, "label='Light Direction'");

	//Fog
	TwAddVarRW(tweakBar, "rho_Fog", TW_TYPE_FLOAT, &rho_Fog, " label='Rho' min=0.0 step=0.01 max=2.0");
	TwAddVarRW(tweakBar, "fogDirection_Fog", TW_TYPE_DIR3F, &fogDirection_Fog, "label='Fog Direction'");
	TwAddVarRW(tweakBar, "fogHeight_Fog", TW_TYPE_FLOAT, &fogHeight_Fog, " label='Fog Height' min=-100.0 step=0.1 max=0.0");

	//expFog
	TwAddVarRW(tweakBar, "a_expFog", TW_TYPE_FLOAT, &a_expFog, " label='a' min=0.0 step=0.01 max=2.0");
	TwAddVarRW(tweakBar, "fogDirection_expFog", TW_TYPE_DIR3F, &fogDirection_expFog, "label='Fog Direction'");
	TwAddVarRW(tweakBar, "fogHeight_expFog", TW_TYPE_FLOAT, &fogHeight_expFog, " label='Fog Height' min=-100.0 step=0.1 max=0.0");	

	//ScreenSpace Fog
	TwAddVarRW(tweakBar, "fogDensity_ScreenSpace", TW_TYPE_FLOAT, &fogDensity_ScreenSpace, " label='Fog density' min=0.0 step=0.001 max=100.0");
	TwAddVarRW(tweakBar, "c_ScreenSpace", TW_TYPE_FLOAT, &c_ScreenSpace, " label='c density' min=-100.0 step=0.001 max=100.0");
	TwAddVarRW(tweakBar, "fogType_ScreenSpace", TW_TYPE_INT32, &fogType_ScreenSpace, " label='Fog Type' min=0 step=1 max=3");

	//Noise-Function
	TwAddVarRW(tweakBar, "density_Noise", TW_TYPE_FLOAT, &density_Noise, " label='Noise density' min=0.0 step=0.001 max=100.0");
	TwAddVarRW(tweakBar, "type_Noise", TW_TYPE_INT32, &type_Noise, " label='Noise Type' min=0 step=1 max=100");

	//Shadow
	TwAddVarRW(tweakBar, "depthBias_MomentShadow", TW_TYPE_FLOAT, &depthBias_MomentShadow, " label='Depth Bias' min=0.0 step=0.00001 max=1.0");
	TwAddVarRW(tweakBar, "momentBias_MomentShadow", TW_TYPE_FLOAT, &momentBias_MomentShadow, " label='Moment Bias' min=0.0 step=0.00001 max=1.0");
	
	//Depth of Field
	TwAddVarRW(tweakBar, "focalPlane", TW_TYPE_FLOAT, &focalPlane, " label='Focal plane' min=0.0 step=0.01 max=100.0");
	TwAddVarRW(tweakBar, "blurFactor", TW_TYPE_FLOAT, &blurFactor, " label='Blur factor' min=0.0 step=0.01 max=100.0");
	TwAddVarRW(tweakBar, "manualBlur", TW_TYPE_BOOLCPP, &manualBlur, " label='Manual Blur' ");

	//Game of life
	TwAddVarRW(tweakBar, "nextFrame", TW_TYPE_BOOLCPP, &nextFrame, " label='Skip Display' ");

	//Helper-Tweakbar-Button
	TwAddVarRW(tweakBar, "helperBoolButton", TW_TYPE_BOOLCPP, &helperBoolButton, " label='Set Light to Cam' ");

	//Fub-Tweakbar
	TwAddVarRW(tweakBar, "uLightIntensity", TW_TYPE_FLOAT, &uLightIntensity, " label='uLightIntensity' min=0.0 step=0.01 max=10.0");
	TwAddVarRW(tweakBar, "uMetallic", TW_TYPE_FLOAT, &uMetallic, " label='uMetallic' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uRoughness", TW_TYPE_FLOAT, &uRoughness, " label='uRoughness' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uSpecular", TW_TYPE_FLOAT, &uSpecular, " label='uSpecular' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uSpecularTint", TW_TYPE_FLOAT, &uSpecularTint, " label='uSpecularTint' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uClearcoat", TW_TYPE_FLOAT, &uClearcoat, " label='uClearcoat' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uClearcoatGloss", TW_TYPE_FLOAT, &uClearcoatGloss, " label='uClearcoatGloss' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uSheen", TW_TYPE_FLOAT, &uSheen, " label='uSheen' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uSheenTint", TW_TYPE_FLOAT, &uSheenTint, " label='uSheenTint' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uAnisotropic", TW_TYPE_FLOAT, &uAnisotropic, " label='uAnisotropic' min=0.0 step=0.01 max=1.0");
	TwAddVarRW(tweakBar, "uSubsurface", TW_TYPE_FLOAT, &uSubsurface, " label='uSubsurface' min=0.0 step=0.01 max=1.0");

}

void updateTweakBar() {
	//Deactivate everything in Tweak-Bar
	//Light Dir
	TwDefine("Settings/lightDir visible=false");

	//Fog
	TwDefine("Settings/fogHeight_Fog visible=false");
	TwDefine("Settings/fogDirection_Fog visible=false");
	TwDefine("Settings/rho_Fog visible=false");
	//Exp-Fog
	TwDefine("Settings/fogHeight_expFog visible=false");
	TwDefine("Settings/fogDirection_expFog visible=false");
	TwDefine("Settings/a_expFog visible=false");

	//ScreenSpaceFog
	TwDefine("Settings/fogDensity_ScreenSpace visible=false");
	TwDefine("Settings/c_ScreenSpace visible=false");
	TwDefine("Settings/fogType_ScreenSpace visible=false");
	
	//Noise-Function
	TwDefine("Settings/density_Noise visible=false");
	TwDefine("Settings/type_Noise visible=false");

	//Shadow
	TwDefine("Settings/depthBias_MomentShadow visible=false");
	TwDefine("Settings/momentBias_MomentShadow visible=false");

	//Depth of field
	TwDefine("Settings/focalPlane visible=false");
	TwDefine("Settings/blurFactor visible=false");
	TwDefine("Settings/manualBlur visible=false");

	//Game Of Life
	TwDefine("Settings/nextFrame visible=false");
	
	//Helper-Tweakbar-Button
	TwDefine("Settings/helperBoolButton visible=false");

	//Fub-Tweakbar
	TwDefine("Settings/uLightIntensity visible=false");
	TwDefine("Settings/uMetallic visible=false");
	TwDefine("Settings/uRoughness visible=false");
	TwDefine("Settings/uSpecular visible=false");
	TwDefine("Settings/uSpecularTint visible=false");
	TwDefine("Settings/uClearcoat visible=false");
	TwDefine("Settings/uClearcoatGloss visible=false");
	TwDefine("Settings/uSheen visible=false");
	TwDefine("Settings/uSheenTint visible=false");
	TwDefine("Settings/uAnisotropic visible=false");
	TwDefine("Settings/uSubsurface visible=false");

	//Only show what we need
	switch (current_Shader) {
		case STANDARD_SHADER: 
			break;

		case SHADOW_MAP_SHADER:
			TwDefine("Settings/helperBoolButton visible=true");
			break;
		
		case VARIANCE_SHADOW_MAP_SHADER:
			TwDefine("Settings/helperBoolButton visible=true");
			break;

		case MOMENTS_SHADOW_MAP_SHADER:
			TwDefine("Settings/helperBoolButton visible=true");
			TwDefine("Settings/depthBias_MomentShadow visible=true");
			TwDefine("Settings/momentBias_MomentShadow visible=true");
			break;

		case DEBUG_SHADER:
			break;

		case FOG_SHADER: 
			TwDefine("Settings/rho_Fog visible=true");
			TwDefine("Settings/fogHeight_Fog visible=true");
			TwDefine("Settings/fogDirection_Fog visible=true");
			break;

		case EXP_FOG_SHADER: 
			TwDefine("Settings/a_expFog visible=true");
			TwDefine("Settings/fogHeight_expFog visible=true");
			TwDefine("Settings/fogDirection_expFog visible=true");
			break;

		case FOG_SCREENSPACE_SHADER:
			TwDefine("Settings/fogType_ScreenSpace visible=true");
			TwDefine("Settings/fogDensity_ScreenSpace visible=true");
			TwDefine("Settings/c_ScreenSpace visible=true");
			break;

		case FOG_SCREENSPACE_NOISE_SHADER:
			TwDefine("Settings/fogDensity_ScreenSpace visible=true");
			TwDefine("Settings/c_ScreenSpace visible=true");
			TwDefine("Settings/density_Noise visible=true");
			break;

		case NOISE_SHADER:
			TwDefine("Settings/density_Noise visible=true");
			TwDefine("Settings/type_Noise visible=true");
			break;

		case DEPTH_OF_FIELD_SHADER:
			TwDefine("Settings/focalPlane visible=true");
			TwDefine("Settings/blurFactor visible=true");
			TwDefine("Settings/manualBlur visible=true");
			break;

		case GAME_OF_LIFE_SHADER:
			TwDefine("Settings/nextFrame visible=true");
			break;

		case MANDALA_SHADER:
			TwDefine("Settings/nextFrame visible=true");
			break;

		case INSTANCED_MESH_SHADER:
			break;

		case FUB_TEST_SHADER:
			TwDefine("Settings/lightDir visible=true");

			TwDefine("Settings/focalPlane visible=true");
			TwDefine("Settings/blurFactor visible=true");
			TwDefine("Settings/manualBlur visible=true");

			TwDefine("Settings/uLightIntensity visible=true");
			TwDefine("Settings/uMetallic visible=true");
			TwDefine("Settings/uRoughness visible=true");
			TwDefine("Settings/uSpecular visible=true");
			TwDefine("Settings/uSpecularTint visible=true");
			TwDefine("Settings/uClearcoat visible=true");
			TwDefine("Settings/uClearcoatGloss visible=true");
			TwDefine("Settings/uSheen visible=true");
			TwDefine("Settings/uSheenTint visible=true");
			TwDefine("Settings/uAnisotropic visible=true");
			TwDefine("Settings/uSubsurface visible=true");
			break;
	}
}
/* *********************************************************************************************************
Initiation
********************************************************************************************************* */
void init() {
	//Simple-Model
	quad = new simpleQuad();
	quad->upload();

	cube = new simpleCube();
	cube->upload();

	instancedSphere = new InstancedMesh();
	instancedSphere->uploadSphere();

	quadMesh.createScreenQuad();
	boxMesh.createBox();

	//Komplex-Model
	//nanosuitModel = *(new Model("C:/Dev/Assets/Nanosuit/nanosuit.obj", false));
	sponzaModel = *(new Model("D:/Dev/Assets/Sponza_Atrium/sponza.obj", false));

	//Skybox
	skybox.createSkybox(negz, posz, posy, negy, negx, posx);



	//Depth of Field
	DoF_sat0 = new Texture(WIDTH, HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	DoF_sat1 = new Texture(WIDTH, HEIGHT, GL_RGBA32F, GL_RGBA, GL_FLOAT);

	//Game Of Life Variables
	gameOfLifeTextureHeight = 32;
	gameOfLifeTextureWidth = 32;
	gameOfLifeInputTexture = new Texture(gameOfLifeTextureWidth, gameOfLifeTextureHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	gameOfLifeOutputTexture = new Texture(gameOfLifeTextureWidth, gameOfLifeTextureHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	//Game Of Life (Fill start Texture)
	gameOfLifeRandomNumberComputeShader.enable();
	glActiveTexture(GL_TEXTURE0);
	gameOfLifeInputTexture->Bind();
	glBindImageTexture(0, gameOfLifeInputTexture->Index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	GLint work_size[3];
	glGetProgramiv(gameOfLifeRandomNumberComputeShader.ID, GL_COMPUTE_WORK_GROUP_SIZE, work_size);
	int w = gameOfLifeTextureWidth, h = gameOfLifeTextureHeight;
	int call_x = (w / work_size[0]) + (w % work_size[0] ? 1 : 0);
	int call_y = (h / work_size[1]) + (h % work_size[1] ? 1 : 0);
	glUniform2i(glGetUniformLocation(gameOfLifeRandomNumberComputeShader.ID, "res"), w, h);
	glUniform1i(glGetUniformLocation(gameOfLifeRandomNumberComputeShader.ID, "type"), 1);
	glDispatchCompute(call_x, call_y, 1); //Number of work groups to be launched in x,y and z direction
	gameOfLifeInputTexture->Unbind();
	gameOfLifeRandomNumberComputeShader.disable();


	//Mandala Variables
	mandalaTextureHeight = 32;
	mandalaTextureWidth = 32;
	mandalaInputTexture = new Texture(mandalaTextureWidth, mandalaTextureHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	mandalaOutputTexture = new Texture(mandalaTextureWidth, mandalaTextureHeight, GL_RGBA32F, GL_RGBA, GL_FLOAT);
	//Game Of Life (Fill start Texture)
	gameOfLifeRandomNumberComputeShader.enable();
	glActiveTexture(GL_TEXTURE0);
	mandalaInputTexture->Bind();
	glBindImageTexture(0, mandalaInputTexture->Index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	GLint work_sizeMandala[3];
	glGetProgramiv(gameOfLifeRandomNumberComputeShader.ID, GL_COMPUTE_WORK_GROUP_SIZE, work_sizeMandala);
	int wMandala = mandalaTextureWidth, hMandala = mandalaTextureHeight;
	int call_xMandala = (wMandala / work_sizeMandala[0]) + (wMandala % work_sizeMandala[0] ? 1 : 0);
	int call_yMandala = (hMandala / work_sizeMandala[1]) + (hMandala % work_sizeMandala[1] ? 1 : 0);
	glUniform2i(glGetUniformLocation(gameOfLifeRandomNumberComputeShader.ID, "res"), wMandala, hMandala);
	glUniform1i(glGetUniformLocation(gameOfLifeRandomNumberComputeShader.ID, "type"), 0);
	glDispatchCompute(call_xMandala, call_yMandala, 1); //Number of work groups to be launched in x,y and z direction
	mandalaInputTexture->Unbind();
	gameOfLifeRandomNumberComputeShader.disable();
}

void initFBO() {
	fbo = new FBO("Gbuffer", WIDTH, HEIGHT, FBO_GBUFFER_32BIT);
	gl_check_error("fbo 1");

	fboDepthShadowMap = new FBO("DepthShadowMap", SHADOW_WIDTH, SHADOW_HEIGHT, FBO_DEPTH_16BIT);
	gl_check_error("post DepthShadowMap");

	fboRgbDepthShadow = new FBO("MomentsShadowMap", SHADOW_WIDTH, SHADOW_HEIGHT, FBO_RGB_DEPTH_32BIT);
	gl_check_error("MomentsShadowMap");

	fboRgbDepth = new FBO("RgbDepth", WIDTH, HEIGHT, FBO_RGBA_DEPTH_32BIT);
	gl_check_error("RgbDepth");
}

void loadShader(bool init) {
	std::cout << "main.cpp, loadShader: Reload Shader" << std::endl;

	basicShader = Shader("./shader/standard.vs.glsl", "./shader/standard.fs.glsl");
	modelLoaderShader = Shader("./shader/modelLoader.vs.glsl", "./shader/modelLoader.fs.glsl");
	modelLoaderShaderGbuffer = Shader("./shader/modelLoaderGbuffer.vs.glsl", "./shader/modelLoaderGbuffer.fs.glsl");
	//Fog
	halfspaceFogTexturedShader = Shader("./shader/halfspaceFogTextured.vs.glsl", "./shader/halfspaceFogTextured.fs.glsl");
	exponentialHalfspaceFogTextured = Shader("./shader/exponentialHalfspaceFogTextured.vs.glsl", "./shader/exponentialHalfspaceFogTextured.fs.glsl");
	screenSpaceFogShader = Shader("./shader/screenSpaceFog.vs.glsl", "./shader/screenSpaceFog.fs.glsl");
	screenSpaceNoiseFogShader = Shader("./shader/screenSpaceNoiseFog.vs.glsl", "./shader/screenSpaceNoiseFog.fs.glsl");
	
	//Noise
	noiseFunctionShader = Shader("./shader/noiseFunction.vs.glsl", "./shader/noiseFunction.fs.glsl");

	//Helper-Shader
	quadScreenSizedShader = Shader("./shader/quadScreenSized.vs.glsl", "./shader/quadScreenSized.fs.glsl");
	standardMiniColorFboShader = Shader("./shader/standardMiniColorFBO.vs.glsl", "./shader/standardMiniColorFBO.fs.glsl");
	standardMiniDepthFboShader = Shader("./shader/standardMiniDepthFBO.vs.glsl", "./shader/standardMiniDepthFBO.fs.glsl");

	//Skybox
	skyboxShader = Shader("./shader/skybox.vs.glsl", "./shader/skybox.fs.glsl");

	//Shadow 
	shadowMapShader = Shader("./shader/shadowMap.vs.glsl", "./shader/shadowMap.fs.glsl");
	msm_depthShader = Shader("./shader/msm_depth.vs.glsl", "./shader/msm_depth.fs.glsl");
	msm_shadowShader = Shader("./shader/msm_shadow.vs.glsl", "./shader/msm_shadow.fs.glsl");
	momentsShadowShader = Shader("./shader/momentsShadow.vs.glsl", "./shader/momentsShadow.fs.glsl");
	variance_shadowShader = Shader("./shader/variance_shadow.vs.glsl", "./shader/variance_shadow.fs.glsl");

	//Depth of Field
	depthOfFieldShader = Shader("./shader/DepthOfField/dof.vs.glsl", "./shader/DepthOfField/dof.fs.glsl");
	satComputeShader = Shader("./shader/DepthOfField/sat.cs.glsl");
	phongDofShader = Shader("./shader/DepthOfField/phong.vs.glsl", "./shader/DepthOfField/phong.fs.glsl");

	//GameOfLife
	gameOfLifeRandomNumberComputeShader = Shader("./shader/GameOfLife/gameOfLifeRandomNumber.cs.glsl");
	gameOfLifeComputeShader = Shader("./shader/GameOfLife/gameOfLife.cs.glsl");
	
	//Instanced Mesh
	instancedMeshShader = Shader("./shader/LearnGL/instancing.vs.glsl", "./shader/LearnGL/instancing.fs.glsl");

	//FBO-Shader
	depthOnlyShader = Shader("./shader/depthOnly.vs.glsl", "./shader/depthOnly.fs.glsl");

	//ParticlePhysic
	particlePhysicComputeShader = Shader("./shader/PhysicEngine/particlePhysic.cs.glsl");

	//fubTestShader
	fubTestShader_forward = Shader("./shader/fub_forward_test01.vs.glsl", "./shader/fub_forward_test01.fs.glsl");
	fubTestShader_postproc = Shader("./shader/fub_postproc_test01.vs.glsl", "./shader/fub_postproc_test01.fs.glsl");

}

/* *********************************************************************************************************
Scenes: Sponza
********************************************************************************************************* */
void sponzaStandardScene(){
	skyboxShader.enable();
	skyboxShader.uniform("projMatrix", projMatrix);
	skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	skybox.Draw(skyboxShader);
	skyboxShader.disable();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	
	modelLoaderShader.enable();
	modelLoaderShader.uniform("projection", projMatrix);
	modelLoaderShader.uniform("view", viewMatrix);
	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	modelLoaderShader.uniform("model", modelMatrix);

	modelLoaderShader.uniform("texture_diffuse", 0);
	modelLoaderShader.uniform("texture_normal", 1);

	sponzaModel.Draw(modelLoaderShader);
	modelLoaderShader.disable();

	basicShader.enable();
	basicShader.uniform("projMatrix", projMatrix);
	basicShader.uniform("viewMatrix", viewMatrix);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(150.0f, 30.0f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(100.0f));
	basicShader.uniform("modelMatrix", modelMatrix);
	basicShader.uniform("col", glm::vec3(0.0f, 1.0f, 0.0f));
	boxMesh.Draw(basicShader);
	basicShader.disable();
}

void sponzaShadowMap() {
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Resolution of Shadowmap-Viewport has to be the same as the resolution of the shadow map depth buffer
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int orig_viewport[4];
	glGetIntegerv(GL_VIEWPORT, orig_viewport);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	if (helperBoolButton) {
		light.lightPosition = glm::vec3(cam.position.x, cam.position.y, cam.position.z);
		light.lightTarget = light.lightPosition + glm::vec3(cam.viewDir.x, cam.viewDir.y, cam.viewDir.z);
		light.lightUp = glm::vec3(cam.upDir.x, cam.upDir.y, cam.upDir.z);
	}


	//Fill FBO
	fboDepthShadowMap->Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	depthOnlyShader.enable();

	float near_plane = 1.0f, far_plane = 200.0f;
	glm::mat4 lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(light.lightPosition, light.lightTarget, light.lightUp);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	if (helperBoolButton) {
		helperBoolButton = false;
		lightSpaceMatrix = projMatrix * viewMatrix;
	}

	depthOnlyShader.uniform("lightSpaceMatrix", lightSpaceMatrix);

	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	depthOnlyShader.uniform("modelMatrix", modelMatrix);

	sponzaModel.Draw(depthOnlyShader); 

	depthOnlyShader.disable();	
	fboDepthShadowMap->Unbind();
	
	//Skybox
	skyboxShader.enable();
	skyboxShader.uniform("projMatrix", projMatrix);
	skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	skybox.Draw(skyboxShader);
	skyboxShader.disable();

	//Draw Scene with shadows
	glViewport(orig_viewport[0], orig_viewport[1], orig_viewport[2], orig_viewport[3]);
	glCullFace(GL_BACK);
	shadowMapShader.enable();
	shadowMapShader.uniform("projection", projMatrix);
	shadowMapShader.uniform("view", viewMatrix);
	shadowMapShader.uniform("model", modelMatrix);
	shadowMapShader.uniform("lightSpaceMatrix", lightSpaceMatrix);
	fboDepthShadowMap->bindDepth(4);
	shadowMapShader.uniform("shadowMap", 4);
	shadowMapShader.uniform("lightPos", light.lightPosition);
	shadowMapShader.uniform("viewPos", glm::vec3(cam.position.x, cam.position.y, cam.position.z));
	sponzaModel.Draw(shadowMapShader);
	shadowMapShader.disable();

	//Show depth for debugging
	standardMiniDepthFboShader.enable();
	fboDepthShadowMap->bindAllTextures();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fboDepthShadowMap->unbindAllTextures();
	standardMiniDepthFboShader.disable();
}

void sponzaVarianceShadowMap() {
	//Fill FBO
	fboRgbDepthShadow->Bind();

	int orig_viewport[4];
	glGetIntegerv(GL_VIEWPORT, orig_viewport);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	msm_depthShader.enable();

	float near_plane = 1.0f, far_plane = 200.0f;
	glm::mat4 lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(light.lightPosition, light.lightTarget, light.lightUp);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	if (helperBoolButton) {
		helperBoolButton = false;
		lightSpaceMatrix = projMatrix * viewMatrix;
	}

	msm_depthShader.uniform("uMatVP_Light", lightSpaceMatrix);

	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	msm_depthShader.uniform("uMatM", modelMatrix);

	msm_depthShader.uniform("derivative", true);

	sponzaModel.Draw(msm_depthShader);

	msm_depthShader.disable();
	fboRgbDepthShadow->Unbind();

	//Draw Shadow
	glViewport(orig_viewport[0], orig_viewport[1], orig_viewport[2], orig_viewport[3]);

	//Skybox
	skyboxShader.enable();
	skyboxShader.uniform("projMatrix", projMatrix);
	skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	skybox.Draw(skyboxShader);
	skyboxShader.disable();

	//Draw Scene with shadows
	glViewport(orig_viewport[0], orig_viewport[1], orig_viewport[2], orig_viewport[3]);
	glCullFace(GL_BACK);
	variance_shadowShader.enable();
	variance_shadowShader.uniform("uMatP", projMatrix);
	variance_shadowShader.uniform("uMatV", viewMatrix);
	variance_shadowShader.uniform("uMatM", modelMatrix);
	variance_shadowShader.uniform("uMatVP_Light", lightSpaceMatrix);

	fboRgbDepthShadow->bindTexture(0, 3);
	variance_shadowShader.uniform("shadowMap_moments", 3);

	fboRgbDepthShadow->bindDepth(4);
	variance_shadowShader.uniform("shadowMap_depth", 4);

	variance_shadowShader.uniform("lightPos", light.lightPosition);
	variance_shadowShader.uniform("viewPos", glm::vec3(cam.position.x, cam.position.y, cam.position.z));
	sponzaModel.Draw(variance_shadowShader);
	variance_shadowShader.disable();

	//Debug Textures
	standardMiniColorFboShader.enable();
	fboRgbDepthShadow->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fboRgbDepthShadow->unbindTexture(0);
	standardMiniColorFboShader.disable();

	standardMiniDepthFboShader.enable();
	fboRgbDepthShadow->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.7f, 0.4f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 0.7f));
	quad->draw();
	fboRgbDepthShadow->unbindDepth();
	standardMiniDepthFboShader.disable();
}

void sponzaMomentsShadowMap(){
	///////////////////////////////						  
	//Fill FBO
	///////////////////////////////	
	fboDepthShadowMap->Bind();

	int orig_viewport[4];
	glGetIntegerv(GL_VIEWPORT, orig_viewport);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

	if (helperBoolButton) {
		light.lightPosition = glm::vec3(cam.position.x, cam.position.y, cam.position.z);
		light.lightTarget = light.lightPosition + glm::vec3(cam.viewDir.x, cam.viewDir.y, cam.viewDir.z);
		light.lightUp = glm::vec3(cam.upDir.x, cam.upDir.y, cam.upDir.z);
	}

	glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	depthOnlyShader.enable();

	float near_plane = 1.0f, far_plane = 200.0f;
	glm::mat4 lightProjection = glm::ortho(-80.0f, 80.0f, -80.0f, 80.0f, near_plane, far_plane);

	glm::mat4 lightView = glm::lookAt(light.lightPosition, light.lightTarget, light.lightUp);

	glm::mat4 lightSpaceMatrix = lightProjection * lightView;

	if (helperBoolButton) {
		helperBoolButton = false;
		lightSpaceMatrix = projMatrix * viewMatrix;
	}

	depthOnlyShader.uniform("lightSpaceMatrix", lightSpaceMatrix);

	glm::mat4 modelMatrix;
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	depthOnlyShader.uniform("modelMatrix", modelMatrix);

	sponzaModel.Draw(depthOnlyShader);

	depthOnlyShader.disable();
	fboDepthShadowMap->Unbind();

	//Draw Shadow
	glViewport(orig_viewport[0], orig_viewport[1], orig_viewport[2], orig_viewport[3]);

	//Skybox
	skyboxShader.enable();
	skyboxShader.uniform("projMatrix", projMatrix);
	skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	skybox.Draw(skyboxShader);
	skyboxShader.disable();

	///////////////////////////////						  
	//Draw Scene
	///////////////////////////////	
	glViewport(orig_viewport[0], orig_viewport[1], orig_viewport[2], orig_viewport[3]);
	glCullFace(GL_BACK);

	momentsShadowShader.enable();
	momentsShadowShader.uniform("uMatP", projMatrix);
	momentsShadowShader.uniform("uMatV", viewMatrix);
	momentsShadowShader.uniform("uMatM", modelMatrix);
	momentsShadowShader.uniform("uMatVP_Light", lightSpaceMatrix);
	momentsShadowShader.uniform("depthBias", depthBias_MomentShadow);
	momentsShadowShader.uniform("momentBias",momentBias_MomentShadow);

	fboDepthShadowMap->bindDepth(4);
	momentsShadowShader.uniform("shadowMap_depth", 4);

	momentsShadowShader.uniform("lightPos", light.lightPosition);
	momentsShadowShader.uniform("viewPos", glm::vec3(cam.position.x, cam.position.y, cam.position.z));
	sponzaModel.Draw(momentsShadowShader);
	momentsShadowShader.disable();

	///////////////////////////////						  
	//Debug Textures
	///////////////////////////////	
	standardMiniDepthFboShader.enable();
	fboDepthShadowMap->bindAllTextures();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fboDepthShadowMap->unbindAllTextures();
	standardMiniDepthFboShader.disable();
}

void sponzaDebugScene() {
	fbo->Bind();
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glm::mat4 modelMatrix;
		modelLoaderShaderGbuffer.enable();
		modelLoaderShaderGbuffer.uniform("projMatrix", projMatrix);
		modelLoaderShaderGbuffer.uniform("viewMatrix", viewMatrix);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
		modelLoaderShaderGbuffer.uniform("modelMatrix", modelMatrix);
		sponzaModel.Draw(modelLoaderShaderGbuffer);
		modelLoaderShaderGbuffer.disable();

		gl_check_error("post teapot error test");
	}
	fbo->Unbind();

	//Diffuse
	standardMiniColorFboShader.enable();
	fbo->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.5f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 1.0f));
	quad->draw();
	fbo->unbindTexture(0);
	standardMiniColorFboShader.disable();

	//Depth
	standardMiniDepthFboShader.enable();
	fbo->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.5f, 0.5f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fbo->unbindDepth();
	standardMiniDepthFboShader.disable();

	//Normal
	standardMiniColorFboShader.enable();
	fbo->bindTexture(1);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(0.5f, 0.5f));
	quad->draw();
	fbo->unbindTexture(1);
	standardMiniColorFboShader.disable();

	//Position
	standardMiniColorFboShader.enable();
	fbo->bindTexture(2);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.5f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.5f));
	quad->draw();
	fbo->unbindTexture(2);
	standardMiniColorFboShader.disable();
}

void sponzaFogScene() {
	glm::mat4 modelMatrix;
	float k_Fog;
	glm::vec4 C_Fog;
	glm::vec4 F_Fog = glm::vec4(fogDirection_Fog, fogHeight_Fog);

	C_Fog = cam.position;
	if (glm::dot(F_Fog, C_Fog) <= 0) {
		k_Fog = 1.0f;
	}
	else {
		k_Fog = 0.0f;
	}
	halfspaceFogTexturedShader.enable();
	halfspaceFogTexturedShader.uniform("viewMatrix", viewMatrix);
	halfspaceFogTexturedShader.uniform("projMatrix", projMatrix);
	halfspaceFogTexturedShader.uniform("rho", rho_Fog);
	halfspaceFogTexturedShader.uniform("F", F_Fog);
	halfspaceFogTexturedShader.uniform("C", C_Fog);
	halfspaceFogTexturedShader.uniform("k", k_Fog);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	halfspaceFogTexturedShader.uniform("modelMatrix", modelMatrix);
	sponzaModel.Draw(halfspaceFogTexturedShader);
}

void sponzaFogExponentialScene() {
	glm::mat4 modelMatrix;
	float k_expFog;
	glm::vec4 C_expFog;
	glm::vec4 F_expFog = glm::vec4(fogDirection_expFog, fogHeight_expFog);

	C_expFog = cam.position;
	if (glm::dot(F_expFog, C_expFog) <= 0) {
		k_expFog = 1.0f;
	}
	else {
		k_expFog = 0.0f;
	}
	exponentialHalfspaceFogTextured.enable();
	exponentialHalfspaceFogTextured.uniform("viewMatrix", viewMatrix);
	exponentialHalfspaceFogTextured.uniform("projMatrix", projMatrix);
	exponentialHalfspaceFogTextured.uniform("a", a_expFog);
	exponentialHalfspaceFogTextured.uniform("F", F_expFog);
	exponentialHalfspaceFogTextured.uniform("C", C_expFog);
	exponentialHalfspaceFogTextured.uniform("k", k_expFog);

	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
	modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
	exponentialHalfspaceFogTextured.uniform("modelMatrix", modelMatrix);
	sponzaModel.Draw(exponentialHalfspaceFogTextured);
	exponentialHalfspaceFogTextured.disable();
}

void sponzaFogScreenspaceSceneNoise() {
	fbo->Bind();
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glm::mat4 modelMatrix;
		modelLoaderShaderGbuffer.enable();
		modelLoaderShaderGbuffer.uniform("projMatrix", projMatrix);
		modelLoaderShaderGbuffer.uniform("viewMatrix", viewMatrix);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
		modelLoaderShaderGbuffer.uniform("modelMatrix", modelMatrix);
		sponzaModel.Draw(modelLoaderShaderGbuffer);
		modelLoaderShaderGbuffer.disable();

		gl_check_error("post teapot error test");
	}
	fbo->Unbind();

	screenSpaceNoiseFogShader.enable();
	screenSpaceNoiseFogShader.uniform("cameraPosition", glm::vec3(cam.position.x, cam.position.y, cam.position.z) );
	screenSpaceNoiseFogShader.uniform("fogDensity", fogDensity_ScreenSpace);
	screenSpaceNoiseFogShader.uniform("c", c_ScreenSpace);
	screenSpaceNoiseFogShader.uniform("time", (float)timer.currentTime);
	screenSpaceNoiseFogShader.uniform("resolution", resolution);
	screenSpaceNoiseFogShader.uniform("density", density_Noise);

	fbo->bindTexture(0, 0);
	screenSpaceNoiseFogShader.uniform("diffuseTex", 0);

	fbo->bindTexture(2, 1);
	screenSpaceNoiseFogShader.uniform("positionTex", 1);

	quadMesh.Draw(screenSpaceNoiseFogShader);
	fbo->unbindTexture(0, 0);
	fbo->unbindTexture(2, 1);

	screenSpaceNoiseFogShader.disable();
}

void sponzaFogScreenspaceScene() {
	fbo->Bind();
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		glm::mat4 modelMatrix;
		modelLoaderShaderGbuffer.enable();
		modelLoaderShaderGbuffer.uniform("projMatrix", projMatrix);
		modelLoaderShaderGbuffer.uniform("viewMatrix", viewMatrix);
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
		modelLoaderShaderGbuffer.uniform("modelMatrix", modelMatrix);
		sponzaModel.Draw(modelLoaderShaderGbuffer);
		modelLoaderShaderGbuffer.disable();

		gl_check_error("post teapot error test");
	}
	fbo->Unbind();

	screenSpaceFogShader.enable();
	screenSpaceFogShader.uniform("projMatrix", projMatrix);
	screenSpaceFogShader.uniform("cameraDirection", glm::vec3(cam.viewDir.x, cam.viewDir.y, cam.viewDir.z));
	screenSpaceFogShader.uniform("cameraPosition", glm::vec3(cam.position.x, cam.position.y, cam.position.z));
	screenSpaceFogShader.uniform("fogDensity", fogDensity_ScreenSpace);
	screenSpaceFogShader.uniform("fogType", fogType_ScreenSpace);
	screenSpaceFogShader.uniform("c", c_ScreenSpace);
	screenSpaceFogShader.uniform("time", (float)timer.intervall);

	fbo->bindAllTextures();
	screenSpaceFogShader.uniform("diffuseTex", 0);
	screenSpaceFogShader.uniform("normalTex", 1);
	screenSpaceFogShader.uniform("positionTex", 2);
	screenSpaceFogShader.uniform("depthTex", 3);

	quad->draw();

	fbo->unbindAllTextures();

	screenSpaceFogShader.disable();
}

/* *********************************************************************************************************
Scenes: Effects
********************************************************************************************************* */
void renderNoise() {
	noiseFunctionShader.enable();
	noiseFunctionShader.uniform("time", (float)timer.currentTime);
	noiseFunctionShader.uniform("resolution", resolution);
	noiseFunctionShader.uniform("density", density_Noise);
	noiseFunctionShader.uniform("type", type_Noise);

	quad->draw();
	noiseFunctionShader.disable();
}

void calculateSATforDoF() {
	Texture* input = &(fboRgbDepth->attachment_texture[0]);
	Texture* output = DoF_sat0;

	int n = std::ceil(std::log2(WIDTH)) * 1;
	int m = std::ceil(std::log2(HEIGHT)) * 1;

	for (int i = 0; i < n; i++) {
		satComputeShader.enable();
		glActiveTexture(GL_TEXTURE0);
		input->Bind();
		glBindImageTexture(1, output->Index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		GLint work_size[3];
		glGetProgramiv(satComputeShader.ID, GL_COMPUTE_WORK_GROUP_SIZE, work_size);

		int w = WIDTH, h = HEIGHT;

		int call_x = (w / work_size[0]) + (w % work_size[0] ? 1 : 0);
		int call_y = (h / work_size[1]) + (h % work_size[1] ? 1 : 0);
		glUniform2i(glGetUniformLocation(satComputeShader.ID, "res"), w, h);

		glUniform1i(glGetUniformLocation(satComputeShader.ID, "pass"), i);
		glUniform1i(glGetUniformLocation(satComputeShader.ID, "stepstuff"), 0);

		glDispatchCompute(call_x, call_y, 1);
		glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		input->Unbind();

		if (input == &(fboRgbDepth->attachment_texture[0]))
			input = DoF_sat1;

		Texture* temp = input;
		input = output;
		output = temp;
	}

	for (int i = 0; i < m; i++) {
		satComputeShader.enable();
		glActiveTexture(GL_TEXTURE0);
		input->Bind();
		glBindImageTexture(1, output->Index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		GLint work_size[3];
		glGetProgramiv(satComputeShader.ID, GL_COMPUTE_WORK_GROUP_SIZE, work_size);
		int w = WIDTH, h = HEIGHT;
		int call_x = (w / work_size[0]) + (w % work_size[0] ? 1 : 0);
		int call_y = (h / work_size[1]) + (h % work_size[1] ? 1 : 0);
		glUniform2i(glGetUniformLocation(satComputeShader.ID, "res"), w, h);

		glUniform1i(glGetUniformLocation(satComputeShader.ID, "pass"), i);
		glUniform1i(glGetUniformLocation(satComputeShader.ID, "stepstuff"), 1);

		glDispatchCompute(call_x, call_y, 1);
		glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);
		input->Unbind();

		Texture* temp = input;
		input = output;
		output = temp;
	}

	DoF_satResult = input;	
}

void renderDofScene() {
	

	fboRgbDepth->Bind();

	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		phongDofShader.enable();
		phongDofShader.uniform("viewMatrix", viewMatrix);
		phongDofShader.uniform("projMatrix", projMatrix);
		phongDofShader.uniform("lightDir", glm::normalize(glm::vec3(0.3f, 0.3f, 0.3f)));
		glm::mat4 modelMatrix;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
		phongDofShader.uniform("modelMatrix", modelMatrix);
		phongDofShader.uniform("texture_diffuse", 0);
		sponzaModel.Draw(phongDofShader);
		phongDofShader.disable();
	}

	fboRgbDepth->Unbind();

	calculateSATforDoF();

	// apply depth of field
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		depthOfFieldShader.enable();

		glActiveTexture(GL_TEXTURE0);
		DoF_satResult->Bind();
		depthOfFieldShader.uniform("sat", 0);

		glActiveTexture(GL_TEXTURE1);
		fboRgbDepth->bindTexture(0, 1);
		depthOfFieldShader.uniform("color", 1);

		glActiveTexture(GL_TEXTURE2);
		fboRgbDepth->bindDepth(2);
		depthOfFieldShader.uniform("depth", 2);

		depthOfFieldShader.uniform("manualBlur", manualBlur);
		depthOfFieldShader.uniform("blur", blurFactor);
		depthOfFieldShader.uniform("focalPlane", focalPlane);
		int w = WIDTH, h = HEIGHT;
		glUniform2i(glGetUniformLocation(depthOfFieldShader.ID, "res"), w, h);

		quad->draw();
	}

	gl_check_error("after dof");

	depthOfFieldShader.disable();
	DoF_satResult->Unbind();
	fboRgbDepth->unbindTexture(0, 1);
	fboRgbDepth->unbindDepth(2);

	//standardMiniColorFboShader.enable();
	//DoF_satResult->Bind();
	//standardMiniColorFboShader.uniform("tex", 0);
	//standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	//standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	//quad->draw();
	//DoF_satResult->Unbind();
	//standardMiniColorFboShader.disable();

}

void renderGameOfLife() {
	/////////////////////////////////////////////////////////////
	//Image vs Texture:
	//-Can only write to images not textues
	//-Can read from images and textues
	//-Images seem to be slower
	/////////////////////////////////////////////////////////////
	glDisable(GL_DEPTH_TEST);

	if (!nextFrame) {
		gameOfLifeComputeShader.enable();

		glActiveTexture(GL_TEXTURE0);
		gameOfLifeInputTexture->Bind();
		glBindImageTexture(0, gameOfLifeInputTexture->Index(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glActiveTexture(GL_TEXTURE1);
		gameOfLifeOutputTexture->Bind();
		glBindImageTexture(1, gameOfLifeOutputTexture->Index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		GLint work_size[3];
		glGetProgramiv(gameOfLifeComputeShader.ID, GL_COMPUTE_WORK_GROUP_SIZE, work_size);
		int w = gameOfLifeTextureWidth, h = gameOfLifeTextureHeight;
		int call_x = (w / work_size[0]) + (w % work_size[0] ? 1 : 0);
		int call_y = (h / work_size[1]) + (h % work_size[1] ? 1 : 0);
		glUniform2i(glGetUniformLocation(gameOfLifeComputeShader.ID, "res"), w, h);
		glUniform1i(glGetUniformLocation(gameOfLifeRandomNumberComputeShader.ID, "type"), 1);
		glDispatchCompute(call_x, call_y, 1); //Number of work groups to be launched in x,y and z direction

		gameOfLifeInputTexture->Unbind();
		gameOfLifeOutputTexture->Unbind();
		gameOfLifeComputeShader.disable();
	}

	
	//Display
	standardMiniColorFboShader.enable();
	glActiveTexture(GL_TEXTURE0);
	gameOfLifeOutputTexture->Bind();
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	gameOfLifeOutputTexture->Unbind();
	standardMiniColorFboShader.disable();
	
	/*
	//Debug window
	//Input Texture
	standardMiniColorFboShader.enable();
	glActiveTexture(GL_TEXTURE0);
	gameOfLifeInputTexture->Bind();
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	gameOfLifeInputTexture->Unbind();
	standardMiniColorFboShader.disable();

	//Output Texture
	standardMiniColorFboShader.enable();
	glActiveTexture(GL_TEXTURE1);
	gameOfLifeOutputTexture->Bind();
	standardMiniColorFboShader.uniform("tex", 1);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.3f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.6f));
	quad->draw();
	gameOfLifeOutputTexture->Unbind();
	standardMiniColorFboShader.disable();
	*/

	//Swap images (necessary to stop racing situation)
	if (!nextFrame) {
		Texture* temp = gameOfLifeInputTexture;
		gameOfLifeInputTexture = gameOfLifeOutputTexture;
		gameOfLifeOutputTexture = temp;
		//nextFrame = true;
	}
}

void renderMandala() {
	/////////////////////////////////////////////////////////////
	//Image vs Texture:
	//-Can only write to images not textues
	//-Can read from images and textues
	//-Images seem to be slower
	/////////////////////////////////////////////////////////////
	glDisable(GL_DEPTH_TEST);

	if (!nextFrame) {
		gameOfLifeComputeShader.enable();

		glActiveTexture(GL_TEXTURE0);
		mandalaInputTexture->Bind();
		glBindImageTexture(0, mandalaInputTexture->Index(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

		glActiveTexture(GL_TEXTURE1);
		mandalaOutputTexture->Bind();
		glBindImageTexture(1, mandalaOutputTexture->Index(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

		GLint work_sizeMandala[3];
		glGetProgramiv(gameOfLifeComputeShader.ID, GL_COMPUTE_WORK_GROUP_SIZE, work_sizeMandala);
		int wMandala = mandalaTextureWidth, hMandala = mandalaTextureHeight;
		int call_xMandala = (wMandala / work_sizeMandala[0]) + (wMandala % work_sizeMandala[0] ? 1 : 0);
		int call_yMandala = (hMandala / work_sizeMandala[1]) + (hMandala % work_sizeMandala[1] ? 1 : 0);
		glUniform2i(glGetUniformLocation(gameOfLifeComputeShader.ID, "res"), wMandala, hMandala);
		glUniform1i(glGetUniformLocation(gameOfLifeRandomNumberComputeShader.ID, "type"), 0);
		glDispatchCompute(call_xMandala, call_yMandala, 1); //Number of work groups to be launched in x,y and z direction

		mandalaInputTexture->Unbind();
		mandalaOutputTexture->Unbind();
		gameOfLifeComputeShader.disable();
	}


	//Display
	standardMiniColorFboShader.enable();
	glActiveTexture(GL_TEXTURE0);
	mandalaOutputTexture->Bind();
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.0f, 0.0f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	mandalaOutputTexture->Unbind();
	standardMiniColorFboShader.disable();

	/*
	//Debug window
	//Input Texture
	standardMiniColorFboShader.enable();
	glActiveTexture(GL_TEXTURE0);
	gameOfLifeInputTexture->Bind();
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	gameOfLifeInputTexture->Unbind();
	standardMiniColorFboShader.disable();

	//Output Texture
	standardMiniColorFboShader.enable();
	glActiveTexture(GL_TEXTURE1);
	gameOfLifeOutputTexture->Bind();
	standardMiniColorFboShader.uniform("tex", 1);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.3f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 0.6f));
	quad->draw();
	gameOfLifeOutputTexture->Unbind();
	standardMiniColorFboShader.disable();
	*/

	//Swap images (necessary to stop racing situation)
	if (!nextFrame) {
		Texture* temp = mandalaInputTexture;
		mandalaInputTexture = mandalaOutputTexture;
		mandalaOutputTexture = temp;
		nextFrame = true;
	}
}

void renderInstancedMesh() {
	//Draw current position of particles
	instancedMeshShader.enable();
	instancedMeshShader.uniform("projMatrix", projMatrix);
	instancedMeshShader.uniform("viewMatrix", viewMatrix);
	instancedSphere->drawSphere();
	instancedMeshShader.disable();

	//Calculate next particle position
	particlePhysicComputeShader.enable();
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, instancedSphere->vbo[2]);
	int particleCount = 1000;
	int workgroupsX = (particleCount / 128) + 1;
	glDispatchCompute(workgroupsX, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	particlePhysicComputeShader.disable();
}

void renderFubTestScene() {
	
	fboRgbDepth->Bind();
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		fubTestShader_forward.enable();
		fubTestShader_forward.uniform("viewMatrix", viewMatrix);
		fubTestShader_forward.uniform("projMatrix", projMatrix);
		fubTestShader_forward.uniform("lightDir", glm::normalize(glm::vec3(0.3f, 0.3f, 0.3f)));
		glm::mat4 modelMatrix;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -1.75f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(0.03f, 0.03f, 0.03f));
		fubTestShader_forward.uniform("modelMatrix", modelMatrix);
		fubTestShader_forward.uniform("texture_diffuse", 0);

		fubTestShader_forward.uniform("cameraPosition", glm::vec3(cam.position.x, cam.position.y, cam.position.z));

		fubTestShader_forward.uniform("uTime", (float)timer.currentTime);

		fubTestShader_forward.uniform("lightDir", lightDir);
		fubTestShader_forward.uniform("uLightIntensity", uLightIntensity);

		fubTestShader_forward.uniform("uMetallic", uMetallic);
		fubTestShader_forward.uniform("uSubsurface", uSubsurface);
		fubTestShader_forward.uniform("uSpecular", uSpecular);
		fubTestShader_forward.uniform("uRoughness", uRoughness);
		fubTestShader_forward.uniform("uSpecularTint", uSpecularTint);
		fubTestShader_forward.uniform("uAnisotropic", uAnisotropic);
		fubTestShader_forward.uniform("uSheen", uSheen);
		fubTestShader_forward.uniform("uSheenTint", uSheenTint);
		fubTestShader_forward.uniform("uClearcoat", uClearcoat);
		fubTestShader_forward.uniform("uClearcoatGloss", uClearcoatGloss);

		sponzaModel.Draw(fubTestShader_forward);

		fubTestShader_forward.disable();
	}

	fboRgbDepth->Unbind();

	calculateSATforDoF();

	// apply depth of field
	{
		glClearColor(0.2f, 0.2f, 0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		fubTestShader_postproc.enable();

		glActiveTexture(GL_TEXTURE0);
		DoF_satResult->Bind();
		fubTestShader_postproc.uniform("sat", 0);

		glActiveTexture(GL_TEXTURE1);
		fboRgbDepth->bindTexture(0, 1);
		fubTestShader_postproc.uniform("color", 1);

		glActiveTexture(GL_TEXTURE2);
		fboRgbDepth->bindDepth(2);
		fubTestShader_postproc.uniform("depth", 2);

		fubTestShader_postproc.uniform("manualBlur", manualBlur);
		fubTestShader_postproc.uniform("blur", blurFactor);
		fubTestShader_postproc.uniform("focalPlane", focalPlane);
		fubTestShader_postproc.uniform("time", (float)timer.currentTime);

		fubTestShader_postproc.uniform("viewMatrix", viewMatrix);
		fubTestShader_postproc.uniform("projMatrix", projMatrix);

		int w = WIDTH, h = HEIGHT;
		glUniform2i(glGetUniformLocation(fubTestShader_postproc.ID, "res"), w, h);

		quad->draw();
	}

	gl_check_error("after renderFubTestScene");

	fubTestShader_postproc.disable();
	DoF_satResult->Unbind();
	fboRgbDepth->unbindTexture(0, 1);
	fboRgbDepth->unbindDepth(2);

	//standardMiniColorFboShader.enable();
	//DoF_satResult->Bind();
	//standardMiniColorFboShader.uniform("tex", 0);
	//standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	//standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	//quad->draw();
	//DoF_satResult->Unbind();
	//standardMiniColorFboShader.disable();
}

/* *********************************************************************************************************
Display + Main
********************************************************************************************************* */
void display() {
	//Timer
	timer.update();
	//FPS-Counter
	frame++;
	timeCounter = glutGet(GLUT_ELAPSED_TIME);
	if (timeCounter - timebase > 1000) {
		sprintf_s(timeString, "FPS:%4.2f", frame*1000.0 / (timeCounter - timebase));
		timebase = timeCounter;
		frame = 0;
		glutSetWindowTitle(timeString);
	}

	//OpenGL Clears
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	updateTweakBar();

	switch (current_Shader) {
		case STANDARD_SHADER:
			sponzaStandardScene();
			break;

		case SHADOW_MAP_SHADER:
			sponzaShadowMap();
			break;

		case VARIANCE_SHADOW_MAP_SHADER:
			sponzaVarianceShadowMap();
			break;

		case MOMENTS_SHADOW_MAP_SHADER:
			sponzaMomentsShadowMap();
			break;

		case DEBUG_SHADER:
			sponzaDebugScene();
			break;

		case FOG_SHADER:
			sponzaFogScene();
			break;

		case EXP_FOG_SHADER:
			sponzaFogExponentialScene();
			break;

		case FOG_SCREENSPACE_SHADER:
			sponzaFogScreenspaceScene();
			break;

		case FOG_SCREENSPACE_NOISE_SHADER:
			sponzaFogScreenspaceSceneNoise();
			break;

		case NOISE_SHADER:
			renderNoise();
			break;

		case DEPTH_OF_FIELD_SHADER:
			renderDofScene();
			break;

		case EMPTY_SHADER:
			break;

		case GAME_OF_LIFE_SHADER:
			renderGameOfLife();
			break;

		case MANDALA_SHADER:
			renderMandala();
			break;

		case INSTANCED_MESH_SHADER:
			renderInstancedMesh();
			break;

		case FUB_TEST_SHADER:
			renderFubTestScene();
			break;
	}

	TwDraw(); //Draw Tweak-Bar

	glutSwapBuffers();
	glutPostRedisplay();

}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);

	glutCreateWindow("Basic Framework");
	
	setupTweakBar();

	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Error : " << glewGetErrorString(err) << std::endl;
	}

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(onMouseMove);
	glutMouseFunc(onMouseDown);
	glutReshapeFunc(reshape);
	glutIdleFunc(onIdle);

	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT); //Type-Cast for VisualStudio
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); //Type-Cast for VisualStudio
	TwGLUTModifiersFunc(glutGetModifiers);

	initGL();

	init();
	initFBO();

	glutMainLoop();

	TwTerminate();
	return 0;
}










