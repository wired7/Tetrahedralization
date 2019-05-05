#pragma once
#include <utility>
#include <vector>
#include <set>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <boost/preprocessor/repeat.hpp>
#include <boost/preprocessor/repeat_from_to.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/if.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/arithmetic/dec.hpp>
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/comparison/less.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <iostream>
#include <queue>

#define MANIFOLD_LIM 6

namespace Geometry
{
	template<typename tPair>
	struct second_t
	{
		typename tPair::second_type operator()(const tPair& p) const
		{
			return p.second;
		}
	};

	template<typename tMap>
	second_t<typename tMap::value_type> second(const tMap& m)
	{
		return second_t<typename tMap::value_type>();
	};

	template<typename T>
	std::vector<T> arrangeSimplexIndices(const std::vector<T>& input)
{
	T count = 0;
	T minVal = UINT_MAX;
	T minIndex = 0;
	for (const auto& vertex : input)
	{
		if (vertex < minVal)
		{
			minVal = vertex;
			minIndex = count;
		}
		count++;
	}

	T location = minIndex;

	std::vector<T> arrangedFacetIndices;
	for (int j = 0; j < input.size(); ++j, minIndex = ((minIndex + 1) % input.size()))
	{
		arrangedFacetIndices.push_back(input[minIndex]);
	}

	return arrangedFacetIndices;
}

	class TopologicalStruct
	{
	public:
		virtual ~TopologicalStruct()
		{
		};
		virtual void getAllChildren(std::vector<TopologicalStruct*>& childrenVector,
									std::unordered_set<TopologicalStruct*>& childrenSet)
		{
			auto result = childrenSet.insert(this);

			if (result.second)
			{
				childrenVector.push_back(this);
			}
		};
		virtual void getAllNthChildren(std::vector<TopologicalStruct*>& childrenVector,
									   std::unordered_set<TopologicalStruct*>& childrenSet, int childDim)
		{
			getAllChildren(childrenVector, childrenSet);
		};

		virtual std::unordered_set<TopologicalStruct*> getAdjacentSimplices()
		{
			return std::unordered_set<TopologicalStruct*>();
		};
	};

	template <const int nDimensional, typename T> class HalfSimplex;

	template <const int nDimensional, typename T>
	class HalfSimplexWithChildren : public TopologicalStruct
	{
	public:
		// lower dimensional simplices that hold that binds two
		// simplices together. The direction follows winding
		// order.
		HalfSimplex<nDimensional - 1, T>* pointsTo = nullptr;

		virtual ~HalfSimplexWithChildren()
		{
		}

		void getAllChildren(std::vector<TopologicalStruct*>& childrenVector,
							std::unordered_set<TopologicalStruct*>& childrenSet) override
		{
			auto current = pointsTo;

			std::unordered_set<TopologicalStruct*> visited;

			while (current != nullptr && visited.find(current) == visited.end())
			{
				visited.insert(current);
				auto result = childrenSet.insert(current);

				if (result.second)
				{
					childrenVector.push_back(current);
				}
				
				current = current->next;
			}
		};

		void getAllNthChildren(std::vector<TopologicalStruct*>& childrenVector,
							   std::unordered_set<TopologicalStruct*>& childrenSet, int childDim) override
		{
			if (childDim > nDimensional)
			{
				return;
			}

			if (childDim == nDimensional)
			{
				auto result0 = childrenSet.insert(this);

				if (result0.second)
				{
					childrenVector.push_back(this);
				}

				return;
			}

			std::vector<TopologicalStruct*> immediateChildrenVector;
			std::unordered_set<TopologicalStruct*> immediateChildrenSet;

			if (nDimensional > 1)
			{
				getAllChildren(immediateChildrenVector, immediateChildrenSet);

				for (const auto& child : immediateChildrenVector)
				{
					child->getAllNthChildren(childrenVector, childrenSet, childDim);
				}
			}
			else if(nDimensional == 1)
			{
				if(pointsTo != nullptr)
					pointsTo->getAllNthChildren(childrenVector, childrenSet, childDim);
			}
		};

