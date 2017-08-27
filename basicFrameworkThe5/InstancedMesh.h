#pragma once
#include <GL/glew.h>
#include <GL/glut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mathConstants.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

//#include <stdlib.h>     /* srand, rand */
//#include <time.h>       /* time */

/* *********************************************************************************************************
ToDo:
Currently only for spheres!!!
********************************************************************************************************* */

class InstancedMesh {
public:
	int particleCount = 1000;
	vector<unsigned int> indices;
	vector<glm::vec3> vertices;
	vector<glm::vec2> uvs;
	vector<glm::vec3> worldVertexPosition;
	GLuint vbo[4];

public:
	/* *********************************************************************************************************
	Sphere
	********************************************************************************************************* */
	void createModelMatrix_BoxFormation() {
		glm::mat4 modelMatrix;
		float distance = 5.0f;

		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				for (int k = 0; k < 10; k++) {
					//modelMatrix = glm::translate(modelMatrix, glm::vec3((float)i * distance, (float)j * distance, (float)k * distance));
					//worldVertexPosition.push_back(modelMatrix);
					worldVertexPosition.push_back(glm::vec3((float)i * distance, (float)j * distance, (float)k * distance));
				}
			}
		}
	}

	void createSphere(const float r, const int slices, const int stacks) {
		const float dTheta = 2.0 * M_PI / (float)stacks;
		const float dPhi = M_PI / (float)slices;

		for (int i = stacks; i >= 0; i--) {
			glm::vec2 t(1 - i*dTheta / (2.0*M_PI), 1.0f);
			glm::vec3 p(0, r, 0);
			vertices.push_back(p);
			uvs.push_back(t);
		}

		const int tmpSize = vertices.size();

		//North pole
		for (int i = stacks; i >= 0; i--) {
			glm::vec2 t(1 - i*dTheta / (2.0*M_PI), (M_PI - dPhi) / M_PI);
			glm::vec3 p(r*sin(dPhi)*cos(i*dTheta), r*cos(dPhi), r*sin(dPhi)*sin(i*dTheta));
			vertices.push_back(p);
			uvs.push_back(t);
		}

		int triangleID = 0;
		for (; triangleID < stacks; triangleID++)
		{
			indices.push_back(triangleID);
			indices.push_back(triangleID + tmpSize);
			indices.push_back(triangleID + tmpSize + 1);
		}

		indices.push_back(stacks - 1);
		indices.push_back(stacks * 2 - 1);
		indices.push_back(stacks);


		// Middle Part 

		//	v0--- v2
		//  |  	/ |
		//  |  /  | 
		//  | /   |
		//  v1--- v3

		for (int j = 1; j<slices - 1; j++)
		{
			for (int i = stacks; i >= 0; i--)
			{
				glm::vec2 t = glm::vec2(1 - i*dTheta / (2.0*M_PI), (M_PI - (j + 1)*dPhi) / M_PI);
				glm::vec3 p = glm::vec3(r*sin((j + 1)*dPhi)*cos(i*dTheta), r*cos((j + 1)*dPhi), r*sin((j + 1)*dPhi)*sin(i*dTheta));
				vertices.push_back(p);
				uvs.push_back(t);

				//add two triangles

				indices.push_back(vertices.size() - stacks - 2);	//v0
				indices.push_back(vertices.size() - 1);			//v1
				indices.push_back(vertices.size() - stacks - 1);	//v2

				indices.push_back(vertices.size() - stacks - 1);	//v2
				indices.push_back(vertices.size() - 1);			//v1
				indices.push_back(vertices.size());			//v3

			}

		}

		const int lastVertex = vertices.size() - 1;

		//South Pole
		for (int i = stacks; i >= 0; i--) {
			glm::vec2 t(1 - i*dTheta / (2.0*M_PI), 0.0f);
			glm::vec3 p = glm::vec3(0, -r, 0);
			vertices.push_back(p);
			uvs.push_back(t);
		}

		triangleID = 0;
		for (; triangleID < stacks; triangleID++)
		{
			indices.push_back(lastVertex - stacks + triangleID);
			indices.push_back(lastVertex + triangleID + 1);
			indices.push_back(lastVertex - stacks + triangleID + 1);
		}


		createModelMatrix_BoxFormation();
	
	}

	void uploadSphere() {
		createSphere(1.0, 10, 10);

		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) * 3, vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(float) * 2, uvs.data(), GL_STATIC_DRAW);

		//////////////////////////////////////
		// GL_DYNAMIC_COPY so it can be changed in compute shader
		//////////////////////////////////////
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, worldVertexPosition.size() * sizeof(float) * 3, &worldVertexPosition[0], GL_DYNAMIC_COPY);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	}

	void drawSphere() {
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribDivisor(2, 1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[3]);

		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, particleCount);
	}
};