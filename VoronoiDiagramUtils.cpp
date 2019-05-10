#pragma once
#include "VoronoiDiagramUtils.h"
#include "ParametricShapes.h"
#include "HalfSimplexGeometryUtils.h"
#include "LinearAlgebraUtils.h"
#include <Eigen/Dense>

// TODO: Should generalize to N dimensions (N + 1 pts)
Parametric::Sphere VoronoiDiagramUtils::getCircumsphere(glm::vec3 points[4])
{
	Eigen::Matrix4d a;

	a << points[0].x, points[0].y, points[0].z, 1.0,
		 points[1].x, points[1].y, points[1].z, 1.0,
		 points[2].x, points[2].y, points[2].z, 1.0,
		 points[3].x, points[3].y, points[3].z, 1.0;

	double detA = a.determinant();

	double lengths[4];

	for (int j = 0; j < 4; j++)
	{
		lengths[j] = pow(length(points[j]), 2.0f);
	}

	Eigen::Matrix4d x[3];
	x[0] << lengths[0], points[0].y, points[0].z, 1.0f,
			lengths[1], points[1].y, points[1].z, 1.0f,
			lengths[2], points[2].y, points[2].z, 1.0f,
			lengths[3], points[3].y, points[3].z, 1.0f;

	x[1] << lengths[0], points[0].x, points[0].z, 1.0f,
			lengths[1], points[1].x, points[1].z, 1.0f,
			lengths[2], points[2].x, points[2].z, 1.0f,
			lengths[3], points[3].x, points[3].z, 1.0f;

	x[2] << lengths[0], points[0].x, points[0].y, 1.0f,
			lengths[1], points[1].x, points[1].y, 1.0f,
			lengths[2], points[2].x, points[2].y, 1.0f,
			lengths[3], points[3].x, points[3].y, 1.0f;

	Eigen::Vector3d detX(x[0].determinant(), -x[1].determinant(), x[2].determinant());
	detX /= (2.0 * detA);
	glm::vec3 center(detX[0], detX[1], detX[2]);

	double radius = glm::length(points[3] - center);

	return Parametric::Sphere(center, radius);
}

bool VoronoiDiagramUtils::isSpaceDegenerate(std::vector<glm::vec3> points)
{
	double orientedVolume = LinearAlgebraUtils::getSimplexOrientation<glm::vec3>(points);
	return orientedVolume > 0.0001 && orientedVolume < 0.0001;
}

