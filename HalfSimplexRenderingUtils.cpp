#pragma once
#include "HalfSimplexRenderingUtils.h"
#include "LinearAlgebraUtils.h"
#include "ReferencedGraphicsObject.h"
#include "HalfSimplices.h"
#include <unordered_map>
#include <iostream>

namespace std
{
	template <> struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3& vector) const
		{
			return hash<float>()(vector.x * vector.y * vector.z + vector.y * vector.z + vector.z);
		}
	};
};

Graphics::DecoratedGraphicsObject* HalfSimplexRenderingUtils::getRenderableVolumesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																							  const std::vector<glm::vec3>& positions,
																							  const std::vector<glm::mat4>& transforms,
																							  Graphics::ReferenceManager* refMan)
{
	std::vector<GLuint> indices;
	std::vector<Graphics::Vertex> vertices;
	
	unsigned int i = 0;
	for (const auto& pos : positions)
	{
		auto vertexVec = manifold->fullSimplexMap0.find({ i })->second->halfSimplices;
		std::unordered_set<Geometry::FullSimplex<1, GLuint>*> fullEdgesSet;
		glm::vec3 normal;
		
		for (const auto& currentHalfVertex : vertexVec)
		{
			fullEdgesSet.insert(currentHalfVertex->belongsTo->fullSimplex);
		}
		
		for (const auto& fullEdge : fullEdgesSet)
		{
			for (const auto& halfEdge : fullEdge->halfSimplices)
			{
				if (halfEdge->pointsTo->halfSimplexData != i)
				{
					auto fV1 = halfEdge->pointsTo;
					auto fV2 = halfEdge->next->pointsTo;
					auto fV3 = halfEdge->next->next->pointsTo;
					normal += glm::cross(positions[fV2->halfSimplexData] - positions[fV1->halfSimplexData],
						positions[fV3->halfSimplexData] - positions[fV2->halfSimplexData]);

					break;
				}
			}
		}

		normal = normalize(normal);

		vertices.push_back(Graphics::Vertex(pos, normal));
		++i;
	}

	auto facets = manifold->map2;

	for (const auto& facet : facets)
	{
		std::unordered_set<Geometry::TopologicalStruct*> facetVerticesSet;
		std::vector<Geometry::TopologicalStruct*> facetVerticesVec;
		facet.second->getAllNthChildren(facetVerticesVec, facetVerticesSet, 0);

		for (const auto& facetVertex : facetVerticesVec)
		{
			auto fV = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(facetVertex);
			indices.push_back(fV->halfSimplexData);
		}
			
		// TODO: triangulate facetVertices and iterate through all the new facets inside this context
	}

	auto meshObject = new Graphics::MeshObject(vertices, indices);

	auto pickable = new Graphics::ReferencedGraphicsObject<GLuint, GLuint>(refMan, meshObject, indices.size() / 3, "INSTANCEID", 3);

	std::vector<GLbyte> selectedC;

	for (int i = 0; i < indices.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::ExtendedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION");

	auto g = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(selectable, transforms, "TRANSFORM");

	return g;
}

Graphics::DecoratedGraphicsObject* HalfSimplexRenderingUtils::getRenderableFacetsFromManifold(Geometry::Manifold2<GLuint>* manifold,
																							  const std::vector<glm::vec3>& positions,
																							  const std::vector<glm::mat4>& transforms,
																							  Graphics::ReferenceManager* refMan)
{
	std::vector<glm::vec3> centroids;
	std::vector<glm::mat4> triangleTransforms;

	auto facets = manifold->map2;
	auto verts = manifold->map0;

	glm::vec3 volumeCentroid;

	for (const auto& vertex : verts)
	{
		volumeCentroid += positions[vertex.second->halfSimplexData];
	}

	volumeCentroid /= (float)verts.size();

	for (const auto& transform : transforms)
	{
		glm::mat4 volumeTransform = transform * translate(glm::mat4(1.0f), volumeCentroid) * scale(glm::mat4(1.0f), glm::vec3(1.01f)) *
			translate(glm::mat4(1.0f), -volumeCentroid);

		for (const auto& facet : facets)
		{
			glm::vec3 facetCentroid;
			std::vector<glm::vec3> vertices;
			Geometry::HalfSimplex<1, GLuint>* currentEdge = facet.second->pointsTo;
			Geometry::HalfSimplex<1, GLuint>* firstEdge = currentEdge;
			do
			{
				glm::vec3 pos = glm::vec3(transform * glm::vec4(positions[currentEdge->pointsTo->halfSimplexData], 1));

				vertices.push_back(pos);
				facetCentroid += pos;
				currentEdge = currentEdge->next;
			} while (currentEdge != firstEdge);

			// TODO: Tesselate if more than 3 points in facet

			triangleTransforms.push_back(translate(glm::mat4(1.0f), facetCentroid) *
										 scale(glm::mat4(1.0f), glm::vec3(0.95f)) *										
										 translate(glm::mat4(1.0f), -facetCentroid) * 
										 LinearAlgebraUtils::getTransformFrom3Points(vertices[0], vertices[1], vertices[2]));
		}
	}

	auto meshObject = new Graphics::Triangle();

	auto instancedTriangles = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(meshObject, triangleTransforms, "TRIANGLEINSTANCES", 1);

	auto pickable = new Graphics::ReferencedGraphicsObject<GLuint, GLuint>(refMan, instancedTriangles, triangleTransforms.size(), "INSTANCEID", 1);

	std::vector<GLbyte> selectedC;

	for (int i = 0; i < triangleTransforms.size(); ++i)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	std::vector<glm::mat4> parentTransforms;

	parentTransforms.push_back(scale(glm::mat4(1.0f), glm::vec3(0.91f)));

	auto g = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(selectable, parentTransforms, "TRANSFORM", triangleTransforms.size());

	return g;
}

Graphics::DecoratedGraphicsObject* HalfSimplexRenderingUtils::getRenderableEdgesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																							 const std::vector<glm::vec3>& positions,
																							 const std::vector<glm::mat4>& transforms,
																							 Graphics::ReferenceManager* refMan)
{
	std::vector<glm::mat4> transformC;
	std::vector<glm::vec3> centroids;
	std::vector<GLbyte> warningC;

	auto facets = manifold->map2;
	auto verts = manifold->map0;

	glm::vec3 volumeCentroid;

	for (const auto& vertex : verts)
	{
		volumeCentroid += positions[vertex.second->halfSimplexData];
	}

	volumeCentroid /= (float)verts.size();

	for (const auto& transform : transforms)
	{
/*		glm::mat4 volumeTransform = transform * translate(glm::mat4(1.0f), volumeCentroid) * scale(glm::mat4(1.0f), glm::vec3(1.01f)) *
			translate(glm::mat4(1.0f), -volumeCentroid);*/

		for (const auto& facet : facets)
		{
			std::unordered_set<Geometry::TopologicalStruct*> facetVerticesSet;
			std::vector<Geometry::TopologicalStruct*> facetVerticesVec;
			facet.second->getAllNthChildren(facetVerticesVec, facetVerticesSet, 0);

			glm::vec3 facetCentroid;
			for (const auto& facetVertex : facetVerticesVec)
			{
				auto fV = reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(facetVertex);
				facetCentroid += glm::vec3(transform * glm::vec4(positions[fV->halfSimplexData], 1));
			}

			facetCentroid /= facetVerticesVec.size();

			std::unordered_set<Geometry::TopologicalStruct*> facetEdgesSet;
			std::vector<Geometry::TopologicalStruct*> facetEdgesVec;
			facet.second->getAllNthChildren(facetEdgesVec, facetEdgesSet, 1);
			for (const auto& edge : facetEdgesSet)
			{
				warningC.push_back(0);
				transformC.push_back(getHalfEdgeTransform(reinterpret_cast<Geometry::HalfSimplex<1, GLuint>*>(edge),
														  positions, transform, facetCentroid));
			}
		}
	}

	auto arrow = new Graphics::Arrow();

	auto g = new Graphics::MatrixInstancedMeshObject<glm::mat4, float>(arrow, transformC, "EDGETRANSFORM");

	auto pickable = new Graphics::ReferencedGraphicsObject<GLuint, GLuint>(refMan, g, transformC.size(), "INSTANCEID", 1);

	std::vector<GLbyte> selectedC;

	for (int i = 0; i < transformC.size(); i++)
	{
		selectedC.push_back(1);
	}

	auto selectable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(pickable, selectedC, "SELECTION", 1);

	auto highlightable = new Graphics::InstancedMeshObject<GLbyte, GLbyte>(selectable, warningC, "WARNING", 1);

	return highlightable;
}

