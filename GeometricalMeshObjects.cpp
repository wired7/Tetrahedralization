#pragma once
#include "GeometricalMeshObjects.h"
#include <gtc/matrix_transform.hpp>
#include <gtx/rotate_vector.hpp>

namespace Graphics
{
	Triangle::Triangle() : MeshObject()
	{
		addVertex(glm::vec3(-1, 0, 0), glm::vec3(0, 0, 1));
		addVertex(glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		addVertex(glm::vec3(1, 0, 0), glm::vec3(0, 0, 1));

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);

		bindBuffers();
	};

	Quad::Quad() : MeshObject()
	{
		addVertex(glm::vec3(-1, -1, 0), glm::vec3());
		addVertex(glm::vec3(1, -1, 0), glm::vec3());
		addVertex(glm::vec3(-1, 1, 0), glm::vec3());
		addVertex(glm::vec3(1, 1, 0), glm::vec3());

		indices.push_back(0);
		indices.push_back(1);
		indices.push_back(2);

		indices.push_back(2);
		indices.push_back(1);
		indices.push_back(3);

		bindBuffers();
	};

	Quad::~Quad() {};

	Cylinder::Cylinder(int resolution) : MeshObject()
	{
		float angleD = 2 * 3.1415f / resolution;

		for (int j = 0; j < 2; j++)
		{
			glm::vec3 center(j, 0, 0);

			for (int i = 0; i < resolution; i++)
			{
				glm::vec3 normal(0, sin(angleD * i), cos(angleD * i));
				glm::vec3 pos = center + normal;
				addVertex(pos, normal);

				if (j > 0)
				{
					indices.push_back(vertices.size() - resolution);
					indices.push_back(vertices.size() - 1);
					indices.push_back(vertices.size() - resolution - 1);

					indices.push_back(vertices.size() - resolution - 1);
					indices.push_back(vertices.size() - 1);
					indices.push_back(vertices.size() - 2);
				}
			}
		}

		bindBuffers();
	};

	Polyhedron::Polyhedron(int resolution, glm::vec3 pos, glm::vec3 radii) : MeshObject()
	{
		model = scale(translate(glm::mat4(1.0f), pos), radii);
		this->resolution = resolution;

		double theta = 2 * 3.1415 / resolution;
		double phi = 3.1415 / resolution;
		std::vector<std::vector<glm::vec3>> circles;
		glm::vec3 start(0, -0.5f, 0);
		for (int j = 1; j < resolution; j++)
		{
			glm::vec3 temp = rotate(start, (GLfloat)(phi * j), glm::vec3(0, 0, 1));
			circles.push_back(std::vector<glm::vec3>());
			for (int i = 0; i < resolution; i++)
				circles[circles.size() - 1].push_back(rotate(temp, (GLfloat)(theta * i), glm::vec3(0, 1, 0)));
		}

		for (int j = 0; j < circles.size(); j++)
		{
			for (int i = 0; i < circles[j].size(); i++)
			{
				addVertex(circles[j][i], normalize(circles[j][i]));

				if (j > 0 && i > 0)
				{
					indices.push_back(vertices.size() - 2 - circles[j].size());
					indices.push_back(vertices.size() - 1 - circles[j].size());
					indices.push_back(vertices.size() - 2);

					indices.push_back(vertices.size() - 2);
					indices.push_back(vertices.size() - 1 - circles[j].size());
					indices.push_back(vertices.size() - 1);
				}
			}

			if (j > 0)
			{
				indices.push_back(vertices.size() - 1 - circles[j].size());
				indices.push_back(vertices.size() - 2 * circles[j].size());
				indices.push_back(vertices.size() - 1);

				indices.push_back(vertices.size() - 1);
				indices.push_back(vertices.size() - 2 * circles[j].size());
				indices.push_back(vertices.size() - circles[j].size());
			}
		}

		addVertex(start, start);
		addVertex(-start, -start);

		for (int i = 0; i < resolution; i++)
		{
			indices.push_back(i);
			indices.push_back(vertices.size() - 2);
			indices.push_back((i + 1) % resolution);

			indices.push_back(vertices.size() - 2 - resolution + i);
			indices.push_back(vertices.size() - resolution + (i + 1) % resolution - 2);
			indices.push_back(vertices.size() - 1);
		}

		bindBuffers();
	};

	Tetrahedron::Tetrahedron() : MeshObject()
	{
		model = scale(translate(glm::mat4(1.0f), glm::vec3()), glm::vec3(1, 1, 1));
		glm::vec3 v1(-1, 0, 0);
		glm::vec3 v2(1, 0, 0);
		glm::vec3 v3(0, 1, 0);
		glm::vec3 v4(0, 0, 1);

		glm::vec3 n1 = -glm::normalize(glm::cross(glm::normalize(v2 - v1), glm::normalize(v3 - v2)));
		addVertex(v1, n1);
		addVertex(v2, n1);
		addVertex(v3, n1);
		addVertex(v4, n1);

		glm::vec3 centroid(0, 0, 0);
		for (int i = 0; i < vertices.size(); i++)
		{
			centroid += vertices[i].position;
		}

		centroid /= vertices.size();

		for (int i = 0; i < vertices.size(); i++)
		{
			vertices[i].normal = normalize(vertices[i].position - centroid);
		}

//		indices = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
		indices = { 1, 3, 0, 0, 2, 1, 0, 3, 2, 1, 2, 3 };

		bindBuffers();
	};

	Arrow::Arrow()
	{
		int resolution = 4;
		float endDistance = 0.8f;

		float angleD = 2 * 3.1415f / resolution;

		for (int j = 0; j < 2; j++)
		{
			glm::vec3 center(j * endDistance, 0, 0);

			for (int i = 0; i < resolution; i++)
			{
				glm::vec3 normal(0, sin(angleD * i), cos(angleD * i));
				glm::vec3 pos = center + normal;
				addVertex(pos, normal);

				if (j > 0)
				{
					indices.push_back(vertices.size() - resolution);
					indices.push_back(vertices.size() - 1);
					indices.push_back(vertices.size() - resolution - 1);

					indices.push_back(vertices.size() - resolution - 1);
					indices.push_back(vertices.size() - 1);
					indices.push_back(vertices.size() - 2);
				}
			}
		}

		glm::vec3 center(endDistance, 0, 0);
		int centerIndex = vertices.size();
		addVertex(center, glm::vec3(-1, 0, 0));

		for (int i = 0; i <= resolution; i++)
		{
			glm::vec3 normal(0, sin(angleD * i), cos(angleD * i));
			glm::vec3 pos = center + normal * 4.0f;
			addVertex(pos, normal);

			if (i >= 1)
			{
				indices.push_back(centerIndex);
				indices.push_back(vertices.size() - 1);
				indices.push_back(vertices.size() - 2);
			}
		}

		center = glm::vec3(1.0f);
		centerIndex = vertices.size();
		addVertex(center, glm::vec3(1.0f, 0, 0));

		for (int i = 0; i <= resolution; i++)
		{
			indices.push_back(centerIndex);
			indices.push_back(vertices.size() - i - 2);
			indices.push_back(vertices.size() - i - 1);
		}

		indices.push_back(centerIndex);
		indices.push_back(vertices.size() - 1);
		indices.push_back(centerIndex + 1);

		bindBuffers();
	};
};