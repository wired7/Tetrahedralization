#pragma once
#include <vector>
#include <glew.h>
#include <glm.hpp>

namespace Parametric
{
	class Sphere;
};

namespace Geometry
{
	template <typename T> class Manifold3;
};

namespace VoronoiDiagramUtils
{
	Parametric::Sphere getCircumsphere(glm::vec3 points[4]);
	bool isSpaceDegenerate(std::vector<glm::vec3> points);
	Geometry::Manifold3<GLuint>* delaunayTriangulation(std::vector<glm::vec3>& positions,
													   const std::vector<GLuint>& backgroundSimplexVertices);
/*	Geometry::Vertex* getVoronoiPointFromTetrahedron(Geometry::Mesh* mesh, const std::vector<glm::vec3>& inputPositions, std::vector<glm::vec3>& outputPositions);
	std::pair<Geometry::Vertex*, Geometry::Vertex*> getVoronoiEdgeFromTetrahedraPair(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2, const std::map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	Geometry::Facet* getVoronoiFacetFromEdge(std::pair<Geometry::Vertex*, Geometry::Vertex*> edgeVertices, const std::map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	std::vector<std::pair<Geometry::Vertex*, Geometry::Vertex*>> getEdgesIncidentToVertex(Geometry::Vertex* vertex, Geometry::VolumetricMesh* volumetricMesh);

	static Geometry::VolumetricMesh* getVoronoiDiagram(Geometry::VolumetricMesh* volumetricMesh, const vector<vec3>& positions);*/
};