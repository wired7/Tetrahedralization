#pragma once
#include "ClusteringStageContext.h"
#include "HalfSimplices.h"
#include "HalfSimplexGeometryUtils.h"
#include "OpenCLContext.h"
#include "CLPass.h"
#include "ShaderProgramPipeline.h"
#include <thread>
#include <glew.h>

ClusteringStageContext::ClusteringStageContext(
	Graphics::DecoratedGraphicsObject* mesh, Geometry::Manifold3<GLuint>* manifold,
	std::vector<glm::vec3>& positions, std::map<Geometry::HalfSimplex<3, GLuint>*, int>& simplexToBufferInstanceMap,
	Graphics::ReferenceManager* refMan, FPSCamera* cam) :
	GeometryRenderingContext<ClusteringStageController, FPSCamera, ClusteringStageContext>(),
	mesh(mesh), refMan(refMan), simplexToBufferInstanceMap(simplexToBufferInstanceMap), positions(positions)
{
	cameras.push_back(cam);

	bufferInstanceToSimplexMap.resize(simplexToBufferInstanceMap.size());
	for (const auto& object : simplexToBufferInstanceMap)
	{
		bufferInstanceToSimplexMap[object.second] = object.first;
	}

	geometries["VOLUMES"] = mesh;
 	setupGeometries();

	//////////////////////////////////////////////////////////////////////////////////////////////////
	clContext = new OpenCLContext();
	volumeUpdateKernel = new CLKernel("volume_update_kernel.cl", "update_volume", clContext);

	std::vector<float> cameraPos = { 0, 0, 0 };
	std::vector<float> cameraDir = { 1, 0, 0 };
	std::vector<float> distance = { 3.0f };
	cameraPosBuffer = new CLBuffer<float>(clContext, cameraPos, CL_MEM_READ_WRITE);
	cameraDirBuffer = new CLBuffer<float>(clContext, cameraDir, CL_MEM_READ_WRITE);
	separationDistanceBuffer = new CLBuffer<float>(clContext, distance, CL_MEM_READ_WRITE);
	transformsBuffer = new CLGLBuffer<glm::mat4>(
		clContext,
		geometries["VOLUMES"]->signatureLookup("TETRAINSTANCES")->VBO,
		CL_MEM_READ_WRITE);
	renderableBuffer = new CLGLBuffer<float>(clContext, geometries["VOLUMES"]->VBO, CL_MEM_READ_WRITE);
	renderableBuffer->bufferData.resize(((Graphics::InstancedMeshObject<float, float>*)geometries["VOLUMES"]->signatureLookup("RENDERABLE"))->extendedData.size());

	auto vertices = (Graphics::MeshObject*)geometries["VOLUMES"]->signatureLookup("VERTEX");
	positionsBuffer = new CLGLBuffer<float>(clContext, vertices->VBO);
	positionsBuffer->bufferData.resize(vertices->vertices.size());

	cameraPosBuffer->bindBuffer();
	cameraDirBuffer->bindBuffer();
	separationDistanceBuffer->bindBuffer();

	setupPasses({"V1", "V2"}, { "SB" });
}

ClusteringStageContext::~ClusteringStageContext()
{
}

void ClusteringStageContext::setupGeometries(void)
{
	makeQuad();

	auto instanceIDs = ((Graphics::ExtendedMeshObject<GLuint, GLuint>*)geometries["VOLUMES"]->signatureLookup("INSTANCEID"));
	auto objectIDs = instanceIDs->extendedData;

	std::vector<float> renderable;
	for (int i = 0; i < objectIDs.size(); i++)
	{
		renderable.push_back(1.0f);
	}

	geometries["VOLUMES"] = new Graphics::InstancedMeshObject<float, float>(geometries["VOLUMES"], renderable, "RENDERABLE", 1);
	dirty = true;
}

