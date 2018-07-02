#pragma once

//#include "Win32_GLAppUtil.h";
//#include "AvatarManager.h";
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <map>


using namespace OVR;

#ifndef VALIDATE
#define VALIDATE(x, msg) if (!(x)) { MessageBoxA(NULL, (msg), "OculusRoomTiny", MB_ICONERROR | MB_OK); exit(-1); }
#endif

#ifndef OVR_DEBUG_LOG
#define OVR_DEBUG_LOG(x)
#endif



//---------------------------------------------------------------------------------------
struct DepthBuffer
{
	GLuint        texId;

	DepthBuffer(Sizei size)
	{
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		GLenum internalFormat = GL_DEPTH_COMPONENT24;
		GLenum type = GL_UNSIGNED_INT;
		if (GLE_ARB_depth_buffer_float)
		{
			internalFormat = GL_DEPTH_COMPONENT32F;
			type = GL_FLOAT;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.w, size.h, 0, GL_DEPTH_COMPONENT, type, NULL);
	}
	~DepthBuffer()
	{
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
	}
};

//--------------------------------------------------------------------------
struct TextureBuffer
{
	GLuint              texId;
	GLuint              fboId;
	Sizei               texSize;

	TextureBuffer(bool rendertarget, Sizei size, int mipLevels, unsigned char * data) :
		texId(0),
		fboId(0),
		texSize(0, 0)
	{
		texSize = size;

		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);

		if (rendertarget)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, texSize.w, texSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		if (mipLevels > 1)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		if (rendertarget)
		{
			glGenFramebuffers(1, &fboId);
		}
	}

	~TextureBuffer()
	{
		if (texId)
		{
			glDeleteTextures(1, &texId);
			texId = 0;
		}
		if (fboId)
		{
			glDeleteFramebuffers(1, &fboId);
			fboId = 0;
		}
	}

	Sizei GetSize() const
	{
		return texSize;
	}

	void SetAndClearRenderSurface(DepthBuffer* dbuffer)
	{
		VALIDATE(fboId, "Texture wasn't created as a render target");

		GLuint curTexId = texId;

		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curTexId, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, dbuffer->texId, 0);

		glViewport(0, 0, texSize.w, texSize.h);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	void UnsetRenderSurface()
	{
		VALIDATE(fboId, "Texture wasn't created as a render target");

		glBindFramebuffer(GL_FRAMEBUFFER, fboId);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
	}
};

//------------------------------------------------------------------------------
struct ShaderFill
{
	GLuint            program;
	TextureBuffer   * texture;

	ShaderFill(GLuint vertexShader, GLuint pixelShader, TextureBuffer* _texture)
	{
		texture = _texture;

		program = glCreateProgram();

		glAttachShader(program, vertexShader);
		glAttachShader(program, pixelShader);

		glLinkProgram(program);

		glDetachShader(program, vertexShader);
		glDetachShader(program, pixelShader);

		GLint r;
		glGetProgramiv(program, GL_LINK_STATUS, &r);
		if (!r)
		{
			GLchar msg[1024];
			glGetProgramInfoLog(program, sizeof(msg), 0, msg);
			OVR_DEBUG_LOG(("Linking shaders failed: %s\n", msg));
		}
	}

	~ShaderFill()
	{
		if (program)
		{
			glDeleteProgram(program);
			program = 0;
		}
		if (texture)
		{
			delete texture;
			texture = nullptr;
		}
	}
};

//----------------------------------------------------------------
struct VertexBuffer
{
	GLuint    buffer;

	VertexBuffer(void* vertices, size_t size)
	{
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}
	~VertexBuffer()
	{
		if (buffer)
		{
			glDeleteBuffers(1, &buffer);
			buffer = 0;
		}
	}
};

//----------------------------------------------------------------
struct IndexBuffer
{
	GLuint    buffer;

	IndexBuffer(void* indices, size_t size)
	{
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
	}
	~IndexBuffer()
	{
		if (buffer)
		{
			glDeleteBuffers(1, &buffer);
			buffer = 0;
		}
	}
};


//-------------------------------------------------------
struct Model
{
	struct Vertex
	{
		Vector3f  Pos;
		DWORD     C;
		float     U, V;
	};

	Vector3f        Pos;
	Quatf           Rot;
	Matrix4f        Mat;
	int             numVertices, numIndices;
	Vertex          Vertices[20000]; // Note fixed maximum
	GLushort        Indices[40000];
	ShaderFill    * Fill;
	VertexBuffer  * vertexBuffer;
	IndexBuffer   * indexBuffer;

	Model(Vector3f pos, ShaderFill * fill) :
		numVertices(0),
		numIndices(0),
		Pos(pos),
		Rot(),
		Mat(),
		Fill(fill),
		vertexBuffer(nullptr),
		indexBuffer(nullptr)
	{}

	~Model()
	{
		FreeBuffers();
	}

	Matrix4f& GetMatrix()
	{
		Mat = Matrix4f(Rot);
		Mat = Matrix4f::Translation(Pos) * Mat;
		return Mat;
	}

	void AddVertex(const Vertex& v) { Vertices[numVertices++] = v; }
	void AddIndex(GLushort a) { Indices[numIndices++] = a; }

	void AllocateBuffers()
	{
		vertexBuffer = new VertexBuffer(&Vertices[0], numVertices * sizeof(Vertices[0]));
		indexBuffer = new IndexBuffer(&Indices[0], numIndices * sizeof(Indices[0]));
	}

