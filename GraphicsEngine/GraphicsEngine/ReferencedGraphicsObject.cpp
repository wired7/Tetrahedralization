#pragma once
#include "ReferencedGraphicsObject.h"

namespace Graphics
{
	ReferenceManager::ReferenceManager()
	{
	}

	ReferenceManager::~ReferenceManager()
	{
	}

	int ReferenceManager::assignNewGUID(DecoratedGraphicsObject* gObject, int indexWithinObject)
	{
		if (managedGraphicsObject.find(gObject) != managedGraphicsObject.end())
		{
			for (const auto& object : managedGraphicsObject[gObject].second)
			{
				if (indexWithinObject == object.first)
				{
					return object.second;
				}
			}

			managedGraphicsObject[gObject].second.push_back(std::pair<int, int>(indexWithinObject, count));
			inverseLookupMap[count++] = std::make_pair(gObject, indexWithinObject);
			return count - 1;
		}

		managedGraphicsObject[gObject] = std::pair<DecoratedGraphicsObject*, std::vector<std::pair<int, int>>>(gObject,
			{ std::pair<int, int>(indexWithinObject, count) });
		inverseLookupMap[count++] = std::make_pair(gObject, indexWithinObject);
		return count - 1;
	}

	int ReferenceManager::assignNewGUID(void)
	{
		return count++;
	}

	std::pair<DecoratedGraphicsObject*, int> ReferenceManager::getInstance(int guid)
	{
		if (inverseLookupMap.find(guid) != inverseLookupMap.end())
		{
			return inverseLookupMap[guid];
		}

		return std::make_pair(nullptr, 0);
	}
}