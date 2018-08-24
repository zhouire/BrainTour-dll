#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


//#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdio.h>

#include <vector>
#include <map>
#include <array>
#include <string>


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

	//default constructor for serialization
	DepthBuffer()
	{}

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

	//default constructor for serialization
	TextureBuffer()
	{}


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

	//default constructor for serialization
	ShaderFill()
	{}


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

/*
//----------------------------------------------------------------
struct VertexBuffer
{
	GLuint    buffer;

	//const void* _vertices;
	void* _vertices;
	const size_t _size;

	VertexBuffer(void* vertices, size_t size) :
		//_vertices(vertices),
		_size(size)
	{
		//for serializing no default constructor
		//_vertices = vertices;
		memcpy(_vertices, vertices, size);
		//_size = (const size_t) size;

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
*/


//-------------------------------------------------------
struct Vertex
{
	friend class boost::serialization::access;

	Vector3f  Pos;
	DWORD     C;
	float     U, V;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & Pos;
		ar & C;
		ar & U;
		ar & V;
	}
};



struct VertexBuffer
{
	GLuint    buffer;

	VertexBuffer(Vertex * vertices, size_t size) 
	{
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, size, (const void*)vertices, GL_STATIC_DRAW);
	}

	//default constructor for serialization
	VertexBuffer()
	{}


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

	IndexBuffer(GLushort * indices, size_t size) 
	{

		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, (const void *) indices, GL_STATIC_DRAW);
	}

	//default constructor for serialization
	IndexBuffer()
	{}


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
	int				client_creator;
	unsigned int	id;

	Vector3f        Pos;
	Quatf           Rot;
	Matrix4f        Mat;
	int             numVertices, numIndices;
	//Vertex          Vertices[20000]; // Note fixed maximum
	//GLushort        Indices[40000];
	std::vector<Vertex>		Vertices;
	std::vector<GLushort>	Indices;
	ShaderFill    * Fill;
	VertexBuffer  * vertexBuffer;
	IndexBuffer   * indexBuffer;

	ShaderFill * plainFill;

	Model(Vector3f pos, ShaderFill * fill, int client_id, unsigned int model_id) :
		client_creator(client_id),
		id(model_id),
		
		numVertices(0),
		numIndices(0),
		Pos(pos),
		Rot(),
		Mat(),
		Fill(fill),
		vertexBuffer(nullptr),
		indexBuffer(nullptr)
	{
		CreatePlainTexture();
	}

	//default constructor for serialization
	Model()
	{
		CreatePlainTexture();
	}

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

	//void AddVertex(const Vertex& v) { Vertices[numVertices++] = v; }
	//void AddIndex(GLushort a) { Indices[numIndices++] = a; }
	void AddVertex(const Vertex& v)
	{
		numVertices += 1;
		Vertices.push_back(v);
	}

	void AddIndex(GLushort a)
	{
		numIndices += 1;
		Indices.push_back(a);
	}


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

	//creating the textures
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


	void CreatePlainTexture()
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

		// Make textures
		static DWORD tex_pixels[256 * 256];

		for (int j = 0; j < 256; ++j)
		{
			for (int i = 0; i < 256; ++i)
			{
				tex_pixels[j * 256 + i] = 0xffffffff;// blank
			}
		}
		TextureBuffer * generated_texture = new TextureBuffer(false, Sizei(256, 256), 4, (unsigned char *)tex_pixels);

		ShaderFill * generated_shaderfill = new ShaderFill(vshader, fshader, generated_texture);

		plainFill = generated_shaderfill;

		glDeleteShader(vshader);
		glDeleteShader(fshader);
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



	void AddSolidColorRect(std::vector<Vector3f> vertices, std::vector<Vector3f> texCoord, DWORD c) {
		//vertices are in clockwise order starting at bottom left
		Vector3f Vert[][2] =
		{
			//may have to change around the texture coordinates
			/*
			vertices[0], Vector3f(0,1), vertices[1], Vector3f(0,0),
			vertices[2], Vector3f(1,0), vertices[3], Vector3f(1,1),
			*/
			vertices[0], texCoord[0], vertices[1], texCoord[1],
			vertices[2], texCoord[2], vertices[3], texCoord[3],
		};

		GLushort RectIndices[] =
		{
			0, 3, 1, 1, 3, 2
		};

		for (int i = 0; i < sizeof(RectIndices) / sizeof(RectIndices[0]); ++i)
			AddIndex(RectIndices[i] + GLushort(numVertices));

		// Generate a quad of vertices
		genQuadVertices(Vert, 1, c);
	}

	/*
	void AddTransparentRect(std::vector<Vector3f> vertices) {
		//vertices are in clockwise order starting at bottom left
		Vector3f Vert[][2] =
		{
			//may have to change around the texture coordinates
			vertices[0], Vector3f(0,1), vertices[1], Vector3f(1,1),
			vertices[2], Vector3f(1,0), vertices[3], Vector3f(0,0),
		};

		GLushort RectIndices[] =
		{
			0, 3, 1, 1, 3, 2
		};

		for (int i = 0; i < sizeof(RectIndices) / sizeof(RectIndices[0]); ++i)
			AddIndex(RectIndices[i] + GLushort(numVertices));

		// Generate a quad of transparent-color vertices
		for (int v = 0; v < 4; v++)
		{
			Vertex vvv; vvv.Pos = Vert[v][0]; vvv.U = Vert[v][1].x; vvv.V = Vert[v][1].y;
			vvv.C = 0x00000000;
			AddVertex(vvv);
		}
	}
	*/




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



	void Render(Matrix4f view, Matrix4f proj, Matrix4f rot)
	{
		Matrix4f modelview = view * rot;
		Matrix4f combined = proj * modelview * GetMatrix();

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


//------------------------------------------------------------------------- 


struct LineComponents
{
	friend class boost::serialization::access;

	std::vector<Vector3f> Core;
	std::vector<glm::quat> Q;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & Core;
		ar & Q;
	}
};


