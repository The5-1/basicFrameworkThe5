#define GLEW_STATIC //Using the static lib, so we need to enable it
#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>
#include <Ant/AntTweakBar.h>

#include "helper.h"
#include "Shader.h"
#include "Model.h"
#include "Skybox.h"
#include "times.h"

#include "FBO.h"
#include "Texture.h"

//Time
Timer timer;

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
glm::vec3 lightDir;

//Test-Object
solidSphere *sphere = 0;
simpleQuad * quad = 0;
simpleCube * cube = 0;

Mesh quadMesh;
Mesh boxMesh;

//Skybox
Skybox skybox;
char* negz = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negz.jpg";
char* posz = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posz.jpg";
char* posy = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posy.jpg";
char* negy = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negy.jpg";
char* negx = "C:/Dev/Assets/SkyboxTextures/Yokohama2/negx.jpg";
char* posx = "C:/Dev/Assets/SkyboxTextures/Yokohama2/posx.jpg";

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

//Debug-Shader
Shader quadScreenSizedShader;
Shader standardMiniColorFboShader;
Shader standardMiniDepthFboShader;

//Models
Model nanosuitModel;
Model sponzaModel;

//Frame buffer object
FBO *fbo = 0;
FBO *fboDepthShadowMap = 0;
FBO *fboMomentsShadowMap = 0;

//Tweakbar
TwBar *tweakBar;
enum SHADER_TYPE { STANDARD_SHADER,
				SHADOW_MAP_SHADER,
				MOMENTS_SHADOW_MAP_SHADER,
				DEBUG_SHADER, 
				FOG_SHADER, 
				EXP_FOG_SHADER, 
				FOG_SCREENSPACE_SHADER, 
				FOG_SCREENSPACE_NOISE_SHADER, 
				NOISE_SHADER};

SHADER_TYPE current_Shader = MOMENTS_SHADOW_MAP_SHADER;

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

//Helper-Tweakbar-Button
bool helperBoolButton = false;

void setupTweakBar() {
	// Tweakbar
	TwInit(TW_OPENGL_CORE, NULL);
	tweakBar = TwNewBar("Settings");

	// Array of drop down items
	TwEnumVal Shader_array[] = { { STANDARD_SHADER, "Standard" }, 
									{SHADOW_MAP_SHADER, "Shadow Map" },
									{MOMENTS_SHADOW_MAP_SHADER , "Variance Shadow Map"},
									{DEBUG_SHADER, "Debug" },
									{FOG_SHADER, "Fog" },
									{EXP_FOG_SHADER, "Exponential fog" },
									{FOG_SCREENSPACE_SHADER, "Screenspace Fog"}, 
									{FOG_SCREENSPACE_NOISE_SHADER, "Screenspace Noise Fog" },
									{NOISE_SHADER, "Noise Textures"} };

	// ATB identifier for the array
	TwType ShaderTwType = TwDefineEnum("Shader: ", Shader_array, 9);

	// Link it to the tweak bar
	TwAddVarRW(tweakBar, "Shader", ShaderTwType, &current_Shader, NULL);

	//Seperator
	TwAddSeparator(tweakBar, "", NULL);

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

	//Helper-Tweakbar-Button
	TwAddVarRW(tweakBar, "helperBoolButton", TW_TYPE_BOOLCPP, &helperBoolButton, " label='Set Light to Cam' ");
}

void updateTweakBar() {
	//Deactivate everything in Tweak-Bar
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

	//Helper-Tweakbar-Button
	TwDefine("Settings/helperBoolButton visible=false");
	//Only show what we need
	switch (current_Shader) {
		case STANDARD_SHADER: 
			break;

		case SHADOW_MAP_SHADER:
			TwDefine("Settings/helperBoolButton visible=true");
			break;

		case MOMENTS_SHADOW_MAP_SHADER:
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
	}
}

