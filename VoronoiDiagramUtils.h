#pragma once
#include "ImplicitGeometry.h"
#include "HalfSimplices.h"
#include "KdTree.h"
#include <utility>
#include <map>
#include <set>
#include <mutex>

static class VoronoiDiagramUtils
{
public:
	static ImplicitGeo::Sphere getCircumcircle(glm::vec3 points[3]);
	static ImplicitGeo::Sphere getCircumsphere(glm::vec3 points[4]);
	static bool isSpaceDegenerate(glm::vec3 points[4]);
	static bool isPointWithinSphere(glm::vec3 point, glm::vec3 points[4]);
	static std::vector<glm::ivec4> calculateDelaunayTetrahedra(const std::vector<glm::vec3>& points);
/*	static Geometry::Vertex* getVoronoiPointFromTetrahedron(Geometry::Mesh* mesh, const std::vector<glm::vec3>& inputPositions, std::vector<glm::vec3>& outputPositions);
	static std::pair<Geometry::Vertex*, Geometry::Vertex*> getVoronoiEdgeFromTetrahedraPair(Geometry::Mesh* mesh1, Geometry::Mesh* mesh2, const std::map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	static Geometry::Facet* getVoronoiFacetFromEdge(std::pair<Geometry::Vertex*, Geometry::Vertex*> edgeVertices, const std::map<Geometry::Mesh*, Geometry::Vertex*>& voronoiVertices);
	static std::vector<std::pair<Geometry::Vertex*, Geometry::Vertex*>> getEdgesIncidentToVertex(Geometry::Vertex* vertex, Geometry::VolumetricMesh* volumetricMesh);

	static Geometry::VolumetricMesh* getVoronoiDiagram(Geometry::VolumetricMesh* volumetricMesh, const vector<vec3>& positions);*/
};

template <typename VectorType, typename OutputVectorType> class DelaunayTree;

