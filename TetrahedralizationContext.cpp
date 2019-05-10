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
		std::vector<Parametric::Triangle> surfaceTriangles = DiscreteGeometryUtils::getTrianglesFromMesh(surface);

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
		DiscreteGeometryUtils::getConvexHullFromManifold(manifold, backgroundSimplexVertices);
		DiscreteGeometryUtils::constrainTriangulation(manifold, positions, surfaceTriangles);

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

	geometries["VOLUMES"] = HalfSimplexRenderingUtils::getRenderableTetrasFromTransforms(tetraTransforms, refMan);
	geometries["FACETS"] = HalfSimplexRenderingUtils::getRenderableFacetsFromTransforms(triangleTransforms, refMan);

	auto gP = (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
	gP->addRenderableObjects(geometries["VOLUMES"], "VOLUMES", "A2");
	gP->addRenderableObjects(geometries["FACETS"], "FACETS", "D");
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