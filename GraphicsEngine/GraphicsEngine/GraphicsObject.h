#pragma once
#include "Decorator.h"
#include "glew.h"
#include "glm.hpp"

// Make a factory to avoid creating erroneous patterns!
// TODO: Add a uniform references array that somehow links to the shader
namespace Graphics {
	struct Vertex {
		glm::vec3 position;
		glm::vec3 normal;

		Vertex() {};
		Vertex(glm::vec3 pos, glm::vec3 norm) : position(pos), normal(norm) {};
	};

	class DecoratedGraphicsObject : public Decorator<DecoratedGraphicsObject>
	{
	protected:
		glm::mat4 model = glm::mat4(1.0f);
	public:
		GLuint VAO;
		GLuint VBO;
		bool dirty = false;
		int layoutCount;
		DecoratedGraphicsObject() {};
		DecoratedGraphicsObject(DecoratedGraphicsObject* child, std::string bufferSignature);
		~DecoratedGraphicsObject();
		virtual void addVertex(glm::vec3 pos, glm::vec3 normal) = 0;
		virtual void commitVBOToGPU(void) = 0;
		virtual void bindBuffers(void) = 0;
		virtual void updateBuffers(void) = 0;
		// TODO: Assign signatures to buffers so as to allow updates to be applied cross-signature and discontinuously, and not necessarily hollistically
		virtual void updateBuffers(std::vector<std::string> bufferSignatures);
		virtual void updateBuffersPartially(int minBufferIndex, int maxBufferIndex);
		virtual void updateBuffersPartially(int minBufferIndex, int maxBufferIndex, std::vector<std::string> bufferSignatures);
		virtual void enableBuffers(void);
		virtual void draw(void) = 0;
		virtual void updateIfDirty(void) = 0;
		virtual std::string printOwnProperties(void);
		glm::mat4 getModelMatrix();
	};

	class MeshObject : public DecoratedGraphicsObject
	{
	public:
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		GLuint EBO;
		int commitedVertexCount;
		int commitedIndexCount;

		MeshObject();
		MeshObject(std::vector<Vertex> vertices, std::vector<GLuint> indices);
		~MeshObject();

		virtual DecoratedGraphicsObject* make() { return NULL; };
		virtual void computeNormals() {};
		virtual void addVertex(glm::vec3 pos, glm::vec3 normal = glm::vec3());
		virtual void addTriangle(int a, int b, int c) {};
		// TODO: apply vertex deletion, but careful! when a vertex is deleted from the buffer, all of the indices change, and it's necessary to also
		// remove any triangles that may refer to that vertex index first
		virtual void deleteVertex(int index) {};
		// TODO: apply triangle deletion by looking for matching triples in any order within the indices array
		virtual void deleteTriangle(int a, int b, int c) {};
		virtual void commitVBOToGPU(void);
		virtual void bindBuffers(void);
		virtual void updateBuffers(void);
		virtual void draw(void);
		virtual void updateIfDirty(void);
	};

	class ImportedMeshObject : public MeshObject
	{
	public:
		ImportedMeshObject(const char* filePath);
		~ImportedMeshObject() {};
		void loadFile(const char* filePath);
	};

	template <class T, class S> class ExtendedMeshObject : public DecoratedGraphicsObject
	{
	public:
		std::vector<T> extendedData;
		int commitedExtendedData;

		ExtendedMeshObject() {};
		ExtendedMeshObject(DecoratedGraphicsObject* child, std::string bufferSignature);
		ExtendedMeshObject(DecoratedGraphicsObject* child, std::vector<T> data, std::string bufferSignature);
		~ExtendedMeshObject();

		virtual DecoratedGraphicsObject* make();
		virtual void addVertex(glm::vec3 pos, glm::vec3 normal);
		virtual void commitVBOToGPU(void);
		virtual void bindBuffers(void);
		virtual void updateBuffers(void);
		virtual void draw(void);
		virtual void updateIfDirty(void);
	};

#pragma region ExtendedMeshObjectTemplate
	template <class T, class S> ExtendedMeshObject<T, S>::ExtendedMeshObject(DecoratedGraphicsObject* child, std::string bufferSignature) :
		DecoratedGraphicsObject(child, bufferSignature)
	{
		layoutCount = child->layoutCount + 1;
		VAO = child->VAO;
	}

