#pragma once
#include <vector>
#include <map>
#include <thread>
#include <mutex>

template <typename VectorType, typename OutputType, typename NodeType> class KdTree;

template <typename VectorType, typename OutputType, typename NodeType> class Node
{
protected:
	int mapCoordinateAtDimension(float coordinate, int dim, int numOfDivisions)
	{
		float step = (this->max[dim] - this->min[dim]) / numOfDivisions;
		int result = (coordinate - this->min[dim]) / step;

		if (result == numOfDivisions)
		{
			result--;
		}

		if (result < 0)
		{
			result = 0;
		}

		return result;
	};

	void makeAllPossibleNodes(const std::vector < std::vector<std::pair<float, float>>>& minsAndMaxPerDim,
		VectorType min = VectorType(), VectorType max = VectorType(), int depth = 0)
	{
		if (depth >= minsAndMaxPerDim.size())
		{
			std::vector<int> dimensionalIndices;
//			std::cout << "CREATED KD NODE: (";
			for (int i = 0; i < depth; ++i)
			{
				dimensionalIndices.push_back(mapCoordinateAtDimension(min[i], i, minsAndMaxPerDim[i].size()));
//				std::cout << dimensionalIndices[i];
//				if (i < depth - 1)
//					std::cout << ", ";
			}
//			std::cout << ")" << std::endl;

			children[dimensionalIndices] = new NodeType(treeData, min, max);
			return;
		}

		for (int i = 0; i < minsAndMaxPerDim[depth].size(); ++i)
		{
			min[depth] = minsAndMaxPerDim[depth][i].first;
			max[depth] = minsAndMaxPerDim[depth][i].second;

			makeAllPossibleNodes(minsAndMaxPerDim, min, max, depth + 1);
		}
	};

	KdTree<VectorType, OutputType, NodeType>* treeData;
	std::vector<int> pointOriginalIndices;
	VectorType min, max;
	std::map<std::vector<int>, NodeType*> children;
public:
	Node(KdTree<VectorType, OutputType, NodeType>* tree, VectorType min, VectorType max) : treeData(tree), min(min), max(max) {};
	~Node()
	{
		for (auto& child : children)
		{
			delete child.second;
		}
	};
	virtual void insert(int index)
	{
		pointOriginalIndices.push_back(index);
		std::sort(pointOriginalIndices.begin(), pointOriginalIndices.end());

		/*		std::sort(points.begin(), points.end(), [point](const VectorType& vec1, const VectorType& vec2) -> bool
				{
					for (int i = 0; i < point.length(); ++i)
					{
						if (vec1[i] < vec2[i])
						{
							return true;
						}
						if (vec1[i] > vec2[i])
						{
							return false;
						}
					}
					return false;
				});*/
	};

	void subdivide(int divisor = 2, int minSize = 0)
	{
		if (pointOriginalIndices.size() <= std::max(minSize, 0))
		{
			return;
		}

		float minDimensionSize = INFINITY;
		VectorType dimensionLengths = max - min;

		for (int i = 0; i < treeData->points[0].length(); ++i)
		{
			if (dimensionLengths[i] < minDimensionSize)
			{
				minDimensionSize = dimensionLengths[i];
			}
		}

		if (minDimensionSize < 0.01)
		{
			return;
		}

		std::vector<int> divisionsPerDimension;
		std::vector < std::vector<std::pair<float, float>>> minsAndMaxPerDim;
		for (int i = 0; i < treeData->points[0].length(); ++i)
		{
			float ratio = dimensionLengths[i] / minDimensionSize;
			divisionsPerDimension.push_back(divisor * ratio);

			std::vector<std::pair<float, float>> minsAndMax;
			float step = dimensionLengths[i] / divisionsPerDimension[i];
			for (float j = min[i]; j < max[i]; j += step)
			{
				minsAndMax.push_back(std::make_pair(j, j + step));
			}

			minsAndMaxPerDim.push_back(minsAndMax);
		}

		makeAllPossibleNodes(minsAndMaxPerDim);

		// make fractionally-strided child nodes

		for (int j = 0; j < pointOriginalIndices.size(); ++j)
		{
			std::vector<int> dimensionalIndices;
			for (int i = 0; i < treeData->points[pointOriginalIndices[j]].length(); ++i)
			{
				dimensionalIndices.push_back(mapCoordinateAtDimension(treeData->points[pointOriginalIndices[j]][i], i, minsAndMaxPerDim[i].size()));
			}

			if (children[dimensionalIndices] != nullptr)
			{
				children[dimensionalIndices]->insert(pointOriginalIndices[j]);
			}
		}

		if (children.size() == 1)
		{
			children.clear();
			return;
		}

		std::vector<std::vector<int>> deleteChildren;
		for (const auto& child : children)
		{
			if (child.second != nullptr)
			{
				if (child.second->pointOriginalIndices.size() < pointOriginalIndices.size() &&
					child.second->pointOriginalIndices.size() >= minSize)
				{
					child.second->subdivide(2, 4);
				}
				else
				{
					deleteChildren.push_back(child.first);
				}
			}
		}

		for (auto& child : deleteChildren)
		{
			delete children[child];
			children.erase(child);
		}
	};

	virtual std::vector<OutputType> forwardPass() = 0
	{
		std::vector<std::vector<OutputType>> results;
		std::vector<std::vector<VectorType>> newPts;
		std::vector<std::vector<int>> newIndices;

		std::vector<std::thread> threads;
		std::mutex mut;
		for (const auto& child : children)
		{
			if (child.second != nullptr)
			{
				threads.push_back(std::thread([&]()
				{
					auto localResults = child.second->forwardPass();

					mut.lock();
					results.push_back(localResults);
					newIndices.push_back(child.second->pointOriginalIndices);
					mut.unlock();
				}));
			}
		}

		for (auto& thread : threads)
		{
			thread.join();
		}

		if (children.size())
		{
			pointOriginalIndices.clear();
		}

		std::vector<OutputType> output;

		int i = 0;
		for (const auto& result : results)
		{
			output.insert(output.end(), result.begin(), result.end());

			if (children.size())
			{
				pointOriginalIndices.insert(pointOriginalIndices.end(), newIndices[i].begin(), newIndices[i].end());
			}
			++i;
		}

		std::sort(pointOriginalIndices.begin(), pointOriginalIndices.end());

		return output;
	};

	std::vector<OutputType> output;
};

template <typename VectorType, typename OutputType, typename NodeType> class KdTree
{
public:
	KdTree() {};
	KdTree(const std::vector<VectorType>& points) : points(points)
	{
		VectorType min(INFINITY), max(-INFINITY);
		for (const auto& point : points)
		{
			for (int i = 0; i < point.length(); ++i)
			{
				if (point[i] < min[i])
				{
					min[i] = point[i];
				}

				if (point[i] > max[i])
				{
					max[i] = point[i];
				}
			}
		}

		VectorType c = (max + min) / 2.0f;

		root = new NodeType(this, min + 0.3f * normalize(min - c), max + 0.3f * normalize(max - c));

		for (int i = 0; i < points.size(); ++i)
		{
			root->insert(i);
		}

//		root->subdivide(2, 4);
	};
	~KdTree()
	{
		delete root;
	};

	std::vector<VectorType> points;
protected:
	NodeType* root;
};
