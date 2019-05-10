#pragma once
#include "ParametricShapes.h"
#include <vector>
#include <utility>
#include <glew.h>
#include <glm.hpp>

namespace Graphics
{
	class DecoratedGraphicsObject;
};

namespace Geometry
{
	template <typename T> class Manifold3;
};

namespace DiscreteGeometryUtils
{
	std::vector<Parametric::Triangle> getTrianglesFromMesh(Graphics::DecoratedGraphicsObject* object);
	std::pair<glm::vec3, glm::vec3> getBoundingBox(Graphics::DecoratedGraphicsObject* object);
	bool isPointInsideMesh(glm::vec3 point, const std::vector<Parametric::Triangle>& triangles);
	void getConvexHullFromManifold(Geometry::Manifold3<GLuint>* manifold,
										const std::vector<GLuint>& backgroundSimplexVertices);
	void constrainTriangulation(Geometry::Manifold3<GLuint>* manifold, const std::vector<glm::vec3>& positions,
								const std::vector<Parametric::Triangle>& surfaceTriangles);
}