glm::mat4 HalfSimplexRenderingUtils::getHalfEdgeTransform(Geometry::HalfSimplex<1, GLuint>* halfEdge,
													 const std::vector<glm::vec3>& positions,
													 const glm::mat4& parentTransform,
													 const glm::vec3 centroid)
{
	glm::vec3 point[2];
	point[0] = glm::vec3(parentTransform * glm::vec4(positions[halfEdge->pointsTo->halfSimplexData], 1));
	point[1] = glm::vec3(parentTransform * glm::vec4(positions[halfEdge->previous->pointsTo->halfSimplexData], 1));
	float edgeLength = length(point[1] - point[0]);
	auto lineTransform = LinearAlgebraUtils::getTransformFrom2Points(point[0], point[1]);

	glm::mat4 aroundCentroid =
		translate(glm::mat4(1.0f), -centroid) *
		lineTransform *
		scale(glm::mat4(1.0f), glm::vec3(1, 0.002f, 0.002f)) *
		translate(glm::mat4(1.0f), glm::vec3(0.5f, 0, 0)) *
		scale(glm::mat4(1.0f), glm::vec3(0.9f)) *
		translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0, 0));

	glm::mat4 transform =
		scale(glm::mat4(1.0f), glm::vec3(1.001f)) *
		translate(glm::mat4(1.0f), centroid) *
		scale(glm::mat4(1.0f), glm::vec3(0.7f)) *
		aroundCentroid;

	return transform;
}