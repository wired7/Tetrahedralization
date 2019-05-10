#pragma once
#include "GeometryRenderingContext.h"
#include "TetrahedralizationController.h"

namespace Graphics
{
	class ReferenceManager;
}

namespace Geometry
{
	template <typename T> class Manifold3;
	template <const int T, typename S> class HalfSimplex;
}

class TetrahedralizationController;

class TetrahedralizationContext : public GeometryRenderingContext<TetrahedralizationController, FPSCamera, TetrahedralizationContext>
{
private:
	std::vector<glm::mat4> tetraTransforms;
	std::vector<glm::mat4> triangleTransforms;
protected:
	void setupCameras(void) override {};
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures) override;
	std::vector<std::pair<std::string, std::string>> getVolumePairs() override;
	std::vector<std::pair<std::string, std::string>> getSurfacePairs() override;
	std::vector<std::pair<std::string, std::string>> getVertexPairs() override;
public:
	std::vector<glm::vec3>& positions;
	bool tetrahedralizationReady = false;
	bool readyToAdvance = false;
	std::map<Geometry::HalfSimplex<3, GLuint>*, int> simplexToBufferInstanceMap;
	Graphics::ReferenceManager* refMan;
	Geometry::Manifold3<GLuint>* manifold;
	TetrahedralizationContext(Graphics::DecoratedGraphicsObject* surface,
							  Graphics::DecoratedGraphicsObject* points,
							  std::vector<glm::vec3>& _points,
							  FPSCamera* cam,
							  Graphics::ReferenceManager* refMan);
	~TetrahedralizationContext() {};
	void update(void) override;
	void updateGeometries();
};
