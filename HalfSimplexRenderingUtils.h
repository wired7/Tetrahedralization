#pragma once
#include "GeometricalMeshObjects.h"

namespace Graphics
{
	class ReferenceManager;
}

namespace Geometry
{
	template <typename T> class Manifold2;
	template <int T, typename S> class HalfSimplex;
}

static class HalfSimplexRenderingUtils
{
public:
	static Graphics::DecoratedGraphicsObject* getRenderableVolumesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																			  const std::vector<glm::vec3>& positions,
																			  const std::vector<glm::mat4>& transforms,
																			  Graphics::ReferenceManager* refMan);

	static Graphics::DecoratedGraphicsObject* getRenderableFacetsFromManifold(Geometry::Manifold2<GLuint>* manifold,
																			  const std::vector<glm::vec3>& positions,
																			  const std::vector<glm::mat4>& transforms,
																			  Graphics::ReferenceManager* refMan);

	static Graphics::DecoratedGraphicsObject* getRenderableEdgesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																			 const std::vector<glm::vec3>& positions,
																			 const std::vector<glm::mat4>& transforms,
																			 Graphics::ReferenceManager* refMan);

	static glm::mat4 getHalfEdgeTransform(Geometry::HalfSimplex<1, GLuint>* halfEdge,
									 const std::vector<glm::vec3>& positions,
									 const glm::mat4& parentTransform,
									 const glm::vec3 centroid);
};

