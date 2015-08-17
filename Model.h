#ifndef MODEL_H
#define MODEL_H

#define USE_ASSIMP 1

#include <vector>
#include <string>
#include <set>

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#include "glm/vec3.hpp"
#include "glm/vec2.hpp"
#if USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

//#include <io.h> // _access

#include "Debug.h"

using glm::vec3;
using glm::vec2;

typedef unsigned int UINT;

class Model
{
public:
	typedef std::vector<unsigned int> UIntArray;
	typedef std::vector<vec3> Vec3Array;
	typedef std::vector<vec2> Vec2Array;

public:

	Model() {}

	Model(const std::string& str_path, bool use_normal = true, bool use_uv = true, bool use_tangent = false)
		: _use_normal(use_normal), _use_uv(use_uv), _use_tangent(use_tangent)
	{
		_LoadMeshFromFile(str_path);
		_BindBuffer();
	}
    
    Model(GLuint vertex_count, const glm::vec3* vertex_buffer_array, GLuint index_count, const GLuint* index_buffer_array)
    : _vertices(vertex_buffer_array, vertex_buffer_array+vertex_count), _triangles(index_buffer_array, index_buffer_array+index_count),
    _use_uv(false), _use_normal(false), _use_tangent(false), _use_bitangent(false)
    {
    }

    static void triangle()
    {

    }
    
	~Model()
	{
		destory();
	}

	void init(const std::string& str_path, bool use_normal = true, bool use_uv = true, bool use_tangent = false, bool use_bitangent = false)
	{
		_use_normal  = use_normal;
		_use_uv		 = use_uv;
		_use_tangent = use_tangent;
		_use_bitangent = use_bitangent;
		_LoadMeshFromFile(str_path);
		_BindBuffer();
	}

	void destory()
	{
		glDeleteBuffers(1, &elementBuffer);
		glDeleteBuffers(1, &vertexBuffer);
		if (_use_normal)
			glDeleteBuffers(1, &normalBuffer);
		if (_use_uv)
			glDeleteBuffers(1, &uvBuffer);
		if (_use_tangent)
			glDeleteBuffers(1, &tangent_buffer);
		if (_use_bitangent)
			glDeleteBuffers(1, &bitangent_buffer);
		Debug::LogInfo("clean up model...");
	}

	void render()
	{
		GLuint attribute_index = 0;

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(attribute_index);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			attribute_index,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

		attribute_index++;

		// 2nd attribute buffer : UVs
		if (_use_normal)
		{
			glEnableVertexAttribArray(attribute_index);
			glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
			glVertexAttribPointer(
				attribute_index,                  // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			attribute_index++;
		}

		if (_use_tangent)
		{
			glEnableVertexAttribArray(attribute_index);
			glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
			glVertexAttribPointer(
				attribute_index,                  // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			attribute_index++;
		}

		if (_use_uv)
		{
			// 3rd attribute buffer : UVs
			glEnableVertexAttribArray(attribute_index);
			glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
			glVertexAttribPointer(
				attribute_index,                  // attribute
				2,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);
			attribute_index++;
		}

		if (_use_bitangent)
		{
			glEnableVertexAttribArray(attribute_index);
			glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
			glVertexAttribPointer(
				attribute_index,                  // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
				);

			attribute_index++;
		}


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

		// Draw the triangle !
		//glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		glDrawElements(
			GL_TRIANGLES,
			(GLsizei)_triangles.size(),
			GL_UNSIGNED_INT,
			(void*)0
			);

		glDisableVertexAttribArray(0);
		if (_use_normal)
			glDisableVertexAttribArray(1);
		if (_use_uv)
			glDisableVertexAttribArray(2);
	}

	UINT VertexCount() const
	{
		return (UINT)_vertices.size();
	}

	UINT IndexCount() const
	{
		return (UINT)_triangles.size();
	}

	const UIntArray& Triangles() const
	{
		return _triangles;
	}

	const Vec3Array& Vertices() const
	{
		return _vertices;
	}

	const Vec3Array& Normals() const
	{
		return _normals;
	}

	const Vec2Array& UV() const
	{
		return _uv;
	}

private:
    

	void _LoadMeshFromFile(const std::string& str_path)
	{
		_triangles.clear();
		_vertices.clear();
		_normals.clear();
		_uv.clear();
		_triangles.clear();
		_bitangent.clear();
#if USE_ASSIMP
//		if (_access(str_path.c_str(), 0) == -1)
//		{
//			Debug::LogError("Model file " + str_path + " not exists!");
//			return;
//		}

		Assimp::Importer importer;
		const char* path = str_path.c_str();
		unsigned int load_option = 
			//aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType;
		if (_use_normal) load_option |= aiProcess_GenSmoothNormals;
		if (_use_tangent || _use_bitangent) load_option |= aiProcess_CalcTangentSpace;
		const aiScene* scene = importer.ReadFile(path, load_option);

		if (!scene) {
			Debug::LogError("Can not open model " + str_path + ". This file may not exist or is not supported");
			return; // TODO
		}

		// get each mesh
		int nvertices = 0;
		int ntriangles = 0;

		for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[i];
			nvertices += mesh->mNumVertices;
			ntriangles += mesh->mNumFaces;
		}
		
		_vertices.reserve(nvertices);
		if (_use_normal) _normals.reserve(nvertices);
		if (_use_uv) _uv.reserve(nvertices);
		if (_use_tangent) _tangent.reserve(nvertices);
		if (_use_bitangent) _bitangent.reserve(nvertices);
		_triangles.resize(ntriangles * 3);	// TODO, *3?
		int idx = 0;
		int idx2 = 0;
		for (unsigned int i = 0; i < scene->mNumMeshes; i++) 
		{
			aiMesh* mesh = scene->mMeshes[i];
			if (_use_uv)
				assert(mesh->HasTextureCoords(0) == true);

			for (unsigned int j = 0; j < mesh->mNumVertices; j++) 
			{
				aiVector3D& v = mesh->mVertices[j];
				_vertices.push_back(vec3(v.x, v.y, v.z));
				
				if (_use_normal)
				{
					aiVector3D& n = mesh->mNormals[j];
					_normals.push_back(vec3(n.x, n.y, n.z));
				}
				
				if (_use_uv)
				{
					aiVector3D& u = mesh->mTextureCoords[0][j];
					_uv.push_back(vec2(u.x, u.y));
				}

				if (_use_tangent)
				{
					auto& t = mesh->mTangents[j];
					_tangent.push_back(vec3(t.x, t.y, t.z));
				}
				if (_use_bitangent)
				{
					auto& t = mesh->mBitangents[j];
					_bitangent.push_back(vec3(t.x, t.y, t.z));
				}
				
				//aabb.expand(glm::vec3(v.x, v.y, v.z));
			}

			//int temp_idx = idx/3;
			for (unsigned int j = 0; j < mesh->mNumFaces; j++) 
			{
				const aiFace& Face = mesh->mFaces[j];
				assert(Face.mNumIndices == 3);
				_triangles[idx++] = Face.mIndices[0] + idx2;
				_triangles[idx++] = Face.mIndices[1] + idx2;
				_triangles[idx++] = Face.mIndices[2] + idx2;
			}
			idx2 += mesh->mNumVertices;
		}
#else
        
#endif
	}