void init() {
	//Simple-Model
	quad = new simpleQuad();
	quad->upload();

	cube = new simpleCube();
	cube->upload();

	quadMesh.createScreenQuad();
	boxMesh.createBox();

	//Komplex-Model
	//nanosuitModel = *(new Model("C:/Dev/Assets/Nanosuit/nanosuit.obj", false));
	sponzaModel = *(new Model("C:/Dev/Assets/Sponza_Atrium/sponza.obj", false));

	//Skybox
	skybox.createSkybox(negz, posz, posy, negy, negx, posx);
}

void initFBO() {
	fbo = new FBO("Gbuffer", WIDTH, HEIGHT, FBO_GBUFFER_32BIT);
	gl_check_error("fbo 1");

	fboDepthShadowMap = new FBO("DepthShadowMap", SHADOW_WIDTH, SHADOW_HEIGHT, FBO_DEPTH_16BIT);
	gl_check_error("post DepthShadowMap");

	fboMomentsShadowMap = new FBO("MomentsShadowMap", SHADOW_WIDTH, SHADOW_HEIGHT, FBO_RGB_DEPTH_16BIT);
	gl_check_error("MomentsShadowMap");
}

void loadShader(bool init) {
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

	//FBO-Shader
	depthOnlyShader = Shader("./shader/depthOnly.vs.glsl", "./shader/depthOnly.fs.glsl");
}

/* *********************************************************************************************************
Sponza Scenes
********************************************************************************************************* */
void sponzaStandardScene(){
	skyboxShader.enable();
	skyboxShader.uniform("projMatrix", projMatrix);
	skyboxShader.uniform("viewMatrix", cam.cameraRotation);
	skybox.Draw(skyboxShader);
	skyboxShader.disable();

	glm::mat4 modelMatrix;
	modelLoaderShader.enable();
	modelLoaderShader.uniform("projection", projMatrix);
	modelLoaderShader.uniform("view", viewMatrix);
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

void sponzaMomentsShadowMap(){
	//Fill FBO
	fboMomentsShadowMap->Bind();

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

	sponzaModel.Draw(msm_depthShader);

	msm_depthShader.disable();
	fboMomentsShadowMap->Unbind();	

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
	msm_shadowShader.enable();
	msm_shadowShader.uniform("uMatP", projMatrix);
	msm_shadowShader.uniform("uMatV", viewMatrix);
	msm_shadowShader.uniform("uMatM", modelMatrix);
	msm_shadowShader.uniform("uMatVP_Light", lightSpaceMatrix);

	fboMomentsShadowMap->bindTexture(0, 3);
	msm_shadowShader.uniform("shadowMap_moments", 3);

	fboMomentsShadowMap->bindDepth(4);
	msm_shadowShader.uniform("shadowMap_depth", 4);

	msm_shadowShader.uniform("lightPos", light.lightPosition);
	msm_shadowShader.uniform("viewPos", glm::vec3(cam.position.x, cam.position.y, cam.position.z));
	sponzaModel.Draw(msm_shadowShader);
	msm_shadowShader.disable();

	//Debug Textures
	standardMiniColorFboShader.enable();
	fboMomentsShadowMap->bindTexture(0);
	standardMiniColorFboShader.uniform("tex", 0);
	standardMiniColorFboShader.uniform("downLeft", glm::vec2(0.7f, 0.7f));
	standardMiniColorFboShader.uniform("upRight", glm::vec2(1.0f, 1.0f));
	quad->draw();
	fboMomentsShadowMap->unbindTexture(0);
	standardMiniColorFboShader.disable();

	standardMiniDepthFboShader.enable();
	fboMomentsShadowMap->bindDepth();
	standardMiniDepthFboShader.uniform("tex", 0);
	standardMiniDepthFboShader.uniform("downLeft", glm::vec2(0.7f, 0.4f));
	standardMiniDepthFboShader.uniform("upRight", glm::vec2(1.0f, 0.7f));
	quad->draw();
	fboMomentsShadowMap->unbindDepth();
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
Render Effects
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

void display() {
	//Timer update
	timer.update();

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










