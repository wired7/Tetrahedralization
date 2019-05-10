#pragma once
#include "GeometryRenderingContext.h"
#include "SurfaceViewController.h"

namespace Graphics
{
	class ReferenceManager;
}

namespace Geometry
{
	template <typename T> class Manifold2;
}

class SurfaceViewController;

class SurfaceViewContext : virtual public GeometryRenderingContext<SurfaceViewController, FPSCamera, SurfaceViewContext>
{
private:
	void setupRenderableHalfEdges(Geometry::Manifold2<GLuint>* manifold, const std::vector<glm::vec3>& positions, const std::vector<glm::mat4>& transform);
	void setupRenderableVertices(const std::vector<glm::vec3>& positions, const std::vector<glm::mat4>& transform);
	void setupRenderableFacets(Geometry::Manifold2<GLuint>* manifold, const std::vector<glm::vec3>& positions, const std::vector<glm::mat4>& transform);
protected:
	void setupCameras(void) override;
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& programSignatures = {}, const std::vector<std::string>& lProgramSignatures = {}) override;
	std::vector<std::pair<std::string, std::string>> getVolumePairs() override;
	std::vector<std::pair<std::string, std::string>> getSurfacePairs() override;
	std::vector<std::pair<std::string, std::string>> getEdgePairs() override;
	std::vector<std::pair<std::string, std::string>> getVertexPairs() override;
public:
	Graphics::ReferenceManager* refMan;
	SurfaceViewContext();
	~SurfaceViewContext() {};
};