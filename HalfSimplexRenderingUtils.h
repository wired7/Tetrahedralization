#pragma once
#include "GeometricalMeshObjects.h"
#include <map>

namespace Graphics
{
	class ReferenceManager;
}

namespace Geometry
{
	template <typename T> class Manifold2;
	template <typename T> class Manifold3;
	template <int T, typename S> class HalfSimplex;
}

namespace HalfSimplexRenderingUtils
{
	Graphics::DecoratedGraphicsObject* getRenderableVolumesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																		const std::vector<glm::vec3>& positions,
																		const std::vector<glm::mat4>& transforms,
																		Graphics::ReferenceManager* refMan);

	void getRenderableTetraTransforms(const std::vector<Geometry::HalfSimplex<3, GLuint>*>& tetras,
									  std::vector<glm::mat4>& tetraTransforms,
									  const std::vector<glm::vec3>& positions,
									  const std::vector<glm::mat4>& transforms,
									  std::map<Geometry::HalfSimplex<3, GLuint>*, int>& simplexToBufferInstanceMap);

	Graphics::DecoratedGraphicsObject* getRenderableTetrasFromTransforms(const std::vector<glm::mat4>& tetraTransforms,
																		 Graphics::ReferenceManager* refMan);

	void getRenderableFacetTransforms(const std::vector<Geometry::HalfSimplex<3, GLuint>*>& tetras,
									  std::vector<glm::mat4>& triangleTransforms,
									  const std::vector<glm::vec3>& positions,
									  const std::vector<glm::mat4>& transforms);

	Graphics::DecoratedGraphicsObject* getRenderableFacetsFromManifold(Geometry::Manifold2<GLuint>* manifold,
																		const std::vector<glm::vec3>& positions,
																		const std::vector<glm::mat4>& transforms,
																		Graphics::ReferenceManager* refMan);

	Graphics::DecoratedGraphicsObject* getRenderableFacetsFromTransforms(const std::vector<glm::mat4>& triangleTransforms,
																		 Graphics::ReferenceManager* refMan);

	Graphics::DecoratedGraphicsObject* getRenderableFacetsFromManifold(Geometry::Manifold3<GLuint>* manifold,
																		const std::vector<glm::vec3>& positions,
																		const std::vector<glm::mat4>& transforms,
																		Graphics::ReferenceManager* refMan);

	Graphics::DecoratedGraphicsObject* getRenderableEdgesFromManifold(Geometry::Manifold2<GLuint>* manifold,
																		const std::vector<glm::vec3>& positions,
																		const std::vector<glm::mat4>& transforms,
																		Graphics::ReferenceManager* refMan);

	glm::mat4 getHalfEdgeTransform(Geometry::HalfSimplex<1, GLuint>* halfEdge,
									 const std::vector<glm::vec3>& positions,
									 const glm::mat4& parentTransform,
									 const glm::vec3 centroid);
};