Geometry::Manifold3<GLuint>* VoronoiDiagramUtils::delaunayTriangulation(std::vector<glm::vec3>& positions,
																		const std::vector<GLuint>& backgroundSimplexVertices)
{
	auto delaunayCriterion = [](Geometry::HalfSimplex<3, GLuint>* currentSimplex,
		std::vector<glm::vec3>& positions, glm::vec3 targetPoint) -> bool
	{
		std::vector<Geometry::TopologicalStruct*> vertexVec;
		std::unordered_set<Geometry::TopologicalStruct*> vertexSet;
		currentSimplex->getAllNthChildren(vertexVec, vertexSet, 0);
		std::vector<glm::vec3> simplexVertices;

		std::unordered_set<GLuint> vertexIndices;
		for (const auto& vertex : vertexVec)
		{
			auto index = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(vertex)->halfSimplexData;

			if (vertexIndices.insert(index).second)
			{
				simplexVertices.push_back(positions[index]);
			}
		}

		auto sphere = getCircumsphere(&(simplexVertices[0]));
		return length(targetPoint - sphere.center) <= sphere.radius;
	};

	auto contourCriterion = [](Geometry::HalfSimplex<3, GLuint>* currentSimplex) -> bool
	{
		std::vector<Geometry::TopologicalStruct*> facetVec;
		std::unordered_set<Geometry::TopologicalStruct*> facetSet;
		currentSimplex->getAllChildren(facetVec, facetSet);

		for (const auto& facet : facetVec)
		{
			if (reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet)->twin == nullptr)
			{
				return true;
			}
		}

		return false;
	};

	Geometry::Manifold3<GLuint>* manifold = new Geometry::Manifold3<GLuint>();

	manifold->add3(std::make_pair(backgroundSimplexVertices, nullptr));

	std::queue<GLuint> insertionIndices;

	for (GLuint i = 0; i < positions.size() - 4; ++i)
	{
		insertionIndices.push(i);
	}

	std::map<GLuint, int> deferredIndices;

	while (!insertionIndices.empty())
	{
		GLuint i = insertionIndices.front();
		insertionIndices.pop();

		glm::vec3 targetPoint = positions[i];
		auto currentSimplex0 = manifold->map3.begin()->second;
		Geometry::HalfSimplex<3, GLuint>* targetSimplex =
			HalfSimplexGeometryUtils::findSimplex<3, glm::vec3>(currentSimplex0, positions, targetPoint, delaunayCriterion);

		std::map<std::vector<GLuint>, std::vector<std::vector<GLuint>>> facetMap;
		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> visitedSimplices;
		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> simplicesToErase;
		std::queue<Geometry::HalfSimplex<3, GLuint>*> simplexQueue;
		simplexQueue.push(targetSimplex);
		bool failed = currentSimplex0 == nullptr;

		while (!failed && !simplexQueue.empty())
		{
			auto currentSimplex = simplexQueue.front();
			simplexQueue.pop();

			if (visitedSimplices.find(currentSimplex) != visitedSimplices.end())
			{
				continue;
			}

			visitedSimplices.insert(currentSimplex);

			std::vector<Geometry::TopologicalStruct*> targetSimplexVerticesVec;
			std::unordered_set<Geometry::TopologicalStruct*> targetSimplexVerticesSet;
			currentSimplex->getAllNthChildren(targetSimplexVerticesVec, targetSimplexVerticesSet, 0);

			std::unordered_set<GLuint> targetSimplexIndices;
			std::vector<glm::vec3> simplexPositions;
			for (const auto& targetSimplexVertex : targetSimplexVerticesVec)
			{
				auto index = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(targetSimplexVertex)->halfSimplexData;

				if (targetSimplexIndices.insert(index).second)
				{
					simplexPositions.push_back(positions[index]);
				}
			}

			auto sphere = getCircumsphere(&(simplexPositions[0]));
			auto distance = glm::length(positions[i] - sphere.center);

			if (distance > (sphere.radius - 0.00001) && distance < (sphere.radius + 0.00001))
			{
				failed = true;
				break;
			}

			if (distance >(sphere.radius + 0.00001))
			{
				continue;
			}

			simplicesToErase.insert(currentSimplex);

			std::vector<Geometry::TopologicalStruct*> targetSimplexFacetsVec;
			std::unordered_set<Geometry::TopologicalStruct*> targetSimplexFacetsSet;
			currentSimplex->getAllChildren(targetSimplexFacetsVec, targetSimplexFacetsSet);

			for (const auto& facet : targetSimplexFacetsVec)
			{
				auto halfFacet = reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet);

				if (halfFacet->twin != nullptr)
				{
					simplexQueue.push(halfFacet->twin->belongsTo);
				}

				std::vector<Geometry::TopologicalStruct*> facetVertexVec;
				std::unordered_set<Geometry::TopologicalStruct*> facetVertexSet;
				facet->getAllNthChildren(facetVertexVec, facetVertexSet, 0);

				// obtain facet indices
				std::unordered_set<GLuint> facetIndicesSet;
				std::vector<GLuint> facetIndices;
				for (const auto& facetIndex : facetVertexVec)
				{
					auto index = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(facetIndex)->halfSimplexData;

					if (facetIndicesSet.insert(index).second)
					{
						facetIndices.push_back(index);
					}
				}

				auto sortedFacetIndices = facetIndices;
				std::sort(sortedFacetIndices.begin(), sortedFacetIndices.end());

				facetMap[sortedFacetIndices].push_back(Geometry::arrangeSimplexIndices<GLuint>(facetIndices));

				if (facetMap[sortedFacetIndices].size() > 1)
				{
					facetMap.erase(sortedFacetIndices);
				}
			}
		}

		if (!failed)
		{
			for (auto& facet : facetMap)
			{
				facet.second[0].push_back(i);

				std::vector<glm::vec3> simplexPositions;
				for (const auto& index : facet.second[0])
				{
					simplexPositions.push_back(positions[index]);
				}

				if (isSpaceDegenerate(simplexPositions))
				{
					failed = true;
					break;
				}
			}
		}

		if (failed)
		{
			if (deferredIndices[i] < 2)
			{
				insertionIndices.push(i);
			}

			deferredIndices[i]++;
			continue;
		}

		for (const auto& simplex : simplicesToErase)
		{
			manifold->erase3(simplex);
		}

		for (auto& facet : facetMap)
		{
			auto newTetra = Geometry::arrangeSimplexIndices<GLuint>(facet.second[0]);
			manifold->add3(std::make_pair(newTetra, nullptr));
		}
	}

	return manifold;
}