struct Scene
{
	int client_id;
	unsigned int model_id;
	
	//use map for built-in find function
	std::map<Model*, int> worldModels;
	//markers have meaningless int values
	std::map<Model*, int> removableMarkers;
	//straight lines are represented by their vector core [start,end]
	std::map<Model*, LineComponents> removableStraightLines;
	//curved lines are represented by the points in their lineCore
	std::map<Model*, LineComponents> removableCurvedLines;

	std::map<Model*, int> tempWorldMarkers;
	std::map<Model*, int> tempWorldLines;

	//volume rendering maps
	std::map<Model*, int> volumeModels;
	std::map<Model*, int> tempVolumeLines;

	//local temp models; Client can only edit its own temp models
	//std::map<Model*, int> localTempWorldMarkers;
	//std::map<Model*, int> localTempWorldLines;
	//std::map<Model*, int> localTempVolumeLines;

	float MARKER_SIZE = 0.02f;
	float TARGET_SIZE = 0.01f;
	float LINE_THICKNESS = 0.01f;

	//world = green, volume = cyan, target = red, phantom = purple
	DWORD WORLDMODE_COLOR = 0xff008000;
	DWORD VOLUMEMODE_COLOR = 0xFF00FFFF;
	DWORD TARGET_COLOR = 0xFF800000;
	DWORD PHANTOM_COLOR = 0xFFA535F0;

	//ShaderFill * gridMaterial;
	std::vector <ShaderFill*> grid_material;
	//model highlighted for potential removal
	Model *targetModel;
	//targetModelType can be "marker", "straight line" or "curved line"
	std::string targetModelType;
	//targetMode is true if world, false if volume
	bool targetMode;
	int targetModelClient;
	unsigned int targetModelId;

	bool clientMode;
	bool worldMode = true;

	//for line drawing functionality
	std::vector<Vector3f> lineCore;
	std::vector<glm::quat> allHandQ;


	//for creating the HUD
	std::map <const char*, std::array<int, 3>> image_files;
	std::vector<Model*> HUDcomponents;
	bool visibleHUD = true;

	Model * lengthDisplay;

	//std::string imagePath = "E:\\Classes\\2018 Summer\\UROP\\BrainTour-dll\\TestSolution0\\Images\\";
	std::string imagePath = "C:\\Users\\zhoui\\Documents\\8keXm\\BrainTour-dll\\TestSolution0\\Images\\";

	

	/************************************
	Set/get functions
	************************************/
	//these are called from the dll
	/*
	void SetWorldToVoxelScale(float scale) {
		WORLDTOVOXELSCALE = scale;
	}

	void SetVoxelSize(float x, float y, float z) {
		Vector3f size;
		size.x = x;
		size.y = y;
		size.z = z;
		VOXELSIZE = size;
	}
	*/

	/************************************
	Add functions
	*************************************/

	virtual void AddRemovable(Model * n, bool worldMode)
	{
		//don't actually care about the values
		if (worldMode) {
			worldModels.insert(std::pair<Model*, int>(n, 1));
		}
		
		else {
			volumeModels.insert(std::pair<Model*, int>(n, 1));
		}
	}

	virtual void AddTemp(Model * n)
	{
		RemoveModel(n);
		tempWorldMarkers.insert(std::pair<Model*, int>(n, 1));

		//for clients; local editing
		//localTempWorldMarkers.insert(std::pair<Model*, int>(n, 1));
	}

	virtual void AddTempLine(Model * n, bool worldMode)
	{
		if (worldMode) {
			tempWorldLines.insert(std::pair<Model *, int>(n, 1));
			//for clients; local editing
			//localTempWorldLines.insert(std::pair<Model*, int>(n, 1));
		}

		else {
			tempVolumeLines.insert(std::pair<Model *, int>(n, 1));
			//for clients; local editing
			//localTempVolumeLines.insert(std::pair<Model*, int>(n, 1));
		}
	}

	virtual void AddRemovableMarker(Model * n, bool worldMode)
	{
		removableMarkers.insert(std::pair<Model*, int>(n, 1));
		AddRemovable(n, worldMode);
	}

	virtual void AddRemovableStraightLine(Model * n, Vector3f start, Vector3f end, glm::quat handQ, bool worldMode)
	{
		std::vector<Vector3f> core{ start,end };
		LineComponents line;
		line.Core = core;
		std::vector<glm::quat> quat{ handQ };
		line.Q = quat;
		removableStraightLines.insert(std::pair<Model*, LineComponents>(n, line));

		AddRemovable(n, worldMode);
	}

	virtual void AddRemovableCurvedLine(Model * n, std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, bool worldMode)
	{
		LineComponents line;
		line.Core = lineCore;
		line.Q = allHandQ;
		removableCurvedLines.insert(std::pair<Model*, LineComponents>(n, line));

		AddRemovable(n, worldMode);
	}


	//calling this removes non-temp models from all "removable" maps
	virtual void RemoveModel(Model * n)
	{
		//Model * copy = n;

		worldModels.erase(n);
		removableMarkers.erase(n);
		removableStraightLines.erase(n);
		removableCurvedLines.erase(n);
		volumeModels.erase(n);

		delete n;
	}


	void Render(Matrix4f view, Matrix4f proj, Matrix4f rot)
	{
		//Render all worldMode models with a rotation matrix of Identity
		Matrix4f identity = Matrix4f();

		//render HUD and length display in camera view with no rotation
		for (auto const m : HUDcomponents) {
			m->Render(identity, proj, identity);
		}

		if (lengthDisplay) {
			lengthDisplay->Render(identity, proj, identity);
		}

		//Render world models in world view with no rotation
		for (auto const m : worldModels) {
			m.first->Render(view, proj, identity);
		}

		for (auto const m : tempWorldMarkers) {
			m.first->Render(view, proj, identity);
		}

		for (auto const m : tempWorldLines) {
			m.first->Render(view, proj, identity);
		}

		//Render all volumeMode models with the real rotation matrix
		for (auto const m : volumeModels) {
			m.first->Render(view, proj, rot);
		}

		for (auto const m : tempVolumeLines) {
			m.first->Render(view, proj, rot);
		}
	}


