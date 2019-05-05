#pragma once
#include <vector>
#include <glm.hpp>

namespace Parametric
{
	class Sphere;
}

namespace VoronoiDiagramUtils
{
	Parametric::Sphere getCircumsphere(glm::vec3 points[4]);
	bool isSpaceDegenerate(std::vector<glm::vec3> points);
	bool isPointWithinSphere(glm::vec3 point, glm::vec3 points[4]);
	std::vector<glm::ivec4> calculateDelaunayTetrahedra(const std::vector<glm::vec3>& points);
/*	Geometry::Vertex* getVoronoiPointFromTetrahedron(Geometry::Mesh* mesh, const std::vector<glm::vec3>& inputPositions, std::vector<glm::vec3>& outputPositions);
	std::pair<Geometry::Vertex*, Geometry::Vertex*> getVoronoiEdgeFromTetrahedraPair(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2, const std::map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	Geometry::Facet* getVoronoiFacetFromEdge(std::pair<Geometry::Vertex*, Geometry::Vertex*> edgeVertices, const std::map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	std::vector<std::pair<Geometry::Vertex*, Geometry::Vertex*>> getEdgesIncidentToVertex(Geometry::Vertex* vertex, Geometry::VolumetricMesh* volumetricMesh);

	static Geometry::VolumetricMesh* getVoronoiDiagram(Geometry::VolumetricMesh* volumetricMesh, const vector<vec3>& positions);*/
};