		std::unordered_set<TopologicalStruct*> getAdjacentSimplices() override
		{
			std::unordered_set<TopologicalStruct*> adjacentSet;

			std::vector<TopologicalStruct*> childrenVec;
			std::unordered_set<TopologicalStruct*> childrenSet;
			getAllChildren(childrenVec, childrenSet);
//			std::cout << childrenVec.size() << " CHILDREN" << std::endl;
			for (const auto& child : childrenVec)
			{
				auto childSimplex = reinterpret_cast<HalfSimplex<nDimensional - 1, T>*>(child);
				if (childSimplex->twin)
				{
					adjacentSet.insert(childSimplex->twin->belongsTo);
				}
			}

			return adjacentSet;
		};

		void setPointsTo(HalfSimplex<nDimensional - 1, T>* pointsTo)
		{
			this->pointsTo = pointsTo;
		};
	};

	template<const int nDimensional, typename T>
	class FullSimplex
	{
	public:
		std::unordered_set<HalfSimplex<nDimensional, T>*> halfSimplices;
		std::vector<T> data;

		FullSimplex(std::vector<T> newData) : data(newData) {};
	};

	template <const int nDimensional, typename T>
	class HalfSimplex : public std::conditional<nDimensional, HalfSimplexWithChildren<nDimensional, T>, TopologicalStruct>::type
	{
	protected:
		const int dimension = nDimensional;
	public:
		T halfSimplexData;
		FullSimplex<nDimensional, T>* fullSimplex = nullptr;
		// must have opposite orientation (the loops of pointsTo objects must
		// have opposite orders)
		HalfSimplex<nDimensional, T>* twin = nullptr;
		HalfSimplex<nDimensional, T>* next = nullptr;
		HalfSimplex<nDimensional, T>* previous = nullptr;

		// larger-dimensional construct that this half-simplex belongs to.
		HalfSimplex<nDimensional + 1, T>* belongsTo = nullptr;

		HalfSimplex() {};
		HalfSimplex(T halfSimplexData) : halfSimplexData(halfSimplexData) {};
		~HalfSimplex()
		{
			if (twin != nullptr)
				twin->twin = nullptr;
			if (next != nullptr)
				next->previous = nullptr;
			if (previous != nullptr)
				previous->next = nullptr;
			if (fullSimplex != nullptr)
				fullSimplex->halfSimplices.erase(this);
		};

		void setTwin(HalfSimplex<nDimensional, T>* twin)
		{
			this->twin = twin;
			twin->twin = this;
		};

		void setNext(HalfSimplex<nDimensional, T>* next)
		{
			this->next = next;
		};

		void setPrevious(HalfSimplex<nDimensional, T>* previous)
		{
			this->previous = previous;
		};

		void setBelongsTo(HalfSimplex<nDimensional + 1, T>* belongsTo)
		{
			this->belongsTo = belongsTo;
		};

		template <const int simplexDim, const int highestDim>
		std::unordered_set<HalfSimplex<simplexDim, T>*> getIncidentSimplices(HalfSimplex<highestDim, T>* highestParent)
		{
			// get the set of lowest components in the structure
			std::set<TopologicalStruct*> lowestChildrenSet;
			if (nDimensional)
			{
				std::vector<TopologicalStruct*> lowestChildrenVec;				
				getAllNthChildren(lowestChildrenVec, lowestChildrenSet, 0);
			}
			else
			{
				lowestChildrenSet.insert(this);
			}

			std::queue<HalfSimplex<highestDim, T>*> neighbourQueue;
			neighbourQueue.push(highestParent);

			std::set<HalfSimplex<simplexDim, T>*> incidentSet;
			std::set<HalfSimplex<highestDim, T>*> visitedSet;

			while (!neighbourQueue.empty())
			{
				auto currentHighestParent = neighbourQueue.front();
				neighbourQueue.pop();

				// if currentHighestParent has already been visited, skip
				if (visitedSet.find(currentHighestParent) != visitedSet.end())
				{
					continue;
				}

				visitedSet.insert(currentHighestParent);

				// get all the children of current highestParent matching the dimension of simplexDim.
				std::set<TopologicalStruct*> simplexDimChildrenSet;
				if (highestDim == simplexDim)
				{
					simplexDimChildrenSet.insert(currentHighestParent);
				}
				else if (highestDim > simplexDim)
				{
					std::vector<TopologicalStruct*> simplexDimChildrenVec;
					getAllNthChildren(simplexDimChildrenVec, simplexDimChildrenSet, simplexDim);
				}

				bool checkNeighbours = false;
				// for each of those children, find the lowest children of those in lowestChildrenSet
				for (const auto& simplexDimChild : simplexDimChildrenSet)
				{
					auto child = reinterpret_cast<HalfSimplex<simplexDim, T>*>(simplexDimChild);

					std::set<TopologicalStruct*> lowestSet;
					std::vector<TopologicalStruct*> lowestVec;
					reinterpret_cast<HalfSimplexWithChildren<simplexDim, T>*>(simplexDimChild)->
														getAllNthChildren(lowestVec, lowestSet, 0);

					// if even one of the tests returns true, the simplexDim objects are incident
					for (const auto& lowestDimChild : lowestSet)
					{
						// if even one simplexDim object is incident in the currentHighestParent, then we the simplexDimChild is added
						// to the list of incident simplices and we check the neighbours of currentHighestParent
						if (lowestChildrenSet.find(lowestDimChild) != lowestChildrenSet.end())
						{
							checkNeighbours = true;
							incidentSet.insert(child);
							break;
						}
					}
				}

				// repeat process for each neighbour in queue
				if (checkNeighbours)
				{
					auto adjacent = currentHighestParent->getAdjacentSimplices();

					for (const auto& a : adjacent)
					{
						neighbourQueue.push(reinterpret_cast<HalfSimplex<simplexDim, T>*>(a));
					}
				}
			}

			return incidentSet;
		};