	//this function finds and sets the new pointer to the targetModel when the Scene is updated, and if targetModel is not currently nullptr
	//we call this function after the client receives a Scene update from the Server
	//we do not ever call this function within the Scene struct
	void targetModelRefresh()
	{
		if (targetModel) {
			if (targetModelType == "marker") {
				for (auto const m : removableMarkers) {
					if (((m.first->client_creator) == targetModelClient) && ((m.first->id) == targetModelId)) {
						//delete targetModel;
						targetModel = m.first;
						break;
					}
				}
			}

			else if (targetModelType == "straight line") {
				for (auto const m : removableStraightLines) {
					if (((m.first->client_creator) == targetModelClient) && ((m.first->id) == targetModelId)) {
						//delete targetModel;
						targetModel = m.first;
						break;
					}
				}
			}

			else if (targetModelType == "curved line") {
				for (auto const m : removableCurvedLines) {
					if (((m.first->client_creator) == targetModelClient) && ((m.first->id) == targetModelId)) {
						//delete targetModel;
						targetModel = m.first;
						break;
					}
				}
			}
		}
	}


	//rotates the marker pos to accomodate for controller orientation and place it in front of pointer finger
	Vector3f MarkerTranslateToPointer(Vector3f handPos, glm::quat handQuat)
	{
		Vector3f newMarkerPos;

		glm::vec4 u(0, 0, -0.1f, 1.0f);
		glm::vec4 translate = handQuat * u;
		newMarkerPos.x = handPos.x + translate.x;
		newMarkerPos.y = handPos.y + translate.y;
		newMarkerPos.z = handPos.z + translate.z;

		return newMarkerPos;
	}


	Model * CreateMarker(float size, DWORD color, Vector3f pos, bool worldMode)
	{
		Model * marker = new Model(Vector3f(0, 0, 0), (grid_material[0]), client_id, model_id);
		marker->AddSolidColorBox(-0.5f*size, -0.5f*size, -0.5f*size, 0.5f*size, 0.5f*size, 0.5f*size, color);
		marker->AllocateBuffers();
		marker->Pos = pos;
		//AddRemovableMarker(marker, worldMode);

		model_id += 1;

		return marker;
	}


	Model * CreateStraightLine(Vector3f start, Vector3f end, glm::quat handQ, float thickness, DWORD color) {
		//should be okay to put Pos at origin because the line construction is based on the origin
		//However, Pos will NOT reflect the actual location of the line!
		Model * straightLine = new Model(Vector3f(0, 0, 0), (grid_material[0]), client_id, model_id);
		straightLine->AddStraightLine(start, end, handQ, thickness, color);
		straightLine->AllocateBuffers();

		model_id += 1;

		return straightLine;
	}


	Model * CreateCurvedLine(std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, float thickness, DWORD color) {
		//should be okay to put Pos at origin because the line construction is based on the origin
		//However, Pos will NOT reflect the actual location of the line!
		Model * curvedLine = new Model(Vector3f(0, 0, 0), (grid_material[0]), client_id, model_id);
		curvedLine->AddCurvedLine(lineCore, allHandQ, thickness, color);
		curvedLine->AllocateBuffers();

		model_id += 1;

		return curvedLine;
	}


	Model * CreateTextBox(std::vector<Vector3f> defaultVertices, std::vector<Vector3f> texCoord, ShaderFill * text) {
		//creates the text
		Model * textForeground = new Model(Vector3f(0, 0, 0), text, client_id, model_id);
		textForeground->AddSolidColorRect(defaultVertices, texCoord, 0xFFFFFFFF);
		textForeground->AllocateBuffers();

		model_id += 1;
		
		return textForeground;
	}

	/*
	std::vector<Vector3f> AnchorVerticesToHead(std::vector<Vector3f> vertices, Vector3f hmdP, glm::quat hmdQ) {
		//given a set of vertices relative to the origin and default orientation, 
		//returns a set of vertices transformed based on the position and orientation of the head
		std::vector<Vector3f> anchored;

		for (int i = 0; i < vertices.size(); i++) {
			Vector3f new_v;
			glm::vec4 v(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
			glm::vec4 translate = hmdQ * v;

			new_v.x = hmdP.x + translate.x;
			new_v.y = hmdP.y + translate.y;
			new_v.z = hmdP.z + translate.z;

			anchored.push_back(new_v);
		}

		return anchored;
	}
	*/


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
		//legend image; image properties {width, height, numChannels}
		std::array<int, 3> image_properties = { 3740, 2528, 32 };
		image_files["ControllerLegend.png"] = image_properties;

		//WorldMode label
		image_properties = { 672, 147, 32 };
		image_files["WorldMode.png"] = image_properties;

		//VolumeMode label
		image_properties = { 699, 137, 32 };
		image_files["VolumeMode.png"] = image_properties;

		//Consolas character map
		image_properties = { 882, 716, 32 };
		image_files["ConsolasTable.png"] = image_properties;
		


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

		// Make textures
		static DWORD tex_pixels[256 * 256];
		static DWORD tex_pixels_semitransparent[256 * 256];

		for (int j = 0; j < 256; ++j)
		{
			for (int i = 0; i < 256; ++i)
			{
				tex_pixels[j * 256 + i] = 0xffffffff;// blank
				tex_pixels_semitransparent[j * 256 + i] = 0x66ffffff; //semitransparent white
			}
		}
		TextureBuffer * generated_texture = new TextureBuffer(false, Sizei(256, 256), 4, (unsigned char *)tex_pixels);
		TextureBuffer * generated_texture_st = new TextureBuffer(false, Sizei(256, 256), 4, (unsigned char *)tex_pixels_semitransparent);

