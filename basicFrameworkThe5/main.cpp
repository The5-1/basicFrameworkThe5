#define GLEW_STATIC //Using the static lib, so we need to enable it

#include <iostream>
#include <GL/glew.h>
#include <GL/glut.h>

#include "helper.h"
#include "Shader.h"
#include "Model.h"

//Externals
cameraSystem cam(1.0f, 1.0f, glm::vec3(3.0f, 16.0f, 22.f));
glm::vec3 lightDir;
glm::mat4 projMatrix;
glm::mat4 viewMatrix;

//Test-Object
solidSphere *sphere = 0;
Shader basicShader;
Shader modelLoaderShader;

Model nanosuitModel;
Model sponzaModel;

//Model nanosuitModel(FileSystem::getPath("resources/objects/nanosuit/nanosuit.obj"));

void init() {
	//nanosuitModel = *(new Model("./data/Nanosuit/nanosuit.obj", false));
	nanosuitModel = *(new Model("C:/Dev/Assets/Nanosuit/nanosuit.obj", false));

	sponzaModel = *(new Model("C:/Dev/Assets/Sponza_Atrium/sponza.obj", false));

	sphere = new solidSphere(5, 20, 20);
	sphere->upload();
}

void loadShader(bool init) {
	basicShader = Shader("./shader/standard.vs.glsl", "./shader/standard.fs.glsl");
	modelLoaderShader = Shader("./shader/modelLoader.vs.glsl", "./shader/modelLoader.fs.glsl");
}

void display() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.2f, 0.2f, 1);

	//Basic-Sphere
	//basicShader.use();
	//basicShader.setVec3("col", glm::vec3(1.0f, 0.0f, 0.0f));
	//basicShader.setMat4("modelMatrix", glm::mat4(1.0f));
	//basicShader.setMat4("viewMatrix", viewMatrix);
	//basicShader.setMat4("projMatrix", projMatrix);
	//sphere->draw();

	//Nano-Suit
	modelLoaderShader.use();
	modelLoaderShader.setMat4("projection", projMatrix);
	modelLoaderShader.setMat4("view", viewMatrix);

	// render the loaded model
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); // translate it down so it's at the center of the scene
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
	modelLoaderShader.setMat4("model", model);
	nanosuitModel.Draw(modelLoaderShader);

	model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f)); 
	model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	
	modelLoaderShader.setMat4("model", model);
	modelLoaderShader.setMat4("model", model);
	sponzaModel.Draw(modelLoaderShader);


	glutSwapBuffers();
	glutPostRedisplay();

}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);

	glutCreateWindow("Basic Framework");

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

	//glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT); //Type-Cast for VisualStudio
	//glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT); //Type-Cast for VisualStudio
	//TwGLUTModifiersFunc(glutGetModifiers);

	initGL();

	init();

	glutMainLoop();
	return 0;
}