void ClusteringStageContext::setupPasses(const std::vector<std::string>& gProgramSignatures, const std::vector<std::string>& lProgramSignatures)
{
	std::map<std::string, std::pair<CLKernel*, std::pair<int, int>>> clKMap;
	int modulo = renderableBuffer->bufferData.size() % 32;
	clKMap["VOLUMEUPDATE"] = std::make_pair(volumeUpdateKernel, std::make_pair(renderableBuffer->bufferData.size() + (32 - modulo), 2));
	CLPass* clP = new CLPass(clKMap);

	clP->addBuffer(renderableBuffer, "RENDERABLE", "VOLUMEUPDATE", 0, true);
	clP->addBuffer(positionsBuffer, "POSITIONS", "VOLUMEUPDATE", 1, true);
	clP->addBuffer(cameraPosBuffer, "CAMERAPOS", "VOLUMEUPDATE", 2);
	clP->addBuffer(cameraDirBuffer, "CAMERADIR", "VOLUMEUPDATE", 3);
	clP->addBuffer(separationDistanceBuffer, "SEPARATIONDISTANCE", "VOLUMEUPDATE", 4);
	clP->addBuffer(transformsBuffer, "TRANSFORMS", "VOLUMEUPDATE", 5, true);

	std::map<std::string, ShaderProgramPipeline*> gPrograms1;
	gPrograms1["V1"] = ShaderProgramPipeline::getPipeline("V1"); 

	GeometryPass* gP1 = new GeometryPass(gPrograms1, "GEOMETRYPASS0", nullptr, 1, 1, false);
	gP1->setupCamera(cameras[0]);

	clP->addNeighbor(gP1);

	std::map<std::string, ShaderProgramPipeline*> gPrograms2;
	gPrograms2["V2"] = ShaderProgramPipeline::getPipeline("V2");

	GeometryPass* gP2 = new GeometryPass(gPrograms2, "GEOMETRYPASS1");
	gP2->setupCamera(cameras[0]);

	clP->addNeighbor(gP2);

	std::map<std::string, ShaderProgramPipeline*> lPrograms;

	for (const auto& programSignature : lProgramSignatures)
	{
		lPrograms[programSignature] = ShaderProgramPipeline::getPipeline(programSignature);
	}

	LightPass* lP = new LightPass(lPrograms, true);
	lP->setTextureToIgnore("PICKING0", "GEOMETRYPASS0");
	gP1->addNeighbor(lP);
	gP2->addNeighbor(lP);

	passRootNode = clP;

	gP1->addRenderableObjects(geometries["VOLUMES"], "VOLUMES", "V1");
	gP2->addRenderableObjects(geometries["VOLUMES"], "VOLUMES", "V2");
	((LightPass*)((GeometryPass*)passRootNode)->signatureLookup("LIGHTPASS"))->addRenderableObjects(geometries["DISPLAYQUAD"], "DISPLAYQUAD", "SB");

	gP1->setupVec4f(color1, "inputColor");
	gP2->setupVec4f(color2, "inputColor");
//	auto windowSize = WindowContext::context->getSize();
//	gP1->setupVec2i(glm::ivec2(windowSize.first, windowSize.second), "WIN_SCALE");
}

void ClusteringStageContext::updateCameraCL()
{
	for (int i = 0; i < 3; i++)
	{
		cameraPosBuffer->bufferData[i] = cameras[0]->camPosVector[i];
		cameraDirBuffer->bufferData[i] = cameras[0]->lookAtVector[i];
	}

	cameraPosBuffer->updateBuffer();
	cameraDirBuffer->updateBuffer();

	dirty = true;
}

void ClusteringStageContext::update(void)
{
	if (firstTime || cameras[0]->dirty)
	{
		updateCameraCL();
		
		if (firstTime)
		{
			firstTime = false;
		}
	}

	if (updateDistanceBuffer)
	{
		separationDistanceBuffer->updateBuffer();
		updateDistanceBuffer = false;
		dirty = true;
	}

	if (updateColorBuffer)
	{
		auto colorBufferObj = reinterpret_cast<Graphics::InstancedMeshObject<glm::vec4, float>*>(mesh->signatureLookup("TETRACOLORS"));
		colorBufferObj->updateBuffers();
		updateColorBuffer = false;
		dirty = true;
	}

	GraphicsSceneContext<ClusteringStageController, FPSCamera, ClusteringStageContext>::update();
}

GeometryPass* ClusteringStageContext::getGeometryPass()
{
	return reinterpret_cast<GeometryPass*>(passRootNode->signatureLookup("GEOMETRYPASS0"));
}

void ClusteringStageContext::calculateGeodesicFromTetra(Geometry::HalfSimplex<3, GLuint>* startingTetra)
{
	busy = true;
	std::thread t([=] {
		auto colorBufferObj = reinterpret_cast<Graphics::InstancedMeshObject<glm::vec4, float>*>(mesh->signatureLookup("TETRACOLORS"));
		auto& tetraColors = colorBufferObj->extendedData;
		std::map<Geometry::HalfSimplex<3, GLuint>*, float> minimumDistanceMap;
		std::queue<std::pair<Geometry::HalfSimplex<3, GLuint>*, float>> neighbourQueue;
		neighbourQueue.push(std::make_pair(startingTetra, 0));

		while (!neighbourQueue.empty())
		{
			auto currentPair = neighbourQueue.front();
			neighbourQueue.pop();

			auto currentSimplex = currentPair.first;
			float cummulativeDistance = currentPair.second;

			if (minimumDistanceMap.find(currentSimplex) != minimumDistanceMap.end())
			{
				if (cummulativeDistance >= minimumDistanceMap[currentSimplex])
				{
					continue;
				}
			}

			minimumDistanceMap[currentSimplex] = cummulativeDistance;
			auto color = 1.0f - glm::clamp(1.0f / cummulativeDistance, 0.0f, 1.0f);
			tetraColors[simplexToBufferInstanceMap[currentSimplex]] = glm::vec4(1, color, color, 1);
			updateColorBuffer = true;

			auto centroid = HalfSimplexGeometryUtils::getSimplexCentroid(currentSimplex, positions);

			auto neighbours = currentSimplex->getAdjacentSimplices();

			for (const auto& neighbour : neighbours)
			{
				auto n = reinterpret_cast<Geometry::HalfSimplex<3, GLuint>*>(neighbour);
				auto otherCentroid = HalfSimplexGeometryUtils::getSimplexCentroid(n, positions);

				neighbourQueue.push(std::make_pair(n, cummulativeDistance + length(otherCentroid - centroid)));
			}
		}

		busy = false;
	});
	t.detach();
}