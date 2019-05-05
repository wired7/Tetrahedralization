#pragma once
#include "TetrahedralizationContext.h"
#include "GeometricalMeshObjects.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"
#include "HalfSimplexGeometryUtils.h"
#include "FPSCameraControls.h"
#include "VoronoiDiagramUtils.h"
#include "LinearAlgebraUtils.h"
#include "DiscreteGeometryUtils.h"
#include "Pass.h"
#include <algorithm>    // std::sort
#include <map>
#include <unordered_set>
#include <limits>
#include <omp.h>
#include <glew.h>

TetrahedralizationContext::TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface,
													 Graphics::DecoratedGraphicsObject* points,
													 std::vector<glm::vec3>& _points,
													 FPSCamera* cam,
													 Graphics::ReferenceManager* refMan) : positions(_points), refMan(refMan)
{
	cameras.push_back(cam);
	geometries["SURFACE"] = surface;
	geometries["POINTS"] = points;
	setupGeometries();
	setupPasses({ "A", "A2", "EdgeA", "C", "D" }, { "B" });

	std::vector<ImplicitGeo::Triangle> surfaceTriangles = DiscreteGeometryUtils::getTrianglesFromMesh(surface);

	std::thread t([=]
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

			auto sphere = VoronoiDiagramUtils::getCircumsphere(&(simplexVertices[0]));
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

		manifold = new Geometry::Manifold3<GLuint>();

		positions.push_back(glm::vec3(-100000));
		positions.push_back(glm::vec3(0, 100000, 0));
		positions.push_back(glm::vec3(1, -1, -1) * 100000.0f);
		positions.push_back(glm::vec3(0, 0, 100000));

		std::vector<GLuint> backgroundSimplexVertices = { static_cast<GLuint>(positions.size() - 4),
											 static_cast<GLuint>(positions.size() - 3),
											 static_cast<GLuint>(positions.size() - 2), 
											 static_cast<GLuint>(positions.size() - 1) };

		manifold->add3(std::make_pair(backgroundSimplexVertices, nullptr));

		std::queue<GLuint> insertionIndices;

		for (GLuint i = 4; i < positions.size(); ++i)
		{
			insertionIndices.push(i);
		}

		std::map<GLuint, int> deferredIndices;

		while(!insertionIndices.empty())
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

				auto sphere = VoronoiDiagramUtils::getCircumsphere(&(simplexPositions[0]));
				auto distance = glm::length(positions[i] - sphere.center);

				if (distance > (sphere.radius - 0.00001) && distance < (sphere.radius + 0.00001))
				{
					failed = true;
					break;
				}

				if (distance > (sphere.radius + 0.00001))
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

					if (VoronoiDiagramUtils::isSpaceDegenerate(&(simplexPositions[0])))
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

		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> backgroundSimplexTetras;
		for (const auto& tetra : manifold->map3)
		{
			for (const auto& tetraVertex : tetra.first.first)
			{
				if (std::find(backgroundSimplexVertices.begin(), backgroundSimplexVertices.end(), tetraVertex) != backgroundSimplexVertices.end())
				{
					backgroundSimplexTetras.insert(tetra.second);
					break;
				}
			}
		}

		for (const auto& tetra : backgroundSimplexTetras)
		{
			manifold->erase3(tetra);
		}

		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> simplicesToRemove;

		for(const auto& tetra : manifold->map3)
		{
			auto currentSimplex = tetra.second;

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

/*				if (!toBeRemoved)
				{
					for (int j = 0; j < 3 && !toBeRemoved; ++j)
					{
						for (int k = j; k < 4 && !toBeRemoved; ++k)
						{
							if (!DiscreteGeometryUtils::isPointInsideMesh((halfwayVertices[j] + halfwayVertices[k]) / 2.0f, surfaceTriangles))
							{
								toBeRemoved = true;
								break;
							}
						}
					}
				}*/
			}

/*			if (!toBeRemoved)
			{
				std::vector<TopologicalStruct*> facetVec;
				std::unordered_set<TopologicalStruct*> facetxSet;
				currentSimplex->getAllChildren(facetVec, facetxSet);

				for (const auto& facet : facetVec)
				{
					auto simplexFacet = reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet);

					if (simplexFacet->twin == simplexFacet)
					{
						toBeRemoved = true;
						break;
					}
				}
			}*/

			if (toBeRemoved)
			{
				simplicesToRemove.insert(currentSimplex);
			}
		}

		for (auto& tetra : simplicesToRemove)
		{
			manifold->erase3(tetra);
		}

		glm::vec3 centroid = (glm::vec3(-1, 0, 0) + glm::vec3(1, 0, 0) + glm::vec3(0, 1, 0) + glm::vec3(0, 0, 1)) / 4.0f;
		glm::vec4 v1(-1, 0, 0, 1);
		glm::vec4 v2(1, 0, 0, 1);
		glm::vec4 v3(0, 1, 0, 1);
		glm::vec4 v4(0, 0, 1, 1);

		for (auto& simplex : manifold->map3)
		{
			std::vector<Geometry::TopologicalStruct*> verticesVec;
			std::unordered_set<Geometry::TopologicalStruct*> verticesSet;
			simplex.second->getAllNthChildren(verticesVec, verticesSet, 0);
			std::vector<GLuint> volumeIndices;
			std::unordered_set<GLuint> volumeIndicesSet;

			for (const auto& vertex : verticesVec)
			{
				auto vertexIndex = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(vertex)->halfSimplexData;

				if (volumeIndicesSet.insert(vertexIndex).second)
				{
					volumeIndices.push_back(vertexIndex);
				}
			}

			auto transform = LinearAlgebraUtils::getTransformFrom4Points(
				positions[volumeIndices[0]],
				positions[volumeIndices[1]],
				positions[volumeIndices[2]],
				positions[volumeIndices[3]]) *
				glm::translate(glm::mat4(1.0f), centroid) *
				glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)) *
				glm::translate(glm::mat4(1.0f), -centroid);

			simplexToBufferInstanceMap[simplex.second] = tetraTransforms.size();
			tetraTransforms.push_back(transform);
		}

		tetrahedralizationReady = true;
	});
	t.detach();
}

