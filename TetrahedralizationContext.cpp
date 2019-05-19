#pragma once
#include "TetrahedralizationContext.h"
#include "GeometricalMeshObjects.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplexGeometryUtils.h"
#include "HalfSimplexRenderingUtils.h"
#include "VoronoiDiagramUtils.h"
#include "LinearAlgebraUtils.h"
#include "DiscreteGeometryUtils.h"
#include <thread>
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
													 Graphics::ReferenceManager* refMan) :
	GeometryRenderingContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>(), 
	positions(_points), refMan(refMan)
{
	cameras.push_back(cam);
	geometries["SURFACE"] = surface;
	geometries["POINTS"] = points;
	setupGeometries();
	setupPasses({ "A", "A2", "EdgeA", "C", "D" }, { "B" });

	std::thread t([&]
	{
		surfaceTriangles = DiscreteGeometryUtils::getTrianglesFromMesh(surface);

		positions.push_back(glm::vec3(-100000));
		positions.push_back(glm::vec3(0, 100000, 0));
		positions.push_back(glm::vec3(1, -1, -1) * 100000.0f);
		positions.push_back(glm::vec3(0, 0, 100000));

		std::vector<GLuint> backgroundSimplexVertices =
		{
			static_cast<GLuint>(positions.size() - 4),
			static_cast<GLuint>(positions.size() - 3),
			static_cast<GLuint>(positions.size() - 2),
			static_cast<GLuint>(positions.size() - 1)
		};

		manifold = VoronoiDiagramUtils::delaunayTriangulation(positions, backgroundSimplexVertices);
//		DiscreteGeometryUtils::getConvexHullFromManifold(manifold, backgroundSimplexVertices);

		glm::vec3 trianglePts[3] = { surfaceTriangles[0].point1, surfaceTriangles[0].point2, surfaceTriangles[0].point3 };
		Parametric::Triangle triangle(trianglePts[0], trianglePts[1], trianglePts[2]);
		glm::vec3 centroid;
		for (int i = 0; i < 3; ++i)
		{
			centroid += trianglePts[i];
		}
		centroid /= 3.0f;

		auto facetTetraUnion = [](Geometry::HalfSimplex<3, GLuint>* currentSimplex,
								   std::vector<glm::vec3>& positions, const Parametric::Triangle& triangle)->bool
		{
			std::vector<glm::vec3> tetraPoints;
			auto tetraIndices = currentSimplex->fullSimplex->data;

			for (int i = 0; i < 4; ++i)
			{
				tetraPoints.push_back(positions[tetraIndices[i]]);
			}

			for (int i = 0; i < 4; ++i)
			{
				std::vector<glm::vec3> tetraFacet;

				for (int j = 0; j < 3; ++j)
				{
					tetraFacet.push_back(tetraPoints[(i + j) % 4]);
				}

				if (triangle.intersects(Parametric::Triangle(tetraFacet[0], tetraFacet[1], tetraFacet[2])))
				{
					return true;
				}
			}

			return false;
		};

		auto intersectingSimplex = HalfSimplexGeometryUtils::findSimplex<3, glm::vec3>(manifold->map3.begin()->second, positions,
			centroid, [&](Geometry::HalfSimplex<3, GLuint>* currentSimplex,
				std::vector<glm::vec3>& positions, glm::vec3 targetPoint)->bool
		{
			return facetTetraUnion(currentSimplex, positions, triangle);
		});

		std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> visitedSimplices;
		std::vector<Geometry::HalfSimplex<3, GLuint>*> simplicesToErase;
		std::queue<Geometry::HalfSimplex<3, GLuint>*> simplexQueue;
		simplexQueue.push(intersectingSimplex);

		while (!simplexQueue.empty())
		{
			auto currentSimplex = simplexQueue.front();
			simplexQueue.pop();

			if (visitedSimplices.find(currentSimplex) != visitedSimplices.end())
			{
				continue;
			}

			visitedSimplices.insert(currentSimplex);

			if (facetTetraUnion(currentSimplex, positions, triangle))
			{
				simplicesToErase.push_back(currentSimplex);

				std::vector<Geometry::TopologicalStruct*> vertexVec;
				std::unordered_set<Geometry::TopologicalStruct*> vertexSet;
				currentSimplex->getAllNthChildren(vertexVec, vertexSet, 0);

				std::unordered_set<Geometry::HalfSimplex<3, GLuint>*> uniqueTetras;
				for (const auto& vertex : vertexVec)
				{
					auto fullVertex = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(vertex)->fullSimplex;

					for (const auto& halfVertex : fullVertex->halfSimplices)
					{
						auto halfTetra = halfVertex->belongsTo->belongsTo->belongsTo;

						if (halfTetra != currentSimplex && uniqueTetras.insert(halfTetra).second)
						{
							simplexQueue.push(halfTetra);
						}
					}
				}
			}
		}

		std::map<std::vector<GLuint>, int> openFacets;
		for (const auto& simplex : simplicesToErase)
		{
			std::vector<Geometry::TopologicalStruct*> facetVec;
			std::unordered_set<Geometry::TopologicalStruct*> facetSet;
			simplex->getAllChildren(facetVec, facetSet);

			for (const auto& facet : facetVec)
			{
				auto data = reinterpret_cast<Geometry::HalfSimplex<2, GLuint>*>(facet)->fullSimplex->data;

				openFacets[data]++;

				if (openFacets[data] == 2)
				{
					openFacets.erase(data);
				}
			}

			manifold->erase3(simplex);
		}

		std::cout << openFacets.size() << std::endl;

//		DiscreteGeometryUtils::constrainTriangulation(manifold, positions, surfaceTriangles);

		std::vector<Geometry::HalfSimplex<3, GLuint>*> tetraVec;
		tetraVec.resize(manifold->map3.size());
		int count = 0;
		for (const auto& tetra : manifold->map3)
		{
			tetraVec[count++] = tetra.second;
		}
		
		std::thread setupTetraTransforms([&] {
			HalfSimplexRenderingUtils::getRenderableTetraTransforms(tetraVec, tetraTransforms, positions,
																	{ glm::mat4(1.0f) }, simplexToBufferInstanceMap);
		});
		std::thread setupTriangleTransforms([&] {
			HalfSimplexRenderingUtils::getRenderableFacetTransforms(tetraVec, triangleTransforms, positions, { glm::mat4(1.0f) });
		});
		setupTetraTransforms.join();
		setupTriangleTransforms.join();

		tetrahedralizationReady = true;
		readyToAdvance = true;
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
	gP->addRenderableObjects(geometries["SURFACE"], "SURFACE", "A");
	gP->addRenderableObjects(geometries["POINTS"], "POINTS", "C");

	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "DISPLAYQUAD", "B");
}

void TetrahedralizationContext::updateGeometries()
{
	auto facetCentroid = (surfaceTriangles[0].point1 + surfaceTriangles[0].point2 + surfaceTriangles[0].point3) / 3.0f;
	auto tTransform = glm::translate(glm::mat4(1.0f), facetCentroid) *
					  scale(glm::mat4(1.0f), glm::vec3(0.95f)) *
					  glm::translate(glm::mat4(1.0f), -facetCentroid) *
					  glm::scale(glm::mat4(1.0f), glm::vec3(1.0f / 0.91f)) *
					  LinearAlgebraUtils::getTransformFrom3Points(surfaceTriangles[0].point1,
																  surfaceTriangles[0].point2,
																  surfaceTriangles[0].point3);
	geometries["TRIANGLE"] = HalfSimplexRenderingUtils::getRenderableFacetsFromTransforms({ tTransform }, refMan);
	geometries["VOLUMES"] = HalfSimplexRenderingUtils::getRenderableTetrasFromTransforms(tetraTransforms, refMan);
	geometries["FACETS"] = HalfSimplexRenderingUtils::getRenderableFacetsFromTransforms(triangleTransforms, refMan);

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["VOLUMES"], "VOLUMES", "A2");
	gP->addRenderableObjects(geometries["FACETS"], "FACETS", "D");
	gP->addRenderableObjects(geometries["TRIANGLE"], "TRIANGLE", "D");
	dirty = true;
}

void TetrahedralizationContext::update(void)
{
	if (tetrahedralizationReady)
	{
		updateGeometries();
		tetrahedralizationReady = false;
	}

	GraphicsSceneContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>::update();
}

std::vector<std::pair<std::string, std::string>> TetrahedralizationContext::getVolumePairs()
{
	return { std::make_pair("VOLUMES", "A2") };
}

std::vector<std::pair<std::string, std::string>> TetrahedralizationContext::getSurfacePairs()
{
	return { std::make_pair("FACETS", "D") };
}

std::vector<std::pair<std::string, std::string>> TetrahedralizationContext::getVertexPairs()
{
	return { std::make_pair("POINTS", "C") };
}