		ShaderFill * generated_shaderfill = new ShaderFill(vshader, fshader, generated_texture);
		ShaderFill * generated_shaderfill_st = new ShaderFill(vshader, fshader, generated_texture_st);

		grid_material.push_back(generated_shaderfill);
		grid_material.push_back(generated_shaderfill_st);
		

		for (auto i : image_files) {
			//char* name = i.first;
			char* name = (char*)(imagePath + i.first).c_str();
			unsigned char *data = stbi_load(name, &(i.second[0]), &(i.second[1]), &(i.second[2]), 0);
			//unsigned char *data = stbi_load(name, &(i.second[0]), &(i.second[1]), &(i.second[2]), STBI_rgb_alpha);
			TextureBuffer * generated_texture = new TextureBuffer(true, Sizei(i.second[0], i.second[1]), 4, data);
			ShaderFill * generated_shader = new ShaderFill(vshader, fshader, generated_texture);
			grid_material.push_back(generated_shader);
		}


		glDeleteShader(vshader);
		glDeleteShader(fshader);
	}


	/***********************************
	dynamic text functions
	************************************/
	std::map<char, std::vector<Vector3f>> GenerateCharTexCoordMap() {
		//assumes that the texture map is in ASCII order, 6*16
		float char_height = 0.1666f;
		float char_width = 0.0625f;
		//std::map<std::string, std::vector<Vector3f>> texCoordMap;
		
		std::map<char, std::vector<Vector3f>> texCoordMap;
		
		for (int h = 0x0; h < 0x6; h++) {
			for (int w = 0x0; w < 0x10; w++) {
				int asciiChar = (h + 0x2) * 0x10 + w;
				char c;
				

				if (asciiChar != 0x7f) {
					c = (char)asciiChar;
				}
				else {
					c = '?';
				}

				std::vector<Vector3f> v;
				/*
				v.push_back(Vector3f(w*char_width, (h + 1)*char_height));
				v.push_back(Vector3f(h*char_height, w*char_width));
				v.push_back(Vector3f((w + 1)*char_width, h*char_height));
				v.push_back(Vector3f((w + 1)*char_width, (h + 1)*char_height));
				*/
				v.push_back(Vector3f(w*char_width, h*char_height));
				v.push_back(Vector3f(w*char_width, (h + 1)*char_height));
				v.push_back(Vector3f((w + 1)*char_width, (h + 1)*char_height));
				v.push_back(Vector3f((w + 1)*char_width, h*char_height));

				//texCoordMap[s] = v;
				texCoordMap[c] = v;
			}
		}
		
		/*
		std::vector<Vector3f> hi;
		hi.push_back(Vector3f(0.9375f,0.833f));
		hi.push_back(Vector3f(0.9375f, 1));
		hi.push_back(Vector3f(1,1));
		hi.push_back(Vector3f(1, 0.833f));
		*/
		/*
		std::vector<Vector3f> hi;
		hi.push_back(Vector3f(0, 0));
		hi.push_back(Vector3f(0, 1));
		hi.push_back(Vector3f(1, 1));
		hi.push_back(Vector3f(1, 0));
		*/

		//texCoordMap['a'] = hi;
		return texCoordMap;
	}

	std::string LengthToString(float length) {
		//this assumes the length is in nanometers
		int digits = log10(length);
		if (digits < 0) {
			digits = 0;
		}

		std::string s;
		if (digits < 3) {
			//nanometers
			s = std::to_string(length);
			//3 digits to the right of the decimal point
			s = s.substr(0, (digits + 5));
			s.push_back(' ');
			s.push_back('n');
			s.push_back('m');
		}
		else if (digits < 6) {
			//micrometers
			float l = length / 1000;
			s = std::to_string(l);
			s = s.substr(0, (digits + 2));
			s.push_back(' ');
			s.push_back('?');
			s.push_back('m');
		}
		else if (digits < 9) {
			//millimeters
			float l = length / 1000000;
			s = std::to_string(l);
			s = s.substr(0, (digits - 1));
			s.push_back(' ');
			s.push_back('m');
			s.push_back('m');
		}
		else {
			//meters
			float l = length / 1000000000;
			s = std::to_string(l);
			s = s.substr(0, (digits - 4));
			s.push_back(' ');
			s.push_back('m');
		}

		return s;
	}


	std::vector<std::vector<Vector3f>> GenerateTextVertices (Vector3f startCoord, float width, float height){
		//startCoord is the bottom left vertex of the rightmost character
		//generates text vertices from right to left
		std::vector<std::vector<Vector3f>> textVertices;
		
		//we technically only need a max of 10 spaces
		for (int i = 0; i < 11; i++) {
			std::vector<Vector3f> charVertices;

			charVertices.push_back(Vector3f((startCoord.x + (i*width)), startCoord.y + height, startCoord.z));
			charVertices.push_back(Vector3f((startCoord.x + (i*width)), startCoord.y, startCoord.z));
			charVertices.push_back(Vector3f((startCoord.x + ((i + 1)*width)), startCoord.y, startCoord.z));
			charVertices.push_back(Vector3f((startCoord.x + ((i + 1)*width)), startCoord.y + height, startCoord.z));
		
			textVertices.push_back(charVertices);
		}

		return textVertices;
	}


	Model * CreateLengthText(float length) {
		Vector3f startCoord{ -0.5f, 0.5f, -2.5f };
		//float width = 1.0f;
		//float height = 2.17f;
		float width = 0.05f;
		float height = 0.1085f;

		std::map<char, std::vector<Vector3f>> texCoordMap = GenerateCharTexCoordMap();
		std::vector<std::vector<Vector3f>> textVertices = GenerateTextVertices(startCoord, width, height);
		std::string s = LengthToString(length);

		Model * m = new Model(Vector3f(0, 0, 0), grid_material[5], client_id, model_id);

		model_id += 1;

		//for (int i = (s.length()-1); i >= 0; i--) { 
		for (int i = 0; i < s.length(); i++) { 
			char c = s.at(i);
			std::vector<Vector3f> texCoord = texCoordMap[c];
			//std::vector<Vector3f> texCoord = texCoordMap['a'];
			//std::vector<Vector3f> vertices = textVertices[textVertices.size() - 1 - i];
			std::vector<Vector3f> vertices = textVertices[i];
			
			m->AddSolidColorRect(vertices, texCoord, 0xFFFFFFFF);
		}

		m->AllocateBuffers();

		return m;
	}


	float CalculateSegmentLength(Vector3f start, Vector3f end, Matrix4f rot, bool worldMode, float scale, Vector3f voxelSize) {
		Vector3f s = start;
		Vector3f e = end;
		if (worldMode) {
			s = rot.Inverted().Transform(s);
			e = rot.Inverted().Transform(e);
		}
		
		Vector3f diff = s - e;
		diff.x = diff.x / scale * voxelSize.x;
		diff.y = diff.y / scale * voxelSize.y;
		diff.z = diff.z / scale * voxelSize.z;

		return diff.Length();
	}


	float CalculateLength(std::vector<Vector3f> lineCore, Matrix4f rot,  bool worldMode, float scale, Vector3f voxelSize) {
		float length = 0;
		for (int i = 0; i < (lineCore.size() - 1); i++) {
			length += CalculateSegmentLength(lineCore[i], lineCore[i + 1], rot, worldMode, scale, voxelSize);
		}
		return length;
	}

	/*************************************************
	*************************************************/

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


	Model * ColorRemovableModel(Vector3f rightHandPos, Vector3f otherModePos)
	{
		Vector3f worldP;
		Vector3f volumeP;
		

		if (worldMode) {
			worldP = rightHandPos;
			volumeP = otherModePos;
		}

		else {
			worldP = otherModePos;
			volumeP = rightHandPos;
		}

		Vector3f rightP = volumeP;

		//for world mode objects, calc dist using worldP
		//make sure there is currently no targetModel when ColorRemovableModel is called
		if (!targetModel) {
			for (auto const m : removableMarkers) {
				Model *model = m.first;
				Vector3f modelPos = (*model).Pos;
				//true if world, false if volume
				//bool mode = (volumeModels.count(model) == 0);
				//resetting rightP value
				rightP = volumeP;

				if (volumeModels.count(model) == 0) {
					rightP = worldP;
				}

				if (rightP.Distance(modelPos) <= TARGET_SIZE) {
					//true if world, false if volume; needed to make a new red marker
					targetMode = (volumeModels.count(model) == 0);

					Model *newMarker = CreateMarker(MARKER_SIZE, TARGET_COLOR, modelPos, targetMode);

					//removing the old green model; replaced by new red one
					RemoveModel(model);
					//removed this from CreateMarker function; put here
					AddRemovableMarker(newMarker, targetMode);
					
					targetModel = newMarker;
					targetModelType = "marker";
					targetModelClient = newMarker->client_creator; 
					targetModelId = newMarker->id;

					//have to manually add targetModel to maps now (no function), so the rest of the code can detect it before the Scene update
					removableMarkers.insert(std::pair<Model*, int>(targetModel, 1));
					if (targetMode) {
						worldModels.insert(std::pair<Model*, int>(targetModel, 1));
					}
					else {
						volumeModels.insert(std::pair<Model*, int>(targetModel, 1));
					}

					
					return newMarker;
				}
			}

			for (auto const m : removableStraightLines) {
				Model *model = m.first;
				//resetting rightP value
				rightP = volumeP;

				//true if world, false if volume
				if (volumeModels.count(model) == 0) {
					rightP = worldP;
				}

				float dist = DistPointToLineSeg(rightP, (m.second).Core);
				if (dist <= TARGET_SIZE) {
					//true if world, false if volume; needed to make a new red marker
					targetMode = (volumeModels.count(model) == 0);

					LineComponents lc = m.second;
					Model *newStraightLine = CreateStraightLine(lc.Core[0], lc.Core[1],
						lc.Q[0], LINE_THICKNESS, TARGET_COLOR);

					RemoveModel(model);
					AddRemovableStraightLine(newStraightLine, lc.Core[0], lc.Core[1], lc.Q[0], targetMode);
					//AddRemovableStraightLine(targetModel, lc.Core[0], lc.Core[1], lc.Q[0], targetMode);

					/*
					Model *newStraightLine = CreateStraightLine((m.second).Core[0], (m.second).Core[1],
						(m.second).Q[0], LINE_THICKNESS, TARGET_COLOR);
					//adding the new model to appropriate maps
					AddRemovableStraightLine(newStraightLine, (m.second).Core[0], (m.second).Core[1], (m.second).Q[0], targetMode);
					//removing the old model from all maps
					RemoveModel(model);
					*/
					
					targetModel = newStraightLine;
					targetModelType = "straight line";
					targetModelClient = newStraightLine->client_creator;
					targetModelId = newStraightLine->id;

					//have to manually add targetModel to maps now (no function), so the rest of the code can detect it before the Scene update
					removableStraightLines.insert(std::pair<Model*, LineComponents>(targetModel, lc));
					if (targetMode) {
						worldModels.insert(std::pair<Model*, int>(targetModel, 1));
					}
					else {
						volumeModels.insert(std::pair<Model*, int>(targetModel, 1));
					}
					

					return newStraightLine;
				}
			}

			for (auto const m : removableCurvedLines) {
				Model *model = m.first;
				//collision detection with line segments
				std::vector<Vector3f> core = (m.second).Core;

				//resetting rightP value
				rightP = volumeP;

				//true if world, false if volume; change rightP depending on model mode
				bool mode = (volumeModels.count(model) == 0);
				if (volumeModels.count(model) == 0) {
					rightP = worldP;
				}

				for (int i = 0; i < (core.size()-1); i++) {
					std::vector<Vector3f> v_seg{ core[i],core[i + 1] };
					if (DistPointToLineSeg(rightP, v_seg) <= TARGET_SIZE) {
						//true if world, false if volume; needed to make a new red marker
						targetMode = (volumeModels.count(model) == 0);

						LineComponents lc = m.second;
						Model * newCurvedLine = CreateCurvedLine(lc.Core, lc.Q, LINE_THICKNESS, TARGET_COLOR);

						RemoveModel(model);
						AddRemovableCurvedLine(newCurvedLine, lc.Core, lc.Q, targetMode);
						//AddRemovableCurvedLine(targetModel, lc.Core, lc.Q, targetMode);
						
						/*
						Model *newCurvedLine = CreateCurvedLine((m.second).Core, (m.second).Q, LINE_THICKNESS, TARGET_COLOR);
						//adding the new model to appropriate maps
						AddRemovableCurvedLine(newCurvedLine, (m.second).Core, (m.second).Q, targetMode);
						//removing the old model from all maps
						RemoveModel(model);
						*/
						
						targetModel = newCurvedLine;
						targetModelType = "curved line";
						targetModelClient = newCurvedLine->client_creator;
						targetModelId = newCurvedLine->id;

						//have to manually add targetModel to maps now (no function), so the rest of the code can detect it before the Scene update
						removableCurvedLines.insert(std::pair<Model*, LineComponents>(targetModel, lc));
						if (targetMode) {
							worldModels.insert(std::pair<Model*, int>(targetModel, 1));
						}
						else {
							volumeModels.insert(std::pair<Model*, int>(targetModel, 1));
						}


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
		DWORD color;
		if (targetMode) {
			color = WORLDMODE_COLOR;
		}
		else {
			color = VOLUMEMODE_COLOR;
		}


		if (targetModelType == "marker") {
			Vector3f targetPos = targetModel->Pos;
			Model *newMarker = CreateMarker(MARKER_SIZE, color, targetPos, targetMode);

			AddRemovableMarker(newMarker, targetMode);
		}

		else if (targetModelType == "straight line") {
			std::vector<Vector3f> core = (removableStraightLines.find(targetModel)->second).Core;
			std::vector<glm::quat> handQ = (removableStraightLines.find(targetModel)->second).Q;
			Model *newStraightLine = CreateStraightLine(core[0], core[1], handQ[0], LINE_THICKNESS, color);
			AddRemovableStraightLine(newStraightLine, core[0], core[1], handQ[0], targetMode);
		}

		else if (targetModelType == "curved line") {
			std::vector<Vector3f> core = (removableCurvedLines.find(targetModel)->second).Core;
			std::vector<glm::quat> handQ = (removableCurvedLines.find(targetModel)->second).Q;
			Model *newCurvedLine = CreateCurvedLine(core, handQ, LINE_THICKNESS, color);
			AddRemovableCurvedLine(newCurvedLine, core, handQ, targetMode);
		}
		
		//removing the red target model from all maps; has been replaced already with a green model
		RemoveModel(targetModel);
		//delete targetModel;
		targetModel = nullptr;
	}


	//checks to see if the targetmodel can be cleared so a new model is allowed to be the targetmodel
	void CheckTargetModel(Vector3f rightHandPos, Vector3f otherModePos)
	{
		Vector3f worldP;
		Vector3f volumeP;
		

		if (worldMode) {
			worldP = rightHandPos;
			volumeP = otherModePos;
		}

		else {
			worldP = otherModePos;
			volumeP = rightHandPos;
		}

		Vector3f rightP = volumeP;

		//true if world, false if volume; change rightP depending on model mode
		if (targetMode) {
			rightP = worldP;
		}

		//make sure targetModel is not nullptr
		if (targetModel) {
			if (targetModelType == "marker") {
				Vector3f targetPos = targetModel->Pos;
				if (rightP.Distance(targetPos) > TARGET_SIZE) {
					ResetTargetModel();
				}
			}

			else if (targetModelType == "straight line") {
				std::vector<Vector3f> core = (removableStraightLines.find(targetModel)->second).Core;
				float dist = DistPointToLineSeg(rightP, core);
				if (dist > TARGET_SIZE) {
					ResetTargetModel();
				}
			}

			else if (targetModelType == "curved line") {
				bool dist_far = true;
				std::vector<Vector3f> core = (removableCurvedLines.find(targetModel)->second).Core;
				for (int i = 0; i < (core.size() - 1); i++) {
					std::vector<Vector3f> v_seg{ core[i],core[i + 1] };
					if (DistPointToLineSeg(rightP, v_seg) <= TARGET_SIZE) {
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
		ovrInputState &inputState, Matrix4f &gHeadOrientation, OVR::Matrix4f& view, float scale, Vector3f voxelSize)
	{
		//oculus has tendency to detect multiple presses on one press; prevent this
		static bool switchMode = true;

		Vector3f trans_rightP;
		Vector3f other_rightP;

		glm::quat rightQ;

		//pose and orientation variables for world mode
		rightQ = _glmFromOvrQuat(rightPose.Orientation);
			
		Vector3f pos = Vector3f(rightPose.Position);
		Vector3f new_pos = MarkerTranslateToPointer(pos, rightQ);
		new_pos = gHeadOrientation.Inverted().Transform(new_pos - gHeadPos);
		trans_rightP = view.Inverted().Transform(new_pos);

		OVR::Matrix4f rot(gPose);
		other_rightP = rot.Inverted().Transform(trans_rightP);

		//poses for volume mode; simple swap
		if (!worldMode) {
			Vector3f temp = trans_rightP;
			trans_rightP = other_rightP;
			other_rightP = temp;
		}

		DWORD color;
		if (worldMode) {
			color = WORLDMODE_COLOR;
		}
		else {
			color = VOLUMEMODE_COLOR;
		}
		

		//should not be allowed to simply hold button A and continuously make a stream of models; let go and press again
		static bool canCreateMarker = true;
		static bool drawingStraightLine = false;
		static bool drawingCurvedLine = false;
		static bool phantomMarkerExists = false;

		static bool canSwitchHUD = true;


		//if we are actively drawing a line
		if (lineCore.size() > 0) {
			//if we are drawing a straight line, create a new phantom line
			if (drawingStraightLine) {
				Model *newStraightLine = CreateStraightLine(lineCore[0], trans_rightP, rightQ, LINE_THICKNESS, color);
				//if B let go (ending the line), just create the line and reset drawingStraightLine and lineCore
				if (!(inputState.Buttons & ovrButton_B)) {
					AddRemovableStraightLine(newStraightLine, lineCore[0], trans_rightP, rightQ, worldMode);
					drawingStraightLine = false;
					lineCore.clear();
				}
				//dynamic lines must be regenerated at each timestamp
				else {
					AddTempLine(newStraightLine, worldMode);
				}				
			}
			
			//if we are drawing a curved line
			else if (drawingCurvedLine) {

				if ((lineCore.size() == 1) || (trans_rightP.Distance(lineCore[lineCore.size() - 1]) >= 0.005f)) {
					lineCore.push_back(trans_rightP);
					allHandQ.push_back(rightQ);
				}

				int numVerticesNext = 8 + (16 * (lineCore.size() + 1));

				Model *newCurvedLine = CreateCurvedLine(lineCore, allHandQ, LINE_THICKNESS, color);
				
				//if we stop drawing the curved line (PRESSING index), put the line in worldModels and reset
				//if the next lineCore addition will exceed the vertex limit (currently 20000), auto-stop drawing
				if (inputState.IndexTrigger[ovrHand_Right] < 0.2f || numVerticesNext >= 20000) {
					drawingCurvedLine = false;

					AddRemovableCurvedLine(newCurvedLine, lineCore, allHandQ, worldMode);

					lineCore.clear();
					allHandQ.clear();
				}

				//dynamic lines must be regenerated at every timestamp
				else {
					AddTempLine(newCurvedLine, worldMode);
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

			//stop showing the phantom marker if not pressing A
			if (!(inputState.Buttons & ovrButton_A)) {
				if (phantomMarkerExists) {
					removeTempMarkers();
					phantomMarkerExists = false;
				}

				if (targetModel) {
					ResetTargetModel();
				}
			}
			//allow user to create a new marker if they have stopped pressing X
			//allow user to create a new marker if they have stopped pressing X
			if (!(inputState.Buttons & ovrButton_X) && !canCreateMarker) {
				canCreateMarker = true;
			}
			/*
			//clear the target model if the user stops pressing A
			if (!(inputState.Buttons & ovrButton_A) && targetModel) {
				ResetTargetModel();
			}
			*/

			//create a new marker if the user is pressing A+X and the user is allowed to
			if (inputState.Buttons & ovrButton_X && canCreateMarker) {
				if (inputState.Buttons & ovrButton_A) {
					Model * marker = CreateMarker(MARKER_SIZE, color, trans_rightP, worldMode);

					AddRemovableMarker(marker, worldMode);

					canCreateMarker = false;
				}
			}
			//if user is pressing A, create phantom marker
			else if (inputState.Buttons & ovrButton_A) {
				Vector3f pos = trans_rightP;
				if (!worldMode){
					pos = other_rightP;
				}

				if (!phantomMarkerExists) {
					Model * newMarker = CreateMarker(MARKER_SIZE, PHANTOM_COLOR, pos, true);
					AddTemp(newMarker);
					phantomMarkerExists = true;
				}

				//try to find a targetModel if there is not one currently
				if (!targetModel) {
					ColorRemovableModel(trans_rightP, other_rightP);
				}

				else {
					//delete targetModel if pressing Y
					if (inputState.Buttons & ovrButton_Y) {
						RemoveModel(targetModel);

						//clear targetModel because the model in question has been removed
						//delete targetModel;
						targetModel = nullptr;
					}
					//try to clear targetModel by seeing if the user has moved away if there is one currently
					else {
						
						CheckTargetModel(trans_rightP, other_rightP);
					}
				}
			}
		}

		//switch modes if pressing Y
		if (inputState.Buttons & ovrButton_Y) {
			if (switchMode && !(inputState.Buttons & ovrButton_A)) {
				//need to regenerate the HUD after switching modes
				ClearHUD();

				worldMode = !worldMode;
				switchMode = false;

				GenerateHUD();
			}
		}
		else {
			switchMode = true;
		}

		//menu button, switch on and off
		if (inputState.Buttons & ovrButton_Enter) {
			if (canSwitchHUD == true) {
				canSwitchHUD = false;
				visibleHUD = !visibleHUD;
			}
		}

		else {
			canSwitchHUD = true;
		}

		//generate the HUD if visibleHUD is true; we are doing it here because this is the only place 
		//we have the proper variables
		if (visibleHUD) {
			GenerateHUD();
		}
		else {
			ClearHUD();
		}

		//creates or destroys the length display depending on the presence of a target model
		//or a length display
		if (targetModel && !lengthDisplay) {
			if (targetModelType == "straight line") {
				std::vector<Vector3f> core = removableStraightLines[targetModel].Core;
				float len = CalculateLength(core, rot, targetMode, scale, voxelSize);
				lengthDisplay = CreateLengthText(len);
			}

			else if (targetModelType == "curved line") {
				std::vector<Vector3f> core = removableCurvedLines[targetModel].Core;
				float len = CalculateLength(core, rot, targetMode, scale, voxelSize);
				lengthDisplay = CreateLengthText(len);
			}	
		}

		else if (!targetModel && lengthDisplay) {
			lengthDisplay = nullptr;
		}
	}
	

	//move all temp markers to the current hand position, all temp lines removed
	//always in world mode
	void moveTempModels(ovrPosef rightPose, OVR::Quatf& gPose, Vector3f &gHeadPos, Matrix4f &gHeadOrientation, Matrix4f& view)
	{
		glm::quat rightQ = _glmFromOvrQuat(rightPose.Orientation);

		Vector3f pos = Vector3f(rightPose.Position);
		Vector3f new_pos = MarkerTranslateToPointer(pos, rightQ);
		new_pos = gHeadOrientation.Inverted().Transform(new_pos - gHeadPos);
		Vector3f trans_rightP = view.Inverted().Transform(new_pos);
		
		for (auto const &m : tempWorldMarkers) {
		//for (auto const &m : localTempWorldMarkers) {
			auto model = m.first;

			if (clientMode) {
				if (model->client_creator == client_id) {
					moveTempModel(model, trans_rightP);
				}
			}

			//this generally shouldn't happen
			else {
				moveTempModel(model, trans_rightP);
			}
			
			//model->Pos = trans_rightP;
		}

		removeTempLines();
	}

	//overridden in ClientScene to send a message to the Server
	//do not move the model locally, because it should be moved here via the pointer (try moving it in ClientScene if doesn't work)
	virtual void moveTempModel(Model * m, Vector3f newPos)
	{
		m->Pos = newPos;
	}

	

	//remove tempLines; this is necessary at each timestep to update dynamic lines
	//called in moveTempModels
	void removeTempLines()
	{
		if (clientMode) {
			//for (auto const &m : localTempWorldLines) {
			for (auto const &m : tempWorldLines) {
				auto model = m.first;
				//tempWorldLines.erase(model);
				if (model->client_creator == client_id) {
					removeTempLine(model);
				}
			}

			for (auto const &m : tempVolumeLines) {
				auto model = m.first;
				//tempVolumeLines.erase(model);
				if (model->client_creator == client_id) {
					removeTempLine(model);
				}
			}
		}

		else {
			for (auto const &m : tempWorldLines) {
				auto model = m.first;
				//tempWorldLines.erase(model);
				removeTempLine(model);
			}

			for (auto const &m : tempVolumeLines) {
				auto model = m.first;
				//tempVolumeLines.erase(model);
				removeTempLine(model);
			}
		}
	}


	//in Client Scene, this will be overwritten to send message and remove locally
	virtual void removeTempLine(Model * m) {
		/*
		if (!clientMode) {
			tempWorldLines.erase(m);
			tempVolumeLines.erase(m);
		}
		*/
		tempWorldLines.erase(m);
		tempVolumeLines.erase(m);
		//the server's local maps are not broadcast to clients; they don't have a real purpose
		//localTempWorldLines.erase(m);
		//localTempVolumeLines.erase(m);

	}
	
	//clear map of temp markers
	void removeTempMarkers()
	{	
		if (clientMode) {
			//for (auto const &m : localTempWorldMarkers) {
			for (auto const &m : tempWorldMarkers) {
				auto model = m.first;
				
				if (model->client_creator == client_id) {
					removeTempMarker(model);
				}
			}
		}

		else {
			for (auto const &m : tempWorldMarkers) {
				auto model = m.first;
				/*
				tempWorldMarkers.erase(model);
				//possibly redundant
				RemoveModel(model);
				*/
				removeTempMarker(model);
			}
		}
		
	}

	//in a Client Scene, this will be overriden to send a message and remove locally
	virtual void removeTempMarker(Model * model) {
		tempWorldMarkers.erase(model);
		
		//localTempWorldMarkers.erase(model);

		//possibly redundant
		//RemoveModel(model);
	}


	//clears the vector of HUD elements; clear and reset at every timeslot
	void ClearHUD()
	{
		HUDcomponents.clear();
	}

	//generates and stores essential HUD components
	void GenerateHUD() {

		//creates the controller action legend

		float default_x = (image_files["ControllerLegend.png"][0]) / 500;
		float default_y = (image_files["ControllerLegend.png"][1]) / 500;
		float depth = -15;

		
		std::vector<Vector3f> defaultVertices{ Vector3f{ -default_x, -default_y, depth },
			Vector3f{ -default_x, default_y, depth },
			Vector3f{ default_x, default_y, depth },
			Vector3f{ default_x, -default_y, depth } };
		//transparent black: 0x66000000
		//opaque yellow: 0xFFFFFF00

		std::vector<Vector3f> texCoord{ Vector3f(0,1), Vector3f(0,0),
			Vector3f(1,0), Vector3f(1,1) };
		
		Model* controllerLegend = CreateTextBox(defaultVertices, texCoord, grid_material[2]);
		HUDcomponents.push_back(controllerLegend);


		//world/volume mode labels
		if (worldMode) {
			float mode_default_x = (image_files["WorldMode.png"][0]) / 137;
			float mode_default_y = (image_files["WorldMode.png"][1]) / 137;
			depth = -15;

			
			std::vector<Vector3f> defaultVertices{ Vector3f{ -mode_default_x, (-2 * mode_default_y - default_y), depth },
				Vector3f{ -mode_default_x, (-default_y), depth },
				Vector3f{ mode_default_x, (-default_y), depth },
				Vector3f{ mode_default_x, (-2 * mode_default_y - default_y), depth } };

			//grid_material[3] -> world mode label
			Model* worldLabel = CreateTextBox(defaultVertices, texCoord, grid_material[3]);
			HUDcomponents.push_back(worldLabel);
		}

		else {
			float mode_default_x = (image_files["VolumeMode.png"][0]) / 137;
			float mode_default_y = (image_files["VolumeMode.png"][1]) / 137;
			depth = -15;
			
			std::vector<Vector3f> defaultVertices{ Vector3f{ -mode_default_x, (-2 * mode_default_y - default_y), depth },
				Vector3f{ -mode_default_x, (-default_y), depth },
				Vector3f{ mode_default_x, (-default_y), depth },
				Vector3f{ mode_default_x, (-2 * mode_default_y - default_y), depth } };

			//grid_material[4] -> volume mode label
			Model* volumeLabel = CreateTextBox(defaultVertices, texCoord, grid_material[4]);
			HUDcomponents.push_back(volumeLabel);
			
		}
	}

	//initialization; when scene created, create textures as well
	Scene(bool client)
	{
		clientMode = client;
		model_id = 0;
		CreateTextures();
	}
}; 



