#pragma once
#include "HalfSimplices.h"
#include <algorithm>
#include <functional>
#include <glew.h>

static class HalfSimplexGeometryUtils
{
private:
	template <const int simplexDim>
	struct HSPairCompare
	{
		bool operator()(const std::pair<Geometry::HalfSimplex<simplexDim, GLuint>*, float>& pair1,
						const std::pair<Geometry::HalfSimplex<simplexDim, GLuint>*, float>& pair2) const
		{
			return pair1.second > pair2.second;
		}
	};

public:
	template <const int simplexDim, typename embeddingDimVector>
	static embeddingDimVector getSimplexCentroid(Geometry::HalfSimplex<simplexDim, GLuint>* currentSimplex,
												 const std::vector<embeddingDimVector>& positions)
	{
		std::vector<Geometry::TopologicalStruct*> vertexVec;
		std::unordered_set<Geometry::TopologicalStruct*> vertexSet;
		currentSimplex->getAllNthChildren(vertexVec, vertexSet, 0);

		// Get centroid of current simplex
		embeddingDimVector simplexCentroid;

		for (const auto& currentSimplexVertex : vertexSet)
		{
			simplexCentroid += positions[reinterpret_cast<Geometry::HalfSimplex<0, GLuint>*>(currentSimplexVertex)->halfSimplexData];
		}

		simplexCentroid /= vertexSet.size();

		return simplexCentroid;
	}

	template <const int simplexDim, typename embeddingDimVector>
	static Geometry::HalfSimplex<simplexDim, GLuint>* findSimplex(Geometry::HalfSimplex<simplexDim, GLuint>* currentSimplex,
																  std::vector<embeddingDimVector>& positions,
																  embeddingDimVector targetPoint,
																  std::function<bool(Geometry::HalfSimplex<simplexDim, GLuint>*,
																					 std::vector<embeddingDimVector>&,
																					 embeddingDimVector)> testFunction,
																  std::unordered_set<Geometry::HalfSimplex<simplexDim, GLuint>*>& visitedSet =
																  std::unordered_set<Geometry::HalfSimplex<simplexDim, GLuint>*>())
	{
		if (visitedSet.find(currentSimplex) != visitedSet.end())
		{
			return nullptr;
		}

		visitedSet.insert(currentSimplex);

		if (testFunction(currentSimplex, positions, targetPoint))
		{
			return currentSimplex;
		}

		embeddingDimVector simplexCentroid = getSimplexCentroid<simplexDim, embeddingDimVector>(currentSimplex, positions);
		embeddingDimVector movementDirection = normalize(targetPoint - simplexCentroid);

		// Get neighbouring simplices

		auto neighbours = reinterpret_cast<Geometry::HalfSimplexWithChildren<simplexDim, GLuint>*>(currentSimplex)->getAdjacentSimplices();
		std::set<std::pair<Geometry::HalfSimplex<simplexDim, GLuint>*, float>, HSPairCompare<simplexDim>> neighbourPairs;

		for (const auto& neighbour : neighbours)
		{
			auto n = reinterpret_cast<Geometry::HalfSimplex<simplexDim, GLuint>*>(neighbour);
			embeddingDimVector directionToNeighborSimplex =
				glm::normalize(getSimplexCentroid<simplexDim, embeddingDimVector>(n, positions) - simplexCentroid);
			neighbourPairs.insert(std::make_pair(n, glm::dot(movementDirection, directionToNeighborSimplex)));
		}

		Geometry::HalfSimplex<simplexDim, GLuint>* output = nullptr;

		for (const auto& neighbour : neighbourPairs)
		{
			output = findSimplex<simplexDim, embeddingDimVector>(neighbour.first, positions, targetPoint, testFunction, visitedSet);

			if (output != nullptr)
			{
				break;
			}
		}

		return output;
	}
};