void TetrahedralizationContext::setupGeometries(void)
{
	makeQuad();
	dirty = true;
}

void TetrahedralizationContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	GraphicsSceneContext::setupPasses(gProgramSignatures, lProgramSignatures);

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["SURFACE"], "A");
	gP->addRenderableObjects(geometries["POINTS"], "C");

	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "B");
}

void TetrahedralizationContext::updateGeometries()
{
	auto meshObject = new Graphics::Tetrahedron();

	auto pickable = new Graphics::ReferencedGraphicsObject<GLuint, GLuint>(refMan, meshObject, tetraTransforms.size(), "INSTANCEID", 1);

	std::vector<GLbyte> selectedC;
	std::vector<glm::vec4> tetraColor;
	for (int i = 0; i < tetraTransforms.size(); ++i)
	{
		selectedC.push_back(1);
		tetraColor.push_back(glm::vec4(0, 0, 1, 1));
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto volumeTransforms = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(selectable, tetraTransforms, "TETRAINSTANCES", 1);

	geometries["VOLUMES"] = new Graphics::InstancedMeshObject<glm::vec4, float>(volumeTransforms, tetraColor, "TETRACOLORS", 1);

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["VOLUMES"], "A2");
	dirty = true;
}

void TetrahedralizationContext::update(void)
{
	if (glm::length(cameras[0]->velocity) > 0)
	{
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}

	if (tetrahedralizationReady)
	{
		updateGeometries();
		tetrahedralizationReady = false;
		dirty = true;
	}

	GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>::update();
}