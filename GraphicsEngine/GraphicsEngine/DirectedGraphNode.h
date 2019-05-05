#pragma once
#include <vector>
#include <string>

template<class T> class DirectedGraphNode
{
protected:
	std::vector<T*> neighbors;
	std::vector<T*> parents;
public:
	std::string signature;
	DirectedGraphNode();
	DirectedGraphNode(std::vector<T*> neighbors, std::string signature);
	DirectedGraphNode(std::string signature);
	~DirectedGraphNode();
	void addNeighbor(T* neighbor);
	void addNeighbors(std::vector<T*> neighbors);
	std::vector<T*> getLeaves();
	virtual T* clone();
	// TODO: Recursive Lookup of signature through children
	virtual T* signatureLookup(std::string signature);
	virtual std::vector<T*>* signatureLookup(std::vector<std::string>& signatures);
	// TODO: Look for buffer with signature and call the function at occurrence
	virtual bool signatureCallback(std::string signature/*, function callback*/);
	virtual bool signatureCallback(std::vector<std::string> signatures/*, function callback*/);
	// TODO: Look for the signature recursively, unbind it from the VAO, delete VBO, return its child, and reorganize layout indices from any child and parent
	// so that they're sequential, and rebind every buffer that was shifted to the proper layout number
	virtual T* remove(std::string signature);
	virtual T* remove(std::vector<std::string> signatures);
};

template<class T> DirectedGraphNode<T>::DirectedGraphNode()
{
}

template<class T> DirectedGraphNode<T>::DirectedGraphNode(std::vector<T*> neighbors, std::string signature) : neighbors(neighbors), signature(signature)
{
}

template<class T> DirectedGraphNode<T>::DirectedGraphNode(std::string signature) : signature(signature)
{
}

template<class T> DirectedGraphNode<T>::~DirectedGraphNode()
{

}

template<class T> void DirectedGraphNode<T>::addNeighbor(T* neighbor)
{
	neighbors.push_back(neighbor);
	neighbor->parents.push_back((T*)this);
}

template <class T> T* DirectedGraphNode<T>::clone()
{
	return nullptr;
}

template <class T> T* DirectedGraphNode<T>::signatureLookup(std::string signature)
{
	if (signature == this->signature)
	{
		return (T*)this;
	}

	for (int i = 0; i < neighbors.size(); i++)
	{
		auto output = neighbors[i]->signatureLookup(signature);

		if (output != nullptr)
		{
			return output;
		}
	}

	return nullptr;
}

template <class T> std::vector<T*>* DirectedGraphNode<T>::signatureLookup(std::vector<std::string>& signatures)
{
	return nullptr;
}

template <class T> bool DirectedGraphNode<T>::signatureCallback(std::string signature/*, function callback*/)
{
	return false;
}

template <class T> bool DirectedGraphNode<T>::signatureCallback(std::vector<std::string> signatures/*, function callback*/)
{
	return false;
}


template <class T> T* DirectedGraphNode<T>::remove(std::string signature)
{
	return NULL;
}

template <class T> T* DirectedGraphNode<T>::remove(std::vector<std::string> signatures)
{
	return NULL;
}