	template <class T, class S> ExtendedMeshObject<T, S>::ExtendedMeshObject(DecoratedGraphicsObject* child, std::vector<T> data, std::string bufferSignature) :
		DecoratedGraphicsObject(child, bufferSignature), extendedData(data)
	{
		layoutCount = child->layoutCount + 1;
		VAO = child->VAO;
		bindBuffers();
	}

	template <class T, class S> ExtendedMeshObject<T, S>::~ExtendedMeshObject(void)
	{
		glDeleteBuffers(1, &VBO);
	}

	template <class T, class S> DecoratedGraphicsObject* ExtendedMeshObject<T, S>::make(void)
	{
		return new ExtendedMeshObject<T, S>(nullptr, extendedData, signature);
	}

	template <class T, class S> void ExtendedMeshObject<T, S>::addVertex(glm::vec3 pos, glm::vec3 normal)
	{
		child->addVertex(pos, normal);
	}

	template <class T, class S> void ExtendedMeshObject<T, S>::commitVBOToGPU(void)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, extendedData.size() * sizeof(T), &(extendedData[0]), GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(layoutCount - 1);
		glVertexAttribPointer(layoutCount - 1, sizeof(T) / sizeof(S), GL_FLOAT, GL_FALSE, sizeof(T), (GLvoid*)0);

		glBindVertexArray(0);

		commitedExtendedData = extendedData.size();
	}

	template <class T, class S> void ExtendedMeshObject<T, S>::bindBuffers(void)
	{
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);

		commitVBOToGPU();
	}

	template <class T, class S> void ExtendedMeshObject<T, S>::updateBuffers(void)
	{
		glBindVertexArray(VAO);

		commitVBOToGPU();
	}

	template <class T, class S> void ExtendedMeshObject<T, S>::draw(void)
	{
		child->draw();
	}

	template <class T, class S> void ExtendedMeshObject<T, S>::updateIfDirty(void)
	{
		if (dirty)
		{
			if (extendedData.size() != commitedExtendedData)
			{
				glBindVertexArray(VAO);
				glDeleteBuffers(1, &VBO);

				glGenBuffers(1, &VBO);

				commitVBOToGPU();
			}
			else
			{
				updateBuffers();
			}

			dirty = false;
		}
	}

#pragma endregion

	class TexturedMeshObject : public ExtendedMeshObject<glm::vec2, float>
	{
	public:
		GLuint texture;

		TexturedMeshObject(DecoratedGraphicsObject* child, char* filename, std::vector<glm::vec2> data);
		TexturedMeshObject(DecoratedGraphicsObject* child, GLuint texture, std::vector<glm::vec2> data);
		~TexturedMeshObject() {};

		virtual void loadTexture(char* filePath);
		virtual void enableBuffers(void);
	};

	template <class T, class S> class InstancedMeshObject : public ExtendedMeshObject<T, S>
	{
	public:
		MeshObject* instancedObject;
		int divisor;

		InstancedMeshObject() {};
		InstancedMeshObject(DecoratedGraphicsObject* child, std::string bufferSignature, int divisor = 1);
		InstancedMeshObject(DecoratedGraphicsObject* child, std::vector<T> data, std::string bufferSignature, int divisor = 1);
		~InstancedMeshObject() {};

		virtual void commitVBOToGPU(void);
		virtual void draw(void);
	};