	void FreeBuffers()
	{
		delete vertexBuffer; vertexBuffer = nullptr;
		delete indexBuffer; indexBuffer = nullptr;
	}

	void AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, DWORD c)
	{
		Vector3f Vert[][2] =
		{
			Vector3f(x1, y2, z1), Vector3f(z1, x1), Vector3f(x2, y2, z1), Vector3f(z1, x2),
			Vector3f(x2, y2, z2), Vector3f(z2, x2), Vector3f(x1, y2, z2), Vector3f(z2, x1),
			Vector3f(x1, y1, z1), Vector3f(z1, x1), Vector3f(x2, y1, z1), Vector3f(z1, x2),
			Vector3f(x2, y1, z2), Vector3f(z2, x2), Vector3f(x1, y1, z2), Vector3f(z2, x1),
			Vector3f(x1, y1, z2), Vector3f(z2, y1), Vector3f(x1, y1, z1), Vector3f(z1, y1),
			Vector3f(x1, y2, z1), Vector3f(z1, y2), Vector3f(x1, y2, z2), Vector3f(z2, y2),
			Vector3f(x2, y1, z2), Vector3f(z2, y1), Vector3f(x2, y1, z1), Vector3f(z1, y1),
			Vector3f(x2, y2, z1), Vector3f(z1, y2), Vector3f(x2, y2, z2), Vector3f(z2, y2),
			Vector3f(x1, y1, z1), Vector3f(x1, y1), Vector3f(x2, y1, z1), Vector3f(x2, y1),
			Vector3f(x2, y2, z1), Vector3f(x2, y2), Vector3f(x1, y2, z1), Vector3f(x1, y2),
			Vector3f(x1, y1, z2), Vector3f(x1, y1), Vector3f(x2, y1, z2), Vector3f(x2, y1),
			Vector3f(x2, y2, z2), Vector3f(x2, y2), Vector3f(x1, y2, z2), Vector3f(x1, y2)
		};

		GLushort CubeIndices[] =
		{
			0, 1, 3, 3, 1, 2,
			5, 4, 6, 6, 4, 7,
			8, 9, 11, 11, 9, 10,
			13, 12, 14, 14, 12, 15,
			16, 17, 19, 19, 17, 18,
			21, 20, 22, 22, 20, 23
		};

		for (int i = 0; i < sizeof(CubeIndices) / sizeof(CubeIndices[0]); ++i)
			AddIndex(CubeIndices[i] + GLushort(numVertices));

		// Generate a quad for each box face
		for (int v = 0; v < 6 * 4; v++)
		{
			// Make vertices, with some token lighting
			Vertex vvv; vvv.Pos = Vert[v][0]; vvv.U = Vert[v][1].x; vvv.V = Vert[v][1].y;
			float dist1 = (vvv.Pos - Vector3f(-2, 4, -2)).Length();
			float dist2 = (vvv.Pos - Vector3f(3, 4, -3)).Length();
			float dist3 = (vvv.Pos - Vector3f(-4, 3, 25)).Length();
			int   bri = rand() % 160;
			float B = ((c >> 16) & 0xff) * (bri + 192.0f * (0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
			float G = ((c >> 8) & 0xff) * (bri + 192.0f * (0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
			float R = ((c >> 0) & 0xff) * (bri + 192.0f * (0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
			vvv.C = (c & 0xff000000) +
				((R > 255 ? 255 : DWORD(R)) << 16) +
				((G > 255 ? 255 : DWORD(G)) << 8) +
				(B > 255 ? 255 : DWORD(B));
			AddVertex(vvv);
		}
	}

	//returns the vectors added to a center point representing the vertices on the edges
	std::vector<Vector3f> AddedVectors(Vector3f prev, Vector3f next, glm::quat handQ) {
		std::vector<Vector3f> added;
		
		//Unit vector representing the pointing orientation -> v
		glm::vec4 u(0, 0, 1.0f, 1.0f);
		glm::vec4 translate = handQ * u;
		Vector3f v;
		v.x = translate.x;
		v.y = translate.y;
		v.z = translate.z;

		//generate directional unit vector of the lineCore (segment we care about right now)
		//Vector3f lineDir = next - prev;
		Vector3f lineDir = (next - prev)/(next-prev).Length();
		//v_tan is the component of v which is tangent to lineDir
		Vector3f v_tan = lineDir * (v.Dot(lineDir));

		//v_norm is the component of v which is normal to lineDir
		Vector3f v_norm = v - v_tan;
		//unit normal
		Vector3f v_norm_unit = v_norm / (v_norm.Length());
		

		added.push_back(v_norm_unit);
		added.push_back(v_norm_unit.Cross(lineDir));
		added.push_back(v_norm_unit*(-1));
		added.push_back(v_norm_unit.Cross(lineDir)*(-1));

		return added;
	}
	

	//Generate the 4 quads (8 triangles) making up the current segment of curved line; called in AddCurvedLine
	//Also updates the edge vectors
	void AddLineSegment(Vector3f prev, Vector3f next, glm::quat handQ, float thickness, DWORD c,
						std::vector<Vector3f> &edge1, std::vector<Vector3f> &edge2, std::vector<Vector3f> &edge3, std::vector<Vector3f> &edge4) {
		
		std::vector<Vector3f> added = AddedVectors(prev, next, handQ);

		Vector3f midpoint = (prev + next) / 2;
		
		//initialize the edge vectors with an initial value; Vert requires 2 elements in each edge vector, but the first midpoint only generates 1
		if (edge1.size() == 0 && edge2.size() == 0 && edge3.size() == 0 && edge4.size() == 0) {
			edge1.push_back(added[0] * thickness + prev);
			edge2.push_back(added[1] * thickness + prev);
			edge3.push_back(added[2] * thickness + prev);
			edge4.push_back(added[3] * thickness + prev);
		}

		edge1.push_back(added[0]*thickness + midpoint);
		edge2.push_back(added[1]*thickness + midpoint);
		edge3.push_back(added[2]*thickness + midpoint);
		edge4.push_back(added[3]*thickness + midpoint);
		
		Vector3f Vert[][2] =
		{
			edge1[edge1.size() - 2], Vector3f(0,1), edge1[edge1.size() - 1], Vector3f(1,1),
			edge2[edge2.size() - 1], Vector3f(1,0), edge2[edge2.size() - 2], Vector3f(0,0),

			edge4[edge4.size() - 2], Vector3f(0,1), edge4[edge4.size() - 1], Vector3f(1,1),
			edge3[edge3.size() - 1], Vector3f(1,0), edge3[edge3.size() - 2], Vector3f(0,0),

			edge1[edge1.size() - 2], Vector3f(0,1), edge1[edge4.size() - 1], Vector3f(1,1),
			edge4[edge4.size() - 1], Vector3f(1,0), edge4[edge4.size() - 2], Vector3f(0,0),

			edge2[edge2.size() - 2], Vector3f(0,1), edge2[edge2.size() - 1], Vector3f(1,1),
			edge3[edge3.size() - 1], Vector3f(1,0), edge3[edge3.size() - 2], Vector3f(0,0),
		};

		GLushort LinSegIndices[] =
		{
			3, 2, 0, 0, 2, 1,
			6, 7, 5, 5, 7, 4,
			8, 9, 11, 11, 9, 10,
			13, 12, 14, 14, 12, 15
		};

		for (int i = 0; i < sizeof(LinSegIndices) / sizeof(LinSegIndices[0]); ++i)
			AddIndex(LinSegIndices[i] + GLushort(numVertices));

		// Generate a quad of vertices for each line segment
		genQuadVertices(Vert, 4, c);

	}

	void AddEndCaps(std::vector<Vector3f> edge1, std::vector<Vector3f> edge2, std::vector<Vector3f> edge3, std::vector<Vector3f> edge4, DWORD c) {
		Vector3f Vert[][2] =
		{
			//beginning cap; textures are default right now
			edge1[0], Vector3f(0,1), edge2[0], Vector3f(1,1),
			edge3[0], Vector3f(1,0), edge4[0], Vector3f(0,0),

			//end cap; textures are default right now
			edge1[edge1.size()-1], Vector3f(0,1), edge2[edge2.size()-1], Vector3f(1,1),
			edge3[edge3.size()-1], Vector3f(1,0), edge4[edge4.size()-1], Vector3f(0,0),
		};

		GLushort CapIndices[] =
		{
			//0, 1, 3, 3, 1, 2,
			//5, 4, 6, 6, 4, 7,
			0, 3, 1, 1, 3, 2,
			5, 6, 4, 4, 6, 7
		};

		for (int i = 0; i < sizeof(CapIndices) / sizeof(CapIndices[0]); ++i)
			AddIndex(CapIndices[i] + GLushort(numVertices));

		// Generate a quad of vertices for each cap
		genQuadVertices(Vert, 2, c);
	}


	void AddCurvedLine(std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, float thickness, DWORD c) {
		std::vector<Vector3f> edge1;
		std::vector<Vector3f> edge2;
		std::vector<Vector3f> edge3;
		std::vector<Vector3f> edge4;

		for (int i = 0; i < (lineCore.size() - 1); i++) {
			Vector3f prev = lineCore[i];
			Vector3f next = lineCore[i + 1];
			glm::quat handQ = allHandQ[i];

			AddLineSegment(prev, next, handQ, thickness, c, edge1, edge2, edge3, edge4);
		}

		AddEndCaps(edge1, edge2, edge3, edge4, c);
	}


	void AddStraightLine(Vector3f start, Vector3f end, glm::quat handQ, float thickness, DWORD c) {
		std::vector<Vector3f> added = AddedVectors(start, end, handQ);

		std::vector<Vector3f> edge1{ (start + added[0] * thickness), (end + added[0] * thickness) };
		std::vector<Vector3f> edge2{ (start + added[1] * thickness), (end + added[1] * thickness) };
		std::vector<Vector3f> edge3{ (start + added[2] * thickness), (end + added[2] * thickness) };
		std::vector<Vector3f> edge4{ (start + added[3] * thickness), (end + added[3] * thickness) };

		AddEndCaps(edge1, edge2, edge3, edge4, c);


		Vector3f Vert[][2] =
		{
			edge1[0], Vector3f(0,1), edge1[1], Vector3f(1,1),
			edge2[1], Vector3f(1,0), edge2[0], Vector3f(0,0),

			edge4[0], Vector3f(0,1), edge4[1], Vector3f(1,1),
			edge3[1], Vector3f(1,0), edge3[0], Vector3f(0,0),

			edge1[0], Vector3f(0,1), edge1[1], Vector3f(1,1),
			edge4[1], Vector3f(1,0), edge4[0], Vector3f(0,0),

			edge2[0], Vector3f(0,1), edge2[1], Vector3f(1,1),
			edge3[1], Vector3f(1,0), edge3[0], Vector3f(0,0),
		};

		GLushort LinSegIndices[] =
		{
			3, 2, 0, 0, 2, 1,
			6, 7, 5, 5, 7, 4,
			8, 9, 11, 11, 9, 10,
			13, 12, 14, 14, 12, 15
		};

		for (int i = 0; i < sizeof(LinSegIndices) / sizeof(LinSegIndices[0]); ++i)
			AddIndex(LinSegIndices[i] + GLushort(numVertices));

		// Generate a quad of vertices for the straight line
		genQuadVertices(Vert, 4, c);
	}


	void genQuadVertices(Vector3f Vert[][2], int numQuads, DWORD c) {
		for (int v = 0; v < numQuads * 4; v++)
		{
			// Make vertices, with some token lighting
			Vertex vvv; vvv.Pos = Vert[v][0]; vvv.U = Vert[v][1].x; vvv.V = Vert[v][1].y;
			//not sure what the subtracted Vector3fs are supposed to do
			float dist1 = (vvv.Pos - Vector3f(-2, 4, -2)).Length();
			float dist2 = (vvv.Pos - Vector3f(3, 4, -3)).Length();
			float dist3 = (vvv.Pos - Vector3f(-4, 3, 25)).Length();
			int   bri = rand() % 160;
			float B = ((c >> 16) & 0xff) * (bri + 192.0f * (0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
			float G = ((c >> 8) & 0xff) * (bri + 192.0f * (0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
			float R = ((c >> 0) & 0xff) * (bri + 192.0f * (0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
			vvv.C = (c & 0xff000000) +
				((R > 255 ? 255 : DWORD(R)) << 16) +
				((G > 255 ? 255 : DWORD(G)) << 8) +
				(B > 255 ? 255 : DWORD(B));
			AddVertex(vvv);
		}
	}



	void Render(Matrix4f view, Matrix4f proj)
	{
		Matrix4f combined = proj * view * GetMatrix();

		glUseProgram(Fill->program);
		glUniform1i(glGetUniformLocation(Fill->program, "Texture0"), 0);
		glUniformMatrix4fv(glGetUniformLocation(Fill->program, "matWVP"), 1, GL_TRUE, (FLOAT*)&combined);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Fill->texture->texId);

		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->buffer);

		GLuint posLoc = glGetAttribLocation(Fill->program, "Position");
		GLuint colorLoc = glGetAttribLocation(Fill->program, "Color");
		GLuint uvLoc = glGetAttribLocation(Fill->program, "TexCoord");

		glEnableVertexAttribArray(posLoc);
		glEnableVertexAttribArray(colorLoc);
		glEnableVertexAttribArray(uvLoc);

		glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OVR_OFFSETOF(Vertex, Pos));
		glVertexAttribPointer(colorLoc, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)OVR_OFFSETOF(Vertex, C));
		glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)OVR_OFFSETOF(Vertex, U));

		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, NULL);

		glDisableVertexAttribArray(posLoc);
		glDisableVertexAttribArray(colorLoc);
		glDisableVertexAttribArray(uvLoc);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glUseProgram(0);
	}
};



struct Scenes
{
	std::vector<Vector3f> Core;
};



//------------------------------------------------------------------------- 
struct Scene
{
	struct LineComponents
	{
		std::vector<Vector3f> Core;
		std::vector<glm::quat> Q;
	};

	std::vector<Model*> Models;
	//use map for built-in find function
	std::map<Model*, int> removableModels;
	//markers have meaningless int values
	std::map<Model*, int> removableMarkers;
	//straight lines are represented by their vector core [start,end]
	std::map<Model*, LineComponents> removableStraightLines;
	//curved lines are represented by the points in their lineCore
	std::map<Model*, LineComponents> removableCurvedLines;

	std::map<Model*, int> tempModels;
	std::map<Model*, int> tempLines;

	float MARKER_SIZE = 0.02f;
	float TARGET_SIZE = 0.01f;
	float LINE_THICKNESS = 0.01f;

	ShaderFill * grid_material[4];
	//model highlighted for potential removal
	Model *targetModel;
	std::string targetModelType;

	//for line drawing functionality
	std::vector<Vector3f> lineCore;
	std::vector<glm::quat> allHandQ;


	/************************************
	Add functions
	*************************************/
	void    Add(Model * n)
	{
		Models.push_back(n);
	}

	void AddRemovable(Model * n)
	{
		//don't actually care about the values
		removableModels.insert(std::pair<Model*, int>(n, 1));
	}

	void AddTemp(Model * n)
	{
		RemoveModel(n);
		//removableMarkers.erase(n);
		tempModels.insert(std::pair<Model*, int>(n, 1));
	}

	void AddTempLine(Model * n)
	{
		tempLines.insert(std::pair<Model *, int>(n, 1));
	}

	void AddRemovableMarker(Model * n)
	{
		removableMarkers.insert(std::pair<Model*, int>(n, 1));
		AddRemovable(n);
	}

	void AddRemovableStraightLine(Model * n, Vector3f start, Vector3f end, glm::quat handQ)
	{
		std::vector<Vector3f> core{ start,end };
		LineComponents line;
		line.Core = core;
		std::vector<glm::quat> quat{ handQ };
		line.Q = quat;
		removableStraightLines.insert(std::pair<Model*, LineComponents>(n, line));

		AddRemovable(n);
	}
	
	void AddRemovableCurvedLine(Model * n, std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ)
	{
		LineComponents line;
		line.Core = lineCore;
		line.Q = allHandQ;
		removableCurvedLines.insert(std::pair<Model*, LineComponents>(n, line));

		AddRemovable(n);
	}


	//calling this removes non-temp models from all "removable" maps
	void RemoveModel(Model * n)
	{
		removableModels.erase(n);
		removableMarkers.erase(n);
		removableStraightLines.erase(n);
		removableCurvedLines.erase(n);
	}




	void Render(Matrix4f view, Matrix4f proj)
	{
		/*
		for (int i = 0; i < Models.size(); ++i)
			Models[i]->Render(view, proj);
		*/

		//render removableModels as well
		for (auto const m : removableModels) {
			//auto model = *m.first;
			m.first->Render(view, proj);
		}

		//render tempModels as well
		for (auto const m : tempModels) {
			m.first->Render(view, proj);
		}

		//render tempLines as well
		for (auto const m : tempLines) {
			m.first->Render(view, proj);
		}

	}

	//transforms the controller position to be right on top of the user's controller (not far in front)
	Vector3f VirtualPosFromReal(Vector3f pos)
	{
		Vector3f newPos;
		newPos.x = -1 * pos.x;
		newPos.y = pos.y;
		newPos.z = -1 * pos.z - 5.0f;

		return newPos;
	}

	//rotates the marker pos to accomodate for controller orientation and place it in front of controller
	Vector3f MarkerTranslateToPointer(Vector3f handPos, glm::quat handQuat)
	{
		Vector3f newMarkerPos;

		glm::vec4 u(0, 0, 0.1f, 1.0f);
		glm::vec4 translate = handQuat * u;
		newMarkerPos.x = handPos.x + translate.x;
		newMarkerPos.y = handPos.y - translate.y;
		newMarkerPos.z = handPos.z + translate.z;

		return newMarkerPos;
	}


	Model * CreateMarker(float size, DWORD color, Vector3f pos)
	{
		//ShaderFill * grid_material = CreateTextures();

		Model * marker = new Model(Vector3f(0, 0, 0), (grid_material[3]));
		marker->AddSolidColorBox(-0.5f*size, -0.5f*size, -0.5f*size, 0.5f*size, 0.5f*size, 0.5f*size, color);
		marker->AllocateBuffers();
		marker->Pos = pos;
		AddRemovableMarker(marker);

		return marker;
	}


	Model * CreateStraightLine(Vector3f start, Vector3f end, glm::quat handQ, float thickness, DWORD color) {
		//should be okay to put Pos at origin because the line construction is based on the origin
		//However, Pos will NOT reflect the actual location of the line!
		Model * straightLine = new Model(Vector3f(0, 0, 0), (grid_material[3]));
		straightLine->AddStraightLine(start, end, handQ, thickness, color);
		straightLine->AllocateBuffers();

		return straightLine;
	}


	Model * CreateCurvedLine(std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, float thickness, DWORD color) {
		//should be okay to put Pos at origin because the line construction is based on the origin
		//However, Pos will NOT reflect the actual location of the line!
		Model * curvedLine = new Model(Vector3f(0, 0, 0), (grid_material[3]));
		curvedLine->AddCurvedLine(lineCore, allHandQ, thickness, color);
		curvedLine->AllocateBuffers();

		return curvedLine;
	}



	GLuint CreateShader(GLenum type, const GLchar* src)
	{
		GLuint shader = glCreateShader(type);

		glShaderSource(shader, 1, &src, NULL);
		glCompileShader(shader);

		GLint r;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &r);
		if (!r)
		{
			GLchar msg[1024];
			glGetShaderInfoLog(shader, sizeof(msg), 0, msg);
			if (msg[0]) {
				OVR_DEBUG_LOG(("Compiling shader failed: %s\n", msg));
			}
			return 0;
		}

		return shader;
	}

	
	void CreateTextures()
	{
		static const GLchar* VertexShaderSrc =
			"#version 150\n"
			"uniform mat4 matWVP;\n"
			"in      vec4 Position;\n"
			"in      vec4 Color;\n"
			"in      vec2 TexCoord;\n"
			"out     vec2 oTexCoord;\n"
			"out     vec4 oColor;\n"
			"void main()\n"
			"{\n"
			"   gl_Position = (matWVP * Position);\n"
			"   oTexCoord   = TexCoord;\n"
			"   oColor.rgb  = pow(Color.rgb, vec3(2.2));\n"   // convert from sRGB to linear
			"   oColor.a    = Color.a;\n"
			"}\n";

		static const char* FragmentShaderSrc =
			"#version 150\n"
			"uniform sampler2D Texture0;\n"
			"in      vec4      oColor;\n"
			"in      vec2      oTexCoord;\n"
			"out     vec4      FragColor;\n"
			"void main()\n"
			"{\n"
			"   FragColor = oColor * texture2D(Texture0, oTexCoord);\n"
			"}\n";

		GLuint    vshader = CreateShader(GL_VERTEX_SHADER, VertexShaderSrc);
		GLuint    fshader = CreateShader(GL_FRAGMENT_SHADER, FragmentShaderSrc);
		
		//ShaderFill * grid_material[4];

		// Make textures
		for (int k = 0; k < 4; ++k)
		{
			static DWORD tex_pixels[256 * 256];
			for (int j = 0; j < 256; ++j)
			{
				for (int i = 0; i < 256; ++i)
				{
					if (k == 0) tex_pixels[j * 256 + i] = (((i >> 7) ^ (j >> 7)) & 1) ? 0xffb4b4b4 : 0xff505050;// floor
					if (k == 1) tex_pixels[j * 256 + i] = (((j / 4 & 15) == 0) || (((i / 4 & 15) == 0) && ((((i / 4 & 31) == 0) ^ ((j / 4 >> 4) & 1)) == 0)))
						? 0xff3c3c3c : 0xffb4b4b4;// wall
					if (k == 2) tex_pixels[j * 256 + i] = (i / 4 == 0 || j / 4 == 0) ? 0xff505050 : 0xffb4b4b4;// ceiling
					if (k == 3) tex_pixels[j * 256 + i] = 0xffffffff;// blank
				}
			}
			TextureBuffer * generated_texture = new TextureBuffer(false, Sizei(256, 256), 4, (unsigned char *)tex_pixels);
			grid_material[k] = new ShaderFill(vshader, fshader, generated_texture);
		}

		glDeleteShader(vshader);
		glDeleteShader(fshader);
	}
	
	/*
	float DistPointToLineSeg(Vector3f point, std::vector<Vector3f> lineSeg) {
		//directional unit vector of line seg
		Vector3f d = (lineSeg[1]-lineSeg[0] / ((lineSeg[1]-lineSeg[0]).Length()));
		Vector3f v = point - lineSeg[0];
		float t = v.Dot(d);
		float dist = (lineSeg[0] + d*t - point).Length();

		return dist;
	}

	*/

	float DistPointToLineSeg(Vector3f point, std::vector<Vector3f> lineSeg) {
		//vector from start of line seg to point
		Vector3f v = point - lineSeg[0];
		float lineSegLen = ((lineSeg[1] - lineSeg[0]).Length());
		//line seg unit vector
		Vector3f d = (lineSeg[1] - lineSeg[0]) / lineSegLen;
		//Intersection point distance from start of lineseg
		float t = d.Dot(v);
		
		if (t >= 0 && t <= lineSegLen) {
			Vector3f shortestPath = d * t - v; 
			return shortestPath.Length();
		}
		else {
			//returns a very big number; cannot possibly be under the target size
			return 100.0f;
		}
	}

	Model * ColorRemovableModel(Vector3f rightHandPos)
	{
		//make sure there is currently no targetModel when ColorRemovableModel is called
		if (!targetModel) {
			for (auto const m : removableMarkers) {
				Model *model = m.first;
				Vector3f modelPos = (*model).Pos;
				//Vector3f modelPos(0, 0, 0);
				if (rightHandPos.Distance(modelPos) <= TARGET_SIZE) {
					//dark red: 	0xFF800000
					Model *newMarker = CreateMarker(MARKER_SIZE, 0xFF811111, modelPos);
					//removing the old green model; replaced by new red one
					RemoveModel(model);
					//removableMarkers.erase(model);

					targetModel = newMarker;
					targetModelType = "marker";

					return newMarker;
				}
			}

			for (auto const m : removableStraightLines) {
				Model *model = m.first;
				float dist = DistPointToLineSeg(rightHandPos, (m.second).Core);
				if (dist <= TARGET_SIZE) {
					//dark red: 	0xFF800000
					Model *newStraightLine = CreateStraightLine((m.second).Core[0], (m.second).Core[1],
						(m.second).Q[0], LINE_THICKNESS, 0xFF800000);
					//adding the new model to appropriate maps
					AddRemovableStraightLine(newStraightLine, (m.second).Core[0], (m.second).Core[1], (m.second).Q[0]);
					//removing the old model from all maps
					RemoveModel(model);
					targetModel = newStraightLine;
					targetModelType = "straight line";

					return newStraightLine;
				}
			}

			for (auto const m : removableCurvedLines) {
				Model *model = m.first;
				//collision detection with line segments
				std::vector<Vector3f> core = (m.second).Core;
				for (int i = 0; i < (core.size()-1); i++) {
					std::vector<Vector3f> v_seg{ core[i],core[i + 1] };
					if (DistPointToLineSeg(rightHandPos, v_seg) <= TARGET_SIZE) {
						//dark red: 	0xFF800000
						Model *newCurvedLine = CreateCurvedLine((m.second).Core, (m.second).Q, LINE_THICKNESS, 0xFF800000);
						//adding the new model to appropriate maps
						AddRemovableCurvedLine(newCurvedLine, (m.second).Core, (m.second).Q);
						//removing the old model from all maps
						RemoveModel(model);
						targetModel = newCurvedLine;
						targetModelType = "curved line";

						return newCurvedLine;
					}
				}
			}
			return nullptr;
		}
		return nullptr;
	}

	void ResetTargetModel()
	{
		//pure green: 0xff008000
		if (targetModelType == "marker") {
			Vector3f targetPos = targetModel->Pos;
			Model *newMarker = CreateMarker(MARKER_SIZE, 0xff008000, targetPos);
		}

		else if (targetModelType == "straight line") {
			std::vector<Vector3f> core = (removableStraightLines.find(targetModel)->second).Core;
			std::vector<glm::quat> handQ = (removableStraightLines.find(targetModel)->second).Q;
			Model *newStraightLine = CreateStraightLine(core[0], core[1], handQ[0], LINE_THICKNESS, 0xff008000);
			AddRemovableStraightLine(newStraightLine, core[0], core[1], handQ[0]);
		}

		else if (targetModelType == "curved line") {
			std::vector<Vector3f> core = (removableCurvedLines.find(targetModel)->second).Core;
			std::vector<glm::quat> handQ = (removableCurvedLines.find(targetModel)->second).Q;
			Model *newCurvedLine = CreateCurvedLine(core, handQ, LINE_THICKNESS, 0xff008000);
			AddRemovableCurvedLine(newCurvedLine, core, handQ);
		}
		
		//removing the red target model from all maps; has been replaced already with a green model
		RemoveModel(targetModel);
		//removableMarkers.erase(targetModel);
		targetModel = nullptr;
	}


	//checks to see if the targetmodel can be cleared so a new model is allowed to be the targetmodel
	void CheckTargetModel(Vector3f rightHandPos)
	{
		//make sure targetModel is not nullptr
		if (targetModel) {
			if (targetModelType == "marker") {
				Vector3f targetPos = targetModel->Pos;
				if (rightHandPos.Distance(targetPos) > TARGET_SIZE) {
					ResetTargetModel();
				}
			}

			else if (targetModelType == "straight line") {
				std::vector<Vector3f> core = (removableStraightLines.find(targetModel)->second).Core;
				float dist = DistPointToLineSeg(rightHandPos, core);
				if (dist > TARGET_SIZE) {
					ResetTargetModel();
				}
			}

			else if (targetModelType == "curved line") {
				bool dist_far = true;
				std::vector<Vector3f> core = (removableCurvedLines.find(targetModel)->second).Core;
				for (int i = 0; i < (core.size() - 1); i++) {
					std::vector<Vector3f> v_seg{ core[i],core[i + 1] };
					if (DistPointToLineSeg(rightHandPos, v_seg) <= TARGET_SIZE) {
						dist_far = false;
					}
				}

				if (dist_far) {
					ResetTargetModel();
				}
			}
		}
	}

	// math helpers
	static glm::quat _glmFromOvrQuat(const ovrQuatf& ovrQuat)
	{
		return glm::quat(ovrQuat.w, ovrQuat.x, ovrQuat.y, ovrQuat.z);
	}

	
	void ControllerActions(ovrPosef leftPose, ovrPosef rightPose, Quatf &gPose, Vector3f &gHeadPos, 
		ovrInputState &inputState, Matrix4f &gHeadOrientation, OVR::Matrix4f& view, bool worldMode)
	{
		Vector3f ovr_rightP;
		glm::quat rightQ;

		//pose and orientation variables for world mode
		if (worldMode) {
			Vector3f pos = Vector3f(rightPose.Position);
			pos = gHeadOrientation.Inverted().Transform(pos - gHeadPos);
			ovr_rightP = view.Inverted().Transform(pos);

			rightQ = _glmFromOvrQuat(rightPose.Orientation);
		}

		//pose and orientation variables for volume mode
		else {
			Vector3f pos = Vector3f(rightPose.Position);
			pos = gHeadOrientation.Inverted().Transform(pos - gHeadPos);
			OVR::Matrix4f rot(gPose);
			ovr_rightP = rot.Inverted().Transform(view.Inverted().Transform(pos));

			//not sure if this is correct; might have to do some transforms with the volume rot
			rightQ = _glmFromOvrQuat(rightPose.Orientation);
		}

		Vector3f trans_rightP = MarkerTranslateToPointer(ovr_rightP, rightQ);

		//should not be allowed to simply hold button A and continuously make a stream of models; let go and press again
		static bool canCreateMarker = true;
		static bool drawingStraightLine = false;
		static bool drawingCurvedLine = false;


		//if we are actively drawing a line
		if (lineCore.size() > 0) {
			//if we are drawing a straight line, create a new phantom line
			if (drawingStraightLine) {
				//pure green: 0xff008000
				Model *newStraightLine = CreateStraightLine(lineCore[0], trans_rightP, rightQ, LINE_THICKNESS, 0xff008000);
				//if B let go (ending the line), just create the line and reset drawingStraightLine and lineCore
				if (!(inputState.Buttons & ovrButton_B)) {
					AddRemovableStraightLine(newStraightLine, lineCore[0], trans_rightP, rightQ);
					drawingStraightLine = false;
					lineCore.clear();
				}
				//dynamic lines must be regenerated at each timestamp
				else {
					AddTempLine(newStraightLine);
				}				
			}
			
			//if we are drawing a curved line
			else if (drawingCurvedLine) {

				lineCore.push_back(trans_rightP);
				allHandQ.push_back(rightQ);

				int numVerticesNext = 8 + (16 * (lineCore.size() + 1));

				//pure green: 	0xff008000
				Model *newCurvedLine = CreateCurvedLine(lineCore, allHandQ, LINE_THICKNESS, 0xff008000);
				
				//if we stop drawing the curved line (PRESSING index), put the line in removableModels and reset
				//if the next lineCore addition will exceed the vertex limit (currently 20000), auto-stop drawing
				if (inputState.IndexTrigger[ovrHand_Right] < 0.2f || numVerticesNext >= 20000) {
					drawingCurvedLine = false;

					AddRemovableCurvedLine(newCurvedLine, lineCore, allHandQ);

					lineCore.clear();
					allHandQ.clear();
				}

				//dynamic lines must be regenerated at every timestamp
				else {
					AddTempLine(newCurvedLine);
				}
			}
			
		}
		
		else {
			//press B to start drawing a straight line
			if (inputState.Buttons & ovrButton_B) {
				drawingStraightLine = true;
				lineCore.push_back(trans_rightP);
			}

			//press and hold index to draw a curved line
			if (inputState.IndexTrigger[ovrHand_Right] >= 0.2f) {
				drawingCurvedLine = true;
				lineCore.push_back(trans_rightP);
			}

			//replaced all ovrAvatarButtonTwo with touchMask -> ovrAvatarTouch_Index
			//stop showing the phantom marker if not pressing A
			if (!(inputState.Buttons & ovrButton_A)) {
				removeTempModels();
			}
			//allow user to create a new marker if they have stopped pressing A
			//Switched A to X; A+X = create marker
			if (!(inputState.Buttons & ovrButton_X) && !canCreateMarker) {
				canCreateMarker = true;
			}
			//clear the target model if the user stops pressing A
			if (!(inputState.Buttons & ovrButton_A) && targetModel) {
				ResetTargetModel();
			}
			//create a new marker if the user is pressing X and the user is allowed to
			if (inputState.Buttons & ovrButton_X && canCreateMarker) {
				//pure green: 	0xff008000
				//yellow (for testing): 0xFFF6FF00
				//Vector3f * temp;
				//*temp = ovr_rightP;
				CreateMarker(MARKER_SIZE, 0xff008000, trans_rightP);
				canCreateMarker = false;
			}
			//if user is pressing A
			else if (inputState.Buttons & ovrButton_A) {
				//purple: 0xFFA535F0
				Model *newMarker = CreateMarker(MARKER_SIZE, 0xFFA535F0, trans_rightP);
				
				AddTemp(newMarker);
				//we only want the temp marker to be in the tempModels map
				//RemoveModel(newMarker);
				//try to find a targetModel if there is not one currently
				if (!targetModel) {
					ColorRemovableModel(trans_rightP);
				}

				else {
					//delete targetModel if pressing Y
					if (inputState.Buttons & ovrButton_Y) {
						RemoveModel(targetModel);
						//removableMarkers.erase(targetModel);
						//clear targetModel because the model in question has been removed
						targetModel = nullptr;
					}
					//try to clear targetModel if there is one currently
					else {
						CheckTargetModel(trans_rightP);
					}
				}
			}
		}


	}
	
	
	/*
	//move all temp models to the current hand position
	void moveTempModels(ovrSession session) 
	{
		ovrInputState touchState;
		ovr_GetInputState(session, ovrControllerType_Active, &touchState);
		ovrTrackingState trackingState = ovr_GetTrackingState(session, 0.0, false);
		Vector3f rightP = VirtualPosFromReal(trackingState.HandPoses[ovrHand_Right].ThePose.Position);
		glm::quat rightQ = _glmFromOvrQuat(trackingState.HandPoses[ovrHand_Right].ThePose.Orientation);

		Vector3f ovr_rightP = MarkerTranslateToPointer(rightP,rightQ);

		for (auto const &m : tempModels) {
			auto model = m.first;
			model->Pos = ovr_rightP;
		}

		removeTempLines();
	}
	*/

	//move all temp markers to the current hand position, all temp lines removed
	//always in world mode
	void moveTempModels(ovrPosef rightPose, Vector3f &gHeadPos, Matrix4f &gHeadOrientation, Matrix4f& view)
	{
		Vector3f pos = Vector3f(rightPose.Position);
		pos = gHeadOrientation.Inverted().Transform(pos - gHeadPos);
		Vector3f ovr_rightP = view.Inverted().Transform(pos);

		glm::quat rightQ = _glmFromOvrQuat(rightPose.Orientation);
		
		
		
		Vector3f trans_rightP = MarkerTranslateToPointer(ovr_rightP, rightQ);
		for (auto const &m : tempModels) {
			auto model = m.first;
			model->Pos = trans_rightP;
		}

		removeTempLines();
	}

	//remove tempLines; this is necessary at each timestep to update dynamic lines
	//called in moveTempModels
	void removeTempLines()
	{
		for (auto const &m : tempLines) {
			auto model = m.first;
			tempLines.erase(model);
		}
	}
	
	//clear map of temp models
	void removeTempModels()
	{
		for (auto const &m : tempModels) {
			auto model = m.first;
			tempModels.erase(model);
			//possibly redundant
			RemoveModel(model);
		}
	}

	int numModels = Models.size();

	//Scene() : numModels(0) {}
	Scene() :
		numModels(0)
	{
		CreateTextures();
	}

	void Release()
	{
		while (numModels-- > 0)
			delete Models[numModels];
	}
	~Scene()
	{
		Release();
	}
}; 