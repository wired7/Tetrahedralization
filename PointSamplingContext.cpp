#pragma once
#include "PointSamplingContext.h"
#include "GeometricalMeshObjects.h"
#include "ReferencedGraphicsObject.h"
#include "DiscreteGeometryUtils.h"
#include "FPSCameraControls.h"
#include <thread>
#include <random>
#include <glew.h>
#include "gtc/matrix_transform.hpp"

#define NUM_POINTS 10000
#define POINT_SCALE 0.05f

PointSamplingContext::PointSamplingContext(Graphics::DecoratedGraphicsObject* surface, FPSCamera* cam, Graphics::ReferenceManager* refMan) : refMan(refMan)
{
	cameras.push_back(cam);
	geometries["SURFACE"] = surface;
	makeQuad();
	setupPasses({ "A", "C" }, {"B"});

	std::thread t([&]
	{
		points = sampleSurface(NUM_POINTS, surface);
		pointsReady = true;
	});
	t.detach();
}

void PointSamplingContext::setupGeometries(void)
{
	((GeometryPass*)passRootNode)->clearRenderableObjects("C");

	auto m = new Graphics::Polyhedron(10, glm::vec3(), glm::vec3(1.0f));

	std::vector<glm::mat4> transform;

	for (int i = 0; i < points.size(); i++)
	{
		transform.push_back(glm::translate(glm::mat4(1.0f), points[i]) * glm::scale(glm::mat4(1.0f), glm::vec3(1,1,1) * POINT_SCALE));
	}

	auto g = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(m, transform, "OFFSET");

	auto pickable = new Graphics::ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, points.size(), "INSTANCEID", 1);

	std::vector<GLbyte> selected;

	for (int i = 0; i < transform.size(); i++)
	{
		selected.push_back(1);
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selected, "SELECTION", 1);

	geometries["VERTICES"] = selectable;

	((GeometryPass*)passRootNode)->addRenderableObjects(selectable, "C");
	dirty = true;
}

void PointSamplingContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	GraphicsSceneContext::setupPasses(gProgramSignatures, lProgramSignatures);

	((GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS"))->addRenderableObjects(geometries["SURFACE"], "A");
	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "B");
}

void PointSamplingContext::update(void)
{
	if (glm::length(cameras[0]->velocity) > 0)
	{
		FPSCameraControls::moveCamera(cameras[0], cameras[0]->velocity);
		dirty = true;
	}

	if (pointsReady)
	{
		setupGeometries();
		pointsReady = false;
		readyToAdvance = true;
		dirty = true;
	}

	GraphicsSceneContext<PointSamplingController, FPSCamera, PointSamplingContext>::update();
}

std::vector<glm::vec3> PointSamplingContext::sampleSurface(int sampleSize, Graphics::DecoratedGraphicsObject* object)
{
	auto boundingBox = DiscreteGeometryUtils::getBoundingBox(object);
	glm::vec3 boundSize = boundingBox.second - boundingBox.first;
	glm::vec3 center = (boundingBox.second + boundingBox.first) / 2.0f;

	auto triangles = DiscreteGeometryUtils::getTrianglesFromMesh(object);
	std::vector<glm::vec3> samples;
	static thread_local std::mt19937 generator;
	std::uniform_real_distribution<double> distribution(-0.5, 0.5);
	while (samples.size() < NUM_POINTS)
	{
		sampleSize = NUM_POINTS - samples.size();
#pragma omp parallel for schedule(dynamic, 10)
		for (int i = 0; i < sampleSize; ++i)
		{
			glm::vec3 point(distribution(generator), distribution(generator), distribution(generator));
			point *= boundSize;
			point += center;

			if (DiscreteGeometryUtils::isPointInsideMesh(point, triangles))
			{
#pragma omp critical
				{
					samples.push_back(point);
				}
			}
		}
	}

	auto meshObject = reinterpret_cast<Graphics::MeshObject*>(object->signatureLookup("VERTEX"));
	auto vertices = meshObject->vertices;
	auto transform = reinterpret_cast<Graphics::MatrixInstancedMeshObject<glm::mat4, float>*>(object->signatureLookup("TRANSFORM"))->extendedData[0];

	for (const auto& vertex : vertices)
	{
		samples.push_back(glm::vec3(transform * glm::vec4(vertex.position, 1)));
	}

	return samples;
}