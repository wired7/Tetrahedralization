#pragma once
#include "DiscreteGeometryUtils.h"
#include "GraphicsObject.h"
#include "HalfSimplexGeometryUtils.h"

namespace DiscreteGeometryUtils
{
	std::vector<Parametric::Triangle> getTrianglesFromMesh(Graphics::DecoratedGraphicsObject* object)
	{
		auto vertexBuffer = (Graphics::MeshObject*)object->signatureLookup("VERTEX");
		auto vertices = vertexBuffer->vertices;
		auto indices = vertexBuffer->indices;
		auto transformBuffer = (Graphics::MatrixInstancedMeshObject<glm::mat4, float>*)object->signatureLookup("TRANSFORM");
		auto transforms = transformBuffer->extendedData;

		std::vector<Parametric::Triangle> triangles;

		for (int i = 0; i < transforms.size(); i++)
		{
			glm::mat4 t = transforms[i];
			std::vector<glm::vec3> transformedPts;

			for (int j = 0; j < vertices.size(); j++)
			{
				transformedPts.push_back(glm::vec3(t * glm::vec4(vertices[j].position, 1)));
			}

			for (int j = 0; j < indices.size(); j += 3)
			{
				triangles.push_back(Parametric::Triangle(transformedPts[indices[j]], transformedPts[indices[j + 1]], transformedPts[indices[j + 2]]));
			}
		}

		return triangles;
	};

	std::pair<glm::vec3, glm::vec3> getBoundingBox(Graphics::DecoratedGraphicsObject* object)
	{
		auto vertexBuffer = (Graphics::MeshObject*)object->signatureLookup("VERTEX");
		auto vertices = vertexBuffer->vertices;
		auto transformBuffer = (Graphics::MatrixInstancedMeshObject<glm::mat4, float>*)object->signatureLookup("TRANSFORM");
		auto transforms = transformBuffer->extendedData;

		glm::vec3 minN(INFINITY);
		glm::vec3 maxN(-INFINITY);

		for (int i = 0; i < transforms.size(); i++)
		{
			glm::mat4 t = transforms[i];
			std::vector<glm::vec3> transformedPts;

			for (int j = 0; j < vertices.size(); j++)
			{
				transformedPts.push_back(glm::vec3(t * glm::vec4(vertices[j].position, 1)));

				for (int k = 0; k < 3; k++)
				{
					if (transformedPts[transformedPts.size() - 1][k] < minN[k])
					{
						minN[k] = transformedPts[transformedPts.size() - 1][k];
					}
					else if (transformedPts[transformedPts.size() - 1][k] > maxN[k])
					{
						maxN[k] = transformedPts[transformedPts.size() - 1][k];
					}
				}
			}
		}

		return std::make_pair(minN, maxN);
	};

	bool isPointInsideMesh(glm::vec3 point, const std::vector<Parametric::Triangle>& triangles)
	{
		glm::vec3 dir(1, 0, 0);
		int sum = 0;
		for (int j = 0; j < triangles.size(); j++)
		{
			if (triangles[j].intersection(point, dir) > 0.0f)
			{
				sum++;
			}
		}

		return sum % 2 == 1;
	};

	void constrainTriangulation(Geometry::Manifold3<GLuint>* manifold, const std::vector<glm::vec3>& positions,
		const std::vector<Parametric::Triangle>& surfaceTriangles)
	{
		std::vector<Geometry::HalfSimplex<3, GLuint>*> tetraVec;
		tetraVec.resize(manifold->map3.size());
		int count = 0;
		for (const auto& tetra : manifold->map3)
		{
			tetraVec[count++] = tetra.second;
		}

std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> simplicesToRemove;
#pragma omp parallel for schedule(dynamic, 1000)
for (int i = 0; i < tetraVec.size(); ++i)
{
	auto currentSimplex = tetraVec[i];

	std::vector<Geometry::TopologicalStruct*> vertexVec;
	std::unordered_set<Geometry::TopologicalStruct*> vertexSet;
	currentSimplex->getAllNthChildren(vertexVec, vertexSet, 0);

	std::vector<glm::vec3> simplexVertices;
	std::unordered_set<GLuint> simplexVertexIndices;
	for (const auto& vertex : vertexVec)
	{
		auto simplexVertexIndex = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(vertex)->halfSimplexData;

		if (simplexVertexIndices.insert(simplexVertexIndex).second)
		{
			simplexVertices.push_back(positions[simplexVertexIndex]);
		}
	}

	bool toBeRemoved = false;

	glm::vec3 centroid = HalfSimplexGeometryUtils::getSimplexCentroid(currentSimplex, positions);

	if (!DiscreteGeometryUtils::isPointInsideMesh(centroid, surfaceTriangles))
	{
		toBeRemoved = true;
	}
	else
	{
		std::vector<glm::vec3> halfwayVertices;
		for (const auto& vertex : simplexVertices)
		{
			auto halfwayVertex = (centroid + vertex) / 2.0f;
			if (!DiscreteGeometryUtils::isPointInsideMesh(halfwayVertex, surfaceTriangles))
			{
				toBeRemoved = true;
				break;
			}

			halfwayVertices.push_back(halfwayVertex);
		}
	}

	if (toBeRemoved)
	{
#pragma omp critical
		{
			simplicesToRemove.insert(currentSimplex);
		}
	}
}

for (auto& tetra : simplicesToRemove)
{
	manifold->erase3(tetra);
}
	};

	void getConvexHullFromManifold(Geometry::Manifold3<GLuint>* manifold,
		const std::vector<GLuint>& backgroundSimplexVertices)
	{
		//TODO: This can be optimized by shaving off simplices from contour towards the inside
		// Also, OMP can be of great help with the current method

		std::vector<Geometry::HalfSimplex<3, GLuint>*> tetraVec;
		tetraVec.resize(manifold->map3.size());
		int count = 0;
		for (const auto& tetra : manifold->map3)
		{
			tetraVec[count++] = tetra.second;
		}

		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> backgroundSimplexTetras;
#pragma omp parallel for schedule(dynamic, 1000)
		for (int i = 0; i < tetraVec.size(); ++i)
		{
			auto tetra = tetraVec[i];
			for (const auto& tetraVertex : tetra->fullSimplex->data)
			{
				if (std::find(backgroundSimplexVertices.begin(), backgroundSimplexVertices.end(), tetraVertex) != backgroundSimplexVertices.end())
				{
#pragma omp critical
					{
						backgroundSimplexTetras.insert(tetra);
					}
					break;
				}
			}
		}

		for (const auto& tetra : backgroundSimplexTetras)
		{
			manifold->erase3(tetra);
		}
	};
}