#pragma region InstancedMeshObjectTemplate
	template <class T, class S> InstancedMeshObject<T, S>::InstancedMeshObject(DecoratedGraphicsObject* child, std::string bufferSignature, int divisor) :
		ExtendedMeshObject<T, S>(child, bufferSignature), divisor(divisor)
	{
		instancedObject = (MeshObject*)signatureLookup("VERTEX");
	};

	template <class T, class S> InstancedMeshObject<T, S>::InstancedMeshObject(DecoratedGraphicsObject* child, std::vector<T> data,
																			   std::string bufferSignature, int divisor) :
		ExtendedMeshObject<T, S>(child, bufferSignature), divisor(divisor)
	{
		instancedObject = (MeshObject*)signatureLookup("VERTEX");
		extendedData = data;
		bindBuffers();
	};

	template <class T, class S> void InstancedMeshObject<T, S>::commitVBOToGPU(void)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, extendedData.size() * sizeof(T), &(extendedData[0]), GL_DYNAMIC_DRAW);

		auto glType = GL_FLOAT;

		if (std::is_same<S, GLdouble>::value || std::is_same<S, double>::value)
		{
			glType = GL_DOUBLE;
		}
		else if (std::is_same<S, GLbyte>::value || std::is_same<S, bool>::value)
		{
			glType = GL_BYTE;
		}

		glEnableVertexAttribArray(layoutCount - 1);
		glVertexAttribPointer(layoutCount - 1, sizeof(T) / sizeof(S), glType, GL_FALSE, sizeof(T), (GLvoid*)0);
		glVertexAttribDivisor(layoutCount - 1, divisor);

		glBindVertexArray(0);
	}

	template <class T, class S> void InstancedMeshObject<T, S>::draw(void)
	{
		glDrawElementsInstanced(GL_TRIANGLES, instancedObject->indices.size(), GL_UNSIGNED_INT, 0, extendedData.size() * divisor);
		glBindVertexArray(0);
	}
#pragma endregion

	template <class T, class S> class MatrixInstancedMeshObject : public InstancedMeshObject<T, S>
	{
	public:
		MatrixInstancedMeshObject() {};
		MatrixInstancedMeshObject(DecoratedGraphicsObject* child, std::string bufferSignature, int divisor = 1);
		MatrixInstancedMeshObject(DecoratedGraphicsObject* child, std::vector<T> data, std::string bufferSignature, int divisor = 1);
		~MatrixInstancedMeshObject() {};

		virtual void commitVBOToGPU(void);
	};

#pragma region MatrixInstancedObjectTemplate
	template <class T, class S> MatrixInstancedMeshObject<T, S>::MatrixInstancedMeshObject(DecoratedGraphicsObject* child,
																						   std::string bufferSignature, int divisor) :
		InstancedMeshObject<T, S>(child, bufferSignature, divisor)
	{
		layoutCount += 3;
	}

	template <class T, class S> MatrixInstancedMeshObject<T, S>::MatrixInstancedMeshObject(DecoratedGraphicsObject* child, std::vector<T> data,
																						   std::string bufferSignature, int divisor) :
		InstancedMeshObject<T, S>(child, bufferSignature, divisor)
	{
		layoutCount += 3;

		extendedData = data;
		bindBuffers();
	}

	template <class T, class S> void MatrixInstancedMeshObject<T, S>::commitVBOToGPU(void)
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, extendedData.size() * sizeof(T), &(extendedData[0]), GL_DYNAMIC_DRAW);

		auto glType = GL_FLOAT;

		if (std::is_same<S, GLdouble>::value || std::is_same<S, double>::value)
		{
			glType = GL_DOUBLE;
		}
		else if (std::is_same<S, GLbyte>::value || std::is_same<S, bool>::value)
		{
			glType = GL_BYTE;
		}

		for (int i = layoutCount - 4, j = 0; i < layoutCount; i++, j++)
		{
			glEnableVertexAttribArray(i);
			glVertexAttribPointer(i, sizeof(T) / 4 / sizeof(S), glType, GL_FALSE, sizeof(T), (GLvoid*)(sizeof(T) * j / 4));
			glVertexAttribDivisor(i, divisor);
		}

		glBindVertexArray(0);
	}
}
#pragma endregion