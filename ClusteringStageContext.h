#pragma once
#include "Context.h"
#include "ClusteringStageController.h"

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

class ClusteringStageController;
class OpenCLContext;
class CLKernel;
template<typename T> class CLBuffer;
template<typename T> class CLGLBuffer;

class ClusteringStageContext : public GraphicsSceneContext<ClusteringStageController, FPSCamera, ClusteringStageContext>
{
private:
	bool firstTime = true;
protected:
	Graphics::DecoratedGraphicsObject* mesh = nullptr;
	std::vector<glm::vec3>& positions;
	std::map<Geometry::HalfSimplex<3, GLuint>*, int>& simplexToBufferInstanceMap;
	OpenCLContext* clContext = nullptr;
	CLKernel* volumeUpdateKernel = nullptr;
	CLBuffer<float>* cameraPosBuffer = nullptr;
	CLBuffer<float>* cameraDirBuffer = nullptr;
	CLGLBuffer<float>* renderableBuffer = nullptr;
	CLGLBuffer<float>* positionsBuffer = nullptr;
	CLGLBuffer<glm::mat4>* transformsBuffer = nullptr;
	glm::vec4 color1 = glm::vec4(0, 0, 1, 1);
	glm::vec4 color2 = glm::vec4(1, 1, 0, 1);
	void setupCameras(void) override {};
	void setupGeometries(void) override;
	void setupPasses(const std::vector<std::string>& programSignatures = {}, const std::vector<std::string>& lProgramSignatures = {}) override;
	void update(void) override;

public:
	bool updateColorBuffer = false;
	bool busy = false;
	Graphics::ReferenceManager* refMan = nullptr;
	std::map<int, Geometry::HalfSimplex<3, GLuint>*> bufferInstanceToSimplexMap;
	CLBuffer<float>* separationDistanceBuffer = nullptr;
	bool updateDistanceBuffer;
	ClusteringStageContext(Graphics::DecoratedGraphicsObject* volume, Geometry::Manifold3<GLuint>* manifold,
						   std::vector<glm::vec3>& positions, std::map<Geometry::HalfSimplex<3, GLuint>*, int>& simplexToBufferInstanceMap,
						   Graphics::ReferenceManager* refMan, FPSCamera* cam);
	~ClusteringStageContext();
	void calculateGeodesicFromTetra(Geometry::HalfSimplex<3, GLuint>* startingTetra);
};