		std::map<HalfSimplex<nDimensional, T>*, HalfSimplex<nDimensional, T>*> getNeighbours()
		{
			std::map<HalfSimplex<nDimensional, T>*, HalfSimplex<nDimensional, T>*> neighbours;

			auto higherUp = reinterpret_cast<HalfSimplexWithChildren<nDimensional + 1, T>*>(belongsTo->previous);
			HalfSimplex<nDimensional, T>* neighbour = higherUp->pointsTo;

			while(true)
			{
				auto twin = reinterpret_cast<HalfSimplex<nDimensional + 1, T>*>(higherUp)->twin;

				if (twin == nullptr)
				{
					break;
				}

				auto prev = twin->next;

				if (neighbours[neighbour] == nullptr && prev != nullptr)
				{
					neighbours[neighbour] = neighbour;
				}
				else
				{
					break;
				}

				higherUp = reinterpret_cast<HalfSimplexWithChildren<nDimensional + 1, T>*>(prev);
				neighbour = higherUp->pointsTo;
			}

			return neighbours;
		}
	};

#define DEF_MAP_TYPES(i) /* ... */																			\
		std::map<std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(i), T>*>, HalfSimplex<i, T>*> map ## i;	\
		std::map<std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(BOOST_PP_INC(i)), T>*>,					\
			std::map<std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(i), T>*>,							\
				HalfSimplex<i, T>*>> twinMap ## i;															\
		std::map<std::vector<T>, FullSimplex<i, T>*> fullSimplexMap ## i;									\
		std::vector<std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(i), T>*>> addedSimplices ## i;		\


#define DEF_POP_MAPS_RECURSE(dim) /* ... */																	\
	std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(dim), T>*> populateMapsRecursively ## dim(			\
		const std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(dim), T>*>& oldIndices)					\
	{																										\
		std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(dim), T>*> indices;								\
		indices.second = oldIndices.second;																	\
		BOOST_PP_IF(BOOST_PP_GREATER(dim, 1),																\
			indices.first = arrangeSimplexIndices<T>(oldIndices.first);,									\
			indices.first = oldIndices.first;)																\
																											\
		if(map ## dim.find(indices) == map ## dim.end())													\
		{																									\
			map ## dim[indices] = new HalfSimplex<dim, T>(indices.first[0]);								\
			addedSimplices ## dim.push_back(indices);														\
		}																									\
		auto sortedIndices = indices.first;																	\
		std::sort(sortedIndices.begin(), sortedIndices.end());												\
																											\
		if(fullSimplexMap ## dim.find(sortedIndices) == fullSimplexMap ## dim.end())						\
		{																									\
			fullSimplexMap ## dim[sortedIndices] = new FullSimplex<dim, T>(sortedIndices);					\
		}																									\
																											\
		fullSimplexMap ## dim[sortedIndices]->halfSimplices.insert(map ## dim[indices]);					\
		map ## dim[indices]->fullSimplex = fullSimplexMap ## dim[sortedIndices];							\
		HalfSimplex<BOOST_PP_DEC(dim), T>* first = nullptr;													\
		std::vector<T> newIndices, previousIndices;															\
		std::pair<std::vector<T>, HalfSimplex<dim, T>*> newIndicesPair, previousIndicesPair;				\
		HalfSimplex<BOOST_PP_DEC(dim), T>* currentChild = nullptr;											\
		HalfSimplex<BOOST_PP_DEC(dim), T>* previousChild = nullptr;											\
		for(int iter = 0; iter < dim + 1; ++iter)															\
		{																									\
			previousIndices = newIndices;																	\
			newIndices.clear();																				\
																											\
				for(int start = iter; start < iter + dim; ++start)											\
				{																							\
					newIndices.push_back(indices.first[start % (dim + 1)]);									\
				}																							\
																											\
/*			if((dim % 2) && (iter % 2))																		\
			{																								\
				std::reverse(newIndices.begin(), newIndices.end());											\
			}*/																								\
			BOOST_PP_IF(BOOST_PP_GREATER(dim, 0),															\
				auto parent = map ## dim[indices];															\
				newIndicesPair = std::make_pair(newIndices, parent);										\
				BOOST_PP_CAT(newIndicesPair = populateMapsRecursively, BOOST_PP_DEC(dim))(newIndicesPair);	\
				newIndices = newIndicesPair.first;															\
				currentChild = BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[newIndicesPair];						\
				currentChild->setBelongsTo(parent);															\
				parent->setPointsTo(currentChild);															\
				if(first == nullptr)																		\
				{																							\
					first = currentChild;																	\
				}																							\
				else																						\
				{																							\
					previousIndicesPair = std::make_pair(previousIndices, parent);							\
					previousChild = BOOST_PP_CAT(map, BOOST_PP_DEC(dim))[previousIndicesPair];				\
					previousChild->setNext(currentChild);													\
					currentChild->setPrevious(previousChild);												\
				},)																							\
		}																									\
		BOOST_PP_IF(BOOST_PP_GREATER(dim, 0),																\
			first->setPrevious(currentChild);																\
			currentChild->setNext(first);																	\
		,)																									\
		return indices;																						\
	};																										\

#define DEF_RESOLVE_TWINS(i) /* ... */																		\
	void resolveTwins ## i()																				\
	{																										\
		std::set<std::pair<std::vector<T>,																	\
			HalfSimplex<BOOST_PP_INC(BOOST_PP_INC(i)), T>*>> addedSignatures;								\
		for(const auto& indices : addedSimplices ## i)														\
		{																									\
			auto sortedIndices = indices.first;																\
			std::sort(sortedIndices.begin(), sortedIndices.end());											\
			auto parent = indices.second;																	\
			HalfSimplex<BOOST_PP_INC(BOOST_PP_INC(i)), T>* grandParent = nullptr;							\
			if(parent != nullptr)																			\
			{																								\
				grandParent = parent->belongsTo;															\
			}																								\
			auto signaturePair = std::make_pair(sortedIndices, grandParent);								\
			twinMap ## i[signaturePair].insert(std::make_pair(indices, map ## i[indices]));					\
			addedSignatures.insert(signaturePair);															\
		}																									\
		addedSimplices ## i.clear();																		\
		for(const auto& signaturePair : addedSignatures)													\
		{																									\
			if(twinMap ## i[signaturePair].size() == 2)														\
			{																								\
				(*twinMap ## i[signaturePair].begin()).second->setTwin(										\
					(*std::next(twinMap ## i[signaturePair].begin(), 1)).second);							\
			}																								\
		}																									\
	};																										\

#define RESOLVE_TWINS(z, i, _) /* ... */																	\
		resolveTwins ## i();																				\


#define DEF_CALL_POP_MAPS_RECURSE(dim, indices) /* ... */													\
		populateMapsRecursively ## dim(indices);															\
		
#define MANIFOLD_CLASS_TEMPLATE(z, i, _) /* ... */															\
		template <typename T> class BOOST_PP_CAT(Manifold, BOOST_PP_CAT(i, BOOST_PP_IF(i, :					\
			public BOOST_PP_CAT(Manifold, BOOST_PP_DEC(i)) ## <T>,)))										\
		{																									\
		public:																								\
			DEF_MAP_TYPES(i)																				\
			DEF_POP_MAPS_RECURSE(i)																			\
			DEF_RESOLVE_TWINS(i)																			\
																											\
			const int dimension = i;																		\
		public:																								\
			Manifold ## i() {};																				\
				/* assumes an n-dimensional, properly tesselated structure,					
				and that winding has been taken care of. */													\
			Manifold ## i(std::vector<T> indices)															\
			{																								\
				for(int simplexPts = 0;																		\
					simplexPts < indices.size();															\
					simplexPts += (dimension + 1))															\
				{																							\
					std::vector<T> newIndices;																\
					newIndices.insert(newIndices.end(),														\
										indices.begin() + simplexPts,										\
										indices.begin() + simplexPts +										\
											dimension + 1);													\
					add ## i(std::make_pair(newIndices, nullptr));											\
				}																							\
			};																								\
																											\
			~Manifold ## i() {};																			\
																											\
			void add ## i(const std::pair<std::vector<T>, HalfSimplex<BOOST_PP_INC(i), T>*>& indices)		\
			{																								\
				DEF_CALL_POP_MAPS_RECURSE(i, indices)														\
				BOOST_PP_REPEAT_FROM_TO(0, BOOST_PP_INC(i), RESOLVE_TWINS, _)								\
			};																								\
																											\
			void erase ## i(HalfSimplex<i, T>* simplex)														\
			{																								\
				HalfSimplex<BOOST_PP_INC(BOOST_PP_INC(i)), T>* grandParent = nullptr;						\
				auto parent = simplex->belongsTo;															\
				if(parent)																					\
				{																							\
					grandParent = parent->belongsTo;														\
				}																							\
				std::vector<Geometry::TopologicalStruct*> childrenVerticesVec;								\
				std::unordered_set<Geometry::TopologicalStruct*> childrenVerticesSet;						\
				simplex->getAllNthChildren(childrenVerticesVec, childrenVerticesSet, 0);					\
				std::vector<T> indices;																		\
				std::unordered_set<T> indicesSet;															\
				for(const auto& vertex : childrenVerticesVec)												\
				{																							\
					auto vertexIndex = reinterpret_cast<HalfSimplex<0, T>*>(vertex)->halfSimplexData;		\
					if(indicesSet.insert(vertexIndex).second)												\
						indices.push_back(vertexIndex);														\
				}																							\
				std::sort(indices.begin(), indices.end());													\
				auto signaturePair = std::make_pair(indices, grandParent);									\
				BOOST_PP_IF(BOOST_PP_GREATER(i, 0),															\
				std::vector<Geometry::TopologicalStruct*> childrenVec;										\
				std::unordered_set<Geometry::TopologicalStruct*> childrenSet;								\
				simplex->getAllChildren(childrenVec, childrenSet);											\
				for (auto& childSimplex : childrenSet)														\
				{																							\
					BOOST_PP_CAT(erase,																		\
						BOOST_PP_DEC(i))(reinterpret_cast<HalfSimplex<BOOST_PP_DEC(i), T>*>(childSimplex));	\
				},)																							\
																											\
				for(auto& element : twinMap ## i[signaturePair])											\
				{																							\
					if(element.second == simplex)															\
					{																						\
						auto tempSimplex = element.second;													\
						map ## i.erase(element.first);														\
						twinMap ## i[signaturePair].erase(element.first);									\
						if(twinMap ## i[signaturePair].empty())												\
						{																					\
							twinMap ## i.erase(signaturePair);												\
						}																					\
																											\
						auto fullSimplex = tempSimplex->fullSimplex;										\
						delete tempSimplex;																	\
																											\
						if (fullSimplex->halfSimplices.empty())												\
						{																					\
							fullSimplexMap ## i.erase(indices);												\
							delete fullSimplex;																\
						}																					\
																											\
						break;																				\
					}																						\
				}																							\
			};																								\
																											\
		};			 																						\

		BOOST_PP_REPEAT(MANIFOLD_LIM, MANIFOLD_CLASS_TEMPLATE, _)

	#undef DEF_MAP_TYPES
	#undef DEF_POP_MAPS_RECURSE
	#undef MANIFOLD_CLASS_TEMPLATE
};