template <typename VectorType, typename OutputType> class DelaunayNode : public Node<VectorType, OutputType, DelaunayNode<VectorType, OutputType>>
{
public:
	std::vector<OutputType> calculateDelaunaySimplices()
	{
		int size = pointOriginalIndices.size();

		if (size < 4)
		{
			return output;
		}

		DelaunayTree<VectorType, OutputType>* data = (DelaunayTree<VectorType, OutputType>*)treeData;

		std::vector<OutputType> tuples;
		std::vector<OutputType> indices;

#pragma omp parallel for schedule(dynamic, 30)
		for (int i = 0; i < pointOriginalIndices.size() - 3; ++i)
		{
			int w = pointOriginalIndices[i];
			for (int j = i + 1; j < pointOriginalIndices.size() - 2; ++j)
			{
				int x = pointOriginalIndices[j];
				for (int k = j + 1; k < pointOriginalIndices.size() - 1; ++k)
				{
					int y = pointOriginalIndices[k];

					if (data->faceCountMap[{w, x, y}] >= 2)
					{
						continue;
					}

					for (int l = k + 1; l < pointOriginalIndices.size(); ++l)
					{
						int z = pointOriginalIndices[l];

						if (data->discardedMap[{w, x, y, z}] ||
							data->faceCountMap[{w, x, z}] >= 2 ||
							data->faceCountMap[{w, y, z}] >= 2 ||
							data->faceCountMap[{x, y, z}] >= 2)
						{
							continue;
						}
#pragma omp critical
						{
							tuples.push_back(OutputType(w, x, y, z));
						}
					}
				}
			}
		}

		std::map<int, bool> pointsToErase;

#pragma omp parallel for schedule(dynamic, 30)
		for(int i = 0; i < tuples.size(); ++i)
		{
			auto tuple = tuples[i];

 			glm::vec3 pts[4] = { data->points[tuple[0]],
							data->points[tuple[1]],
							data->points[tuple[2]],
							data->points[tuple[3]] };

			if (VoronoiDiagramUtils::isSpaceDegenerate(pts))
			{
				data->discardedMap[{tuple[0], tuple[1], tuple[2], tuple[3]}] = true;					
				continue;
			}

			auto sphere = VoronoiDiagramUtils::getCircumsphere(pts);

			bool valid = true;

			if (checkBounds)
			{
				// check if circumsphere extends outside of box bounds
				for (int dim = 0; dim < 3; ++dim)
				{
					if (sphere.center[dim] - sphere.radius < this->min[dim] ||
						sphere.center[dim] + sphere.radius > this->max[dim])
					{
						valid = false;
						break;
					}
				}

				if (!valid)
				{
					continue;
				}
			}

			data->discardedMap[{tuple[0], tuple[1], tuple[2], tuple[3]}] = true;

			for (int m = 0; m < pointOriginalIndices.size(); ++m)
			{
				if (pointOriginalIndices[m] == tuple[0] ||
					pointOriginalIndices[m] == tuple[1] ||
					pointOriginalIndices[m] == tuple[2] ||
					pointOriginalIndices[m] == tuple[3])
				{
					continue;
				}

				if (length(sphere.center - data->points[pointOriginalIndices[m]]) < sphere.radius)
				{
					valid = false;
					break;
				}
			}

			if (valid)
			{
#pragma omp critical
				{
					output.push_back(tuple);

					data->faceCountMap[{tuple[0], tuple[1], tuple[2]}]++;
					data->faceCountMap[{tuple[0], tuple[1], tuple[3]}]++;
					data->faceCountMap[{tuple[0], tuple[2], tuple[3]}]++;
					data->faceCountMap[{tuple[1], tuple[2], tuple[3]}]++;

					// add all faces incident to each point, then check if all faces incident have a twin.
					// if all incident faces have a twin, the point is useless to us in the future and
					// should be removed

					data->facesAdjacentToNodeMap[tuple[0]].insert({ tuple[0], tuple[1], tuple[2] });
					data->facesAdjacentToNodeMap[tuple[1]].insert({ tuple[0], tuple[1], tuple[2] });
					data->facesAdjacentToNodeMap[tuple[2]].insert({ tuple[0], tuple[1], tuple[2] });

					data->facesAdjacentToNodeMap[tuple[0]].insert({ tuple[0], tuple[1], tuple[3] });
					data->facesAdjacentToNodeMap[tuple[1]].insert({ tuple[0], tuple[1], tuple[3] });
					data->facesAdjacentToNodeMap[tuple[3]].insert({ tuple[0], tuple[1], tuple[3] });

					data->facesAdjacentToNodeMap[tuple[0]].insert({ tuple[0], tuple[2], tuple[3] });
					data->facesAdjacentToNodeMap[tuple[2]].insert({ tuple[0], tuple[2], tuple[3] });
					data->facesAdjacentToNodeMap[tuple[3]].insert({ tuple[0], tuple[2], tuple[3] });

					data->facesAdjacentToNodeMap[tuple[1]].insert({ tuple[1], tuple[2], tuple[3] });
					data->facesAdjacentToNodeMap[tuple[2]].insert({ tuple[1], tuple[2], tuple[3] });
					data->facesAdjacentToNodeMap[tuple[3]].insert({ tuple[1], tuple[2], tuple[3] });

					for (int iter = 0; iter < tuple.length(); ++iter)
					{
						bool deletePoint = true;
						for (const auto& adjFace : data->facesAdjacentToNodeMap[tuple[iter]])
						{
							if (data->faceCountMap[adjFace] < 2)
							{
								deletePoint = false;
								break;
							}
						}

						if (deletePoint)
						{
							pointsToErase[tuple[iter]] = true;
						}
					}
				}
			}
		}

		std::vector<int> temp1;
		for (int i = 0; i < pointOriginalIndices.size(); ++i)
		{
			if (!pointsToErase[i])
			{
				temp1.push_back(pointOriginalIndices[i]);
			}
		}
		pointOriginalIndices = temp1;

		return output;
	};
public:
	DelaunayNode(KdTree<VectorType, OutputType, DelaunayNode<VectorType, OutputType>>* tree, VectorType min, VectorType max) :
		Node<VectorType, OutputType, DelaunayNode<VectorType, OutputType>>(tree, min, max) {};
	std::vector<OutputType> forwardPass()
	{
		auto result = Node<VectorType, OutputType, DelaunayNode<VectorType, OutputType>>::forwardPass();

		calculateDelaunaySimplices();

		result.insert(result.end(), output.begin(), output.end());

		for (const auto& child : children)
		{
			delete child.second;
		}

		children.clear();

		((DelaunayTree<VectorType, OutputType>*)treeData)->printMutex.lock();
//		std::cout << "CALCULATED " << result.size() << " TETRA SO FAR" << std::endl;
		((DelaunayTree<VectorType, OutputType>*)treeData)->printMutex.unlock();

		return result;
	};

	virtual void insert(int index)
	{
		Node<VectorType, OutputType, DelaunayNode<VectorType, OutputType>>::insert(index);

		oldPointIndices.push_back(index);
	};

	bool checkBounds = true;
	std::vector<int> oldPointIndices;
};

template <typename VectorType, typename OutputVectorType> class DelaunayTree :
	public KdTree<VectorType, OutputVectorType, DelaunayNode<VectorType, OutputVectorType>>
{
private:
	int simplexSize;
public:
	DelaunayTree(const std::vector<VectorType>& points) :
		KdTree<VectorType, OutputVectorType, DelaunayNode<VectorType, OutputVectorType>>(points), simplexSize(points[0].length() + 1)
	{
	};
	~DelaunayTree()
	{
		faceCountMap.clear();
		discardedMap.clear();
		facesAdjacentToNodeMap.clear();
	};
	std::vector<OutputVectorType> calculate()
	{
		root->checkBounds = false;
		return root->forwardPass();
	};

	std::map<std::vector<int>, int> faceCountMap;
	std::map<std::vector<int>, bool> discardedMap;
	std::map<int, std::set<std::vector<int>>> facesAdjacentToNodeMap;
	std::mutex printMutex;
};