	void _BindBuffer()
	{
		GLuint VertexArrayID;
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		glGenBuffers(1, &elementBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _triangles.size() * sizeof(unsigned int), &_triangles[0], GL_STATIC_DRAW);

		// Load it into a VBO
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, _vertices.size() * sizeof(glm::vec3), &_vertices[0], GL_STATIC_DRAW);

		if (_use_normal)
		{
			glGenBuffers(1, &normalBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
			glBufferData(GL_ARRAY_BUFFER, _normals.size() * sizeof(glm::vec3), &_normals[0], GL_STATIC_DRAW);
		}

		if (_use_tangent)
		{
			glGenBuffers(1, &tangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, tangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, _tangent.size() * sizeof(glm::vec3), &_tangent[0], GL_STATIC_DRAW);
		}
		
		if (_use_uv)
		{
			glGenBuffers(1, &uvBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
			glBufferData(GL_ARRAY_BUFFER, _uv.size() * sizeof(glm::vec2), &_uv[0], GL_STATIC_DRAW);
		}
		
		if (_use_bitangent)
		{
			glGenBuffers(1, &bitangent_buffer);
			glBindBuffer(GL_ARRAY_BUFFER, bitangent_buffer);
			glBufferData(GL_ARRAY_BUFFER, _bitangent.size() * sizeof(glm::vec3), &_bitangent[0], GL_STATIC_DRAW);
		}
		
	}

public:
	GLuint elementBuffer;
	GLuint vertexBuffer;
	GLuint normalBuffer;
	GLuint tangent_buffer;
	GLuint bitangent_buffer;
	GLuint uvBuffer;

private:
	std::vector<GLuint> _triangles;
	std::vector<glm::vec3> _vertices;
	std::vector<glm::vec3> _normals;
	std::vector<glm::vec2> _uv;
	std::vector<glm::vec3> _tangent;
	std::vector<glm::vec3> _bitangent;

	bool _use_normal = true;
	bool _use_uv = true;
	bool _use_tangent = false;
	bool _use_bitangent = false;
};


class ModelManager
{
public:
	
	static void static_init()
	{
		screen_aligned_quad.init("media/quad.obj", false, false, false);
	}

    static Model triangle;
	static Model screen_aligned_quad;
	//static Model sphere;
	//static Model quad;
	//static Model sphere;

private:
	//static std::vector<Model*> _models_already_built;
	//static std::set<std::string> _models_path_already_built;

	ModelManager() {};
};

//_declspec(selectany) Model ModelManager::screen_aligned_quad;
//_declspec(selectany) Model ModelManager::sphere;
//_declspec(selectany) Model ModelManager::quad;
//_declspec(selectany) Model ModelManager::sphere;

#endif