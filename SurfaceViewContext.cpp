#pragma once
#include "SurfaceViewContext.h"
#include "HalfSimplexRenderingUtils.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"
#include <glew.h>
#include "gtc/matrix_transform.hpp"

SurfaceViewContext::SurfaceViewContext() : GeometryRenderingContext<SurfaceViewController, FPSCamera, SurfaceViewContext>()
{
	setupCameras();
	setupGeometries();
	setupPasses({ "A", "EdgeA", "C", "D" }, {"B"});
}

void SurfaceViewContext::setupCameras(void)
{
	auto widthHeight = WindowContext::context->getSize();
	int width = widthHeight.first;
	int height = widthHeight.second;

	cameras.push_back(new FPSCamera(glm::vec2(0, 0), glm::vec2(1, 1), glm::vec3(0, 0, 10), glm::vec3(0, 0, 0),
		glm::vec3(0, 1, 0), glm::perspective(45.0f, (float)width / height, 0.1f, 1000.0f)));
}

void SurfaceViewContext::setupGeometries(void)
{
	refMan = new Graphics::ReferenceManager();
	makeQuad();

	auto meshObject = new Graphics::ImportedMeshObject("models\\filledChinchilla.obj");

	std::vector<glm::mat4> transform;

	glm::vec3 pos(0, 0, 0);
	transform.push_back(glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0f), glm::vec3(10.0f)));

	Geometry::Manifold2<GLuint>* manifold = new Geometry::Manifold2<GLuint>(meshObject->indices);

	std::vector<glm::vec3> positions;
	for (int i = 0; i < meshObject->vertices.size(); i++)
	{
		positions.push_back(meshObject->vertices[i].position);
	}

	setupRenderableVertices(positions, transform);

	Graphics::DecoratedGraphicsObject* outputGeometry = HalfSimplexRenderingUtils::getRenderableVolumesFromManifold(manifold,
																												   positions,
																												   transform,
																												   refMan);

	geometries["VOLUME"] = outputGeometry;

	setupRenderableHalfEdges(manifold, positions, transform);
	setupRenderableFacets(manifold, positions, transform);
}

void SurfaceViewContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	GraphicsSceneContext::setupPasses(gProgramSignatures, lProgramSignatures);

	auto gP = ((GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS"));
	
	gP->addRenderableObjects(geometries["VOLUME"], "VOLUME", "A");
	gP->addRenderableObjects(geometries["VERTICES"], "VERTICES", "C");
	gP->addRenderableObjects(geometries["EDGES"], "EDGES", "EdgeA");
	gP->addRenderableObjects(geometries["FACETS"], "FACETS", "D");
	
	((LightPass*)passRootNode->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "DISPLAYQUAD", "B");
}

void SurfaceViewContext::setupRenderableHalfEdges(Geometry::Manifold2<GLuint>* manifold, const std::vector<glm::vec3>& positions,
												  const std::vector<glm::mat4>& transform)
{
	Graphics::DecoratedGraphicsObject* outputGeometry = HalfSimplexRenderingUtils::getRenderableEdgesFromManifold(manifold,
																												  positions,
																												  transform,
																												  refMan);

	geometries["EDGES"] = outputGeometry;
}

void SurfaceViewContext::setupRenderableVertices(const std::vector<glm::vec3>& positions, const std::vector<glm::mat4>& transform)
{
	std::vector<glm::mat4> pTransforms;

	for (int j = 0; j < transform.size(); j++)
	{
		for (int i = 0; i < positions.size(); i++)
		{
			pTransforms.push_back(transform[j] * glm::scale(glm::mat4(1.0f), glm::vec3(1.0f)) * glm::translate(glm::mat4(1.0f), positions[i]) *
				glm::scale(glm::mat4(1.0f), glm::vec3(0.002f)));
		}
	}

	auto point = new Graphics::Polyhedron(4, glm::vec3(), glm::vec3(1.0f));

	auto g = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(point, pTransforms, "TRANSFORM");

	auto pickable = new Graphics::ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, pTransforms.size(), "INSTANCEID", 1);

	std::vector<GLbyte> selectedC;

	for (int i = 0; i < pTransforms.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	geometries["VERTICES"] = selectable;
}

void SurfaceViewContext::setupRenderableFacets(Geometry::Manifold2<GLuint>* manifold, const std::vector<glm::vec3>& positions,
											   const std::vector<glm::mat4>& transform)
{
	Graphics::DecoratedGraphicsObject* outputGeometry = HalfSimplexRenderingUtils::getRenderableFacetsFromManifold(manifold,
																												   positions,
																												   transform,
																												   refMan);

	geometries["FACETS"] = outputGeometry;
}

std::vector<std::pair<std::string, std::string>> SurfaceViewContext::getVolumePairs()
{
	return { std::make_pair("VOLUME", "A") };
}

std::vector<std::pair<std::string, std::string>> SurfaceViewContext::getSurfacePairs()
{
	return { std::make_pair("FACETS", "D") };
}

std::vector<std::pair<std::string, std::string>> SurfaceViewContext::getEdgePairs()
{
	return { std::make_pair("EDGES", "EdgeA") };
}

std::vector<std::pair<std::string, std::string>> SurfaceViewContext::getVertexPairs()
{
	return { std::make_pair("VERTICES", "C") };
}