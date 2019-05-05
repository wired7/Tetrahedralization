#pragma once
#include "GraphicsObject.h"

namespace Graphics
{
	class Triangle : public MeshObject
	{
	public:
		Triangle();
	};

	class Quad : public MeshObject
	{
	public:
		Quad();
		~Quad();
	};

	class Cylinder : public MeshObject
	{
	public:
		Cylinder(int resolution);
	};

	class Polyhedron : public MeshObject
	{
	public:
		int resolution;
		glm::vec3 radii;
		Polyhedron(int resolution, glm::vec3 pos, glm::vec3 radii);
	};

	class Tetrahedron : public MeshObject
	{
	public:
		Tetrahedron();
	};

	class Arrow : public MeshObject
	{
	public:
		Arrow();
	};
}