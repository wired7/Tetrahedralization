#pragma once
#include "GraphicsObject.h"
#include <map>
#include <utility>

//Stores and manages reference graphics object and instance index within it associated with map index
namespace Graphics
{
	class ReferenceManager
	{
	private:
		unsigned int count = 0;
		std::map<DecoratedGraphicsObject*, std::pair<DecoratedGraphicsObject*, std::vector<std::pair<int, int>>>> managedGraphicsObject;
		std::map<int, std::pair<DecoratedGraphicsObject*, int>> inverseLookupMap;
	public:
		ReferenceManager();
		~ReferenceManager();
		int assignNewGUID(DecoratedGraphicsObject* gObject, int indexWithinObject = 0);
		int assignNewGUID(void);
		std::pair<DecoratedGraphicsObject*, int> getInstance(int guid);
		void deleteRange(DecoratedGraphicsObject*, int minIndex, int maxIndex);
	};

	template<class T, class S> class ReferencedGraphicsObject : public InstancedMeshObject<T, S>
	{
	public:
		ReferencedGraphicsObject(ReferenceManager* refMan, DecoratedGraphicsObject* child, int numInstances, std::string bufferSignature, int divisor);
		~ReferencedGraphicsObject();
	};

	template<class T, class S>
	ReferencedGraphicsObject<T, S>::ReferencedGraphicsObject(ReferenceManager* refMan, DecoratedGraphicsObject* child, int numInstances,
		std::string bufferSignature, int divisor) :
		InstancedMeshObject<T, S>(child, bufferSignature, divisor)
	{
		for (int i = 0; i < numInstances; i++)
		{
			extendedData.push_back(refMan->assignNewGUID(this, i));
		}

		glDeleteBuffers(1, &VBO);
		bindBuffers();
	}


	template<class T, class S> ReferencedGraphicsObject<T, S>::~ReferencedGraphicsObject()
	{
	}

	class SelectionManager
	{
	public:
		std::vector<std::pair<DecoratedGraphicsObject*, int>> selectedGraphicsObject;
		ReferenceManager* refMan;
	};
}