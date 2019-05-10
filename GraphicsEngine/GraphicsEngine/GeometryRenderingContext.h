#pragma once
#include "Context.h"

template<class ControllerType, class CameraType, class ContextType>
class GeometryRenderingContext : public GraphicsSceneContext<ControllerType, CameraType, ContextType>
{
private:
	virtual void addAllRenderableObjects(GeometryPass* pass,
										 std::vector<std::pair<std::string, std::string>> meshShaderPairs);
	virtual void removeAllRenderableObjects(GeometryPass* pass,
											std::vector<std::pair<std::string, std::string>> meshShaderPairs);
protected:
	GeometryRenderingContext();
	virtual ~GeometryRenderingContext() {};
	virtual std::vector<std::pair<std::string, std::string>> getVolumePairs();
	virtual std::vector<std::pair<std::string, std::string>> getSurfacePairs();
	virtual std::vector<std::pair<std::string, std::string>> getEdgePairs();
	virtual std::vector<std::pair<std::string, std::string>> getVertexPairs();
	virtual GeometryPass* getGeometryPass();
public:
	virtual void addVolumes();
	virtual void addSurfaces();
	virtual void addEdges();
	virtual void addVertices();
	virtual void removeVolumes();
	virtual void removeSurfaces();
	virtual void removeEdges();
	virtual void removeVertices();
};

template<class ControllerType, class CameraType, class ContextType>
GeometryRenderingContext<ControllerType, CameraType, ContextType>::GeometryRenderingContext() :
	GraphicsSceneContext<ControllerType, CameraType, ContextType>()
{
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::
addAllRenderableObjects(GeometryPass* pass,
						std::vector<std::pair<std::string, std::string>> meshShaderPairs)
{
	for (const auto& currentPair : meshShaderPairs)
	{
		pass->addRenderableObjects(geometries[currentPair.first], currentPair.first, currentPair.second);
	}
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::
removeAllRenderableObjects(GeometryPass* pass,
						   std::vector<std::pair<std::string, std::string>> meshShaderPairs)
{
	for (const auto& currentPair : meshShaderPairs)
	{
		pass->clearRenderableObjects(currentPair.first, currentPair.second);
	}
};

template<class ControllerType, class CameraType, class ContextType>
GeometryPass* GeometryRenderingContext<ControllerType, CameraType, ContextType>::getGeometryPass()
{
	return (GeometryPass*)passRootNode->signatureLookup("GEOMETRYPASS");
}

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::addVolumes()
{
	addAllRenderableObjects(getGeometryPass(), getVolumePairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::addSurfaces()
{
	addAllRenderableObjects(getGeometryPass(), getSurfacePairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::addEdges()
{
	addAllRenderableObjects(getGeometryPass(), getEdgePairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::addVertices()
{
	addAllRenderableObjects(getGeometryPass(), getVertexPairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::removeVolumes()
{
	removeAllRenderableObjects(getGeometryPass(), getVolumePairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::removeSurfaces()
{
	removeAllRenderableObjects(getGeometryPass(), getSurfacePairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::removeEdges()
{
	removeAllRenderableObjects(getGeometryPass(), getEdgePairs());
};

template<class ControllerType, class CameraType, class ContextType>
void GeometryRenderingContext<ControllerType, CameraType, ContextType>::removeVertices()
{
	removeAllRenderableObjects(getGeometryPass(), getVertexPairs());
};

template<class ControllerType, class CameraType, class ContextType>
std::vector<std::pair<std::string, std::string>> GeometryRenderingContext<ControllerType, CameraType, ContextType>::getVolumePairs()
{
	return std::vector<std::pair<std::string, std::string>>();
};

template<class ControllerType, class CameraType, class ContextType>
std::vector<std::pair<std::string, std::string>> GeometryRenderingContext<ControllerType, CameraType, ContextType>::getSurfacePairs()
{
	return std::vector<std::pair<std::string, std::string>>();
};

template<class ControllerType, class CameraType, class ContextType>
std::vector<std::pair<std::string, std::string>> GeometryRenderingContext<ControllerType, CameraType, ContextType>::getEdgePairs()
{
	return std::vector<std::pair<std::string, std::string>>();
};

template<class ControllerType, class CameraType, class ContextType>
std::vector<std::pair<std::string, std::string>> GeometryRenderingContext<ControllerType, CameraType, ContextType>::getVertexPairs()
{
	return std::vector<std::pair<std::string, std::string>>();
};