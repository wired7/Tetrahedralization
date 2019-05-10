#pragma once
#include "GeometryRenderingContext.h"
#include "PointSamplingController.h"

namespace Graphics
{
	class ReferenceManager;
	class DecoratedGraphicsObject;
}

class PointSamplingController;

class PointSamplingContext : public GeometryRenderingContext<PointSamplingController, FPSCamera, PointSamplingContext>
{
protected:
	bool pointsReady = false;
	void setupCameras(void) override {};
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& programSignatures = {}, const std::vector<std::string>& lProgramSignatures = {}) override;
	std::vector<std::pair<std::string, std::string>> getVolumePairs() override;
	std::vector<std::pair<std::string, std::string>> getVertexPairs() override;
public:
	Graphics::ReferenceManager* refMan;
	bool readyToAdvance = false;
	std::vector<glm::vec3> points;
	static std::vector<glm::vec3> sampleSurface(int sampleSize, Graphics::DecoratedGraphicsObject* object);
	PointSamplingContext(Graphics::DecoratedGraphicsObject* surface, FPSCamera* cam, Graphics::ReferenceManager* refMan);
	~PointSamplingContext() {};
	void update(void) override;
};