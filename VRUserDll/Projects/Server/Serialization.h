#pragma once

#include <SceneManager.h>

//this contains only the variables required to Render a Scene
struct BasicScene
{
	friend class boost::serialization::access;

	std::map<Model*, int> worldModels;
	std::map<Model*, int> tempWorldMarkers;
	std::map<Model*, int> tempWorldLines;
	std::map<Model*, int> volumeModels;
	std::map<Model*, int> tempVolumeLines;
	std::map<Model*, int> removableMarkers;
	std::map<Model*, LineComponents> removableStraightLines;
	std::map<Model*, LineComponents> removableCurvedLines;


	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & worldModels;
		ar & tempWorldMarkers;
		ar & tempWorldLines;
		ar & volumeModels;
		ar & tempVolumeLines;
		ar & removableMarkers;
		ar & removableStraightLines;
		ar & removableCurvedLines;
	}
	
};


namespace boost {
	namespace serialization {

		template<class Archive>
		void serialize(Archive & ar, glm::quat & g, const unsigned int version)
		{
			ar & g.x;
			ar & g.y;
			ar & g.z;
			ar & g.w;
		}


		template<class Archive>
		void serialize(Archive & ar, OVR::Vector3f & v, const unsigned int version)
		{
			ar & v.x;
			ar & v.y;
			ar & v.z;
		}


		template<class Archive>
		void serialize(Archive & ar, OVR::Quatf & q, const unsigned int version)
		{
			ar & q.x;
			ar & q.y;
			ar & q.z;
			ar & q.w;
		}

		
		template<class Archive>
		void serialize(Archive & ar, OVR::Matrix4f & m, const unsigned int version)
		{
			ar & m.M;
		}


		template <class Archive>
		void serialize(Archive & ar, OVR::Sizei & s, const unsigned int version)
		{
			ar & s.w;
			ar & s.h;
		}

		template<class Archive>
		void serialize(Archive & ar, DepthBuffer & d, const unsigned int version)
		{
			ar & d.texId;
			//ar & d._size;
		}

		/*
		template<class Archive>
		inline void save_construct_data(Archive & ar, const DepthBuffer * d, const unsigned int version)
		{
			// save data required to construct instance
			ar << d->_size;
		}

		template<class Archive>
		inline void load_construct_data(Archive & ar, DepthBuffer * d, const unsigned int version) 
		{
			// retrieve data from archive required to construct new instance
			Sizei s;
			ar >> s;
			// invoke inplace constructor to initialize instance of my_class
			::new(d)my_class(s);
		}
		*/

		
		template<class Archive>
		void serialize(Archive & ar, TextureBuffer & t, const unsigned int version)
		{
			ar & t.texId;
			ar & t.fboId;
			ar & t.texSize;
		}

		template<class Archive>
		inline void save_construct_data(Archive & ar, const TextureBuffer * t, const unsigned int version)
		{
			// save data required to construct instance
			ar << t->_rendertarget << t->texSize << t->_mipLevels << t->_data;
		}

		template<class Archive>
		inline void load_construct_data(Archive & ar, TextureBuffer * t, const unsigned int version)
		{
			// retrieve data from archive required to construct new instance
			bool rendertarget;
			Sizei size;
			int mipLevels;
			std::vector<unsigned char> data;

			ar >> rendertarget >> size >> mipLevels >> data;
			// invoke inplace constructor to initialize instance of my_class
			::new(t)TextureBuffer(rendertarget, size, mipLevels, &data[0]);
		}




		template<class Archive>
		void serialize(Archive & ar, ShaderFill & s, const unsigned int version)
		{
			ar & s.program;
			ar & s.texture;

			//ar & s._pixelShader;
			//ar & s._vertexShader;
		}

		
		template<class Archive>
		inline void save_construct_data(Archive & ar, const ShaderFill & s, const unsigned int version)
		{
			// save data required to construct instance
			ar << s->_vertexShader << s->_pixelShader << s->texture;
		}

		template<class Archive>
		inline void load_construct_data(Archive & ar, ShaderFill * s, const unsigned int version)
		{
			// retrieve data from archive required to construct new instance
			GLuint vertexShader;
			GLuint pixelShader;
			TextureBuffer * _texture;

			ar >> vertexShader >> pixelShader >> _texture;
			// invoke inplace constructor to initialize instance of my_class
			::new(s)ShaderFill(vertexShader, pixelShader, _texture);
		}


		

		template<class Archive>
		void serialize(Archive & ar, VertexBuffer & v, const unsigned int version)
		{
			ar & v.buffer;
			//ar & v._size;
			//ar & v._vertices;
		}

		
		template<class Archive>
		inline void save_construct_data(Archive & ar, const VertexBuffer * v, const unsigned int version)
		{
			// save data required to construct instance
			ar << v->_size << v->_vertices;
		}

		template<class Archive>
		inline void load_construct_data(Archive & ar, VertexBuffer * v, const unsigned int version)
		{
			// retrieve data from archive required to construct new instance
			//Vertex * vertices;
			std::vector<Vertex> vertices;
			size_t size;

			ar >> size >> vertices;
			// invoke inplace constructor to initialize instance of my_class
			::new(v)VertexBuffer(&vertices[0], size);
		}
		



		template<class Archive>
		void serialize(Archive & ar, IndexBuffer & i, const unsigned int version)
		{
			ar & i.buffer;

			//ar & i._indices;
			//ar & i._size;
		}

		
		template<class Archive>
		inline void save_construct_data(Archive & ar, const IndexBuffer * i, const unsigned int version)
		{
			// save data required to construct instance
			ar << i->_indices << i->_size;
		}

		template<class Archive>
		inline void load_construct_data(Archive & ar, IndexBuffer * i, const unsigned int version)
		{
			// retrieve data from archive required to construct new instance
			//GLushort * indices;
			std::vector<GLushort> indices;
			size_t size;

			ar >> indices >> size;
			// invoke inplace constructor to initialize instance of my_class
			::new(i)IndexBuffer(&indices[0], size);
		}
		




		template<class Archive>
		void serialize(Archive & ar, Model & m, const unsigned int version)
		{
			ar & m.Pos;
			ar & m.Rot;
			ar & m.Mat;
			ar & m.numVertices;
			ar & m.numIndices;
			ar & m.Vertices;
			ar & m.Indices;
			ar & m.Fill;
			ar & m.vertexBuffer;
			ar & m.indexBuffer;
		}

		template<class Archive>
		inline void save_construct_data(Archive & ar, const Model * m, const unsigned int version)
		{
			// save data required to construct instance
			ar << m->Pos << m->Fill;
		}

		template<class Archive>
		inline void load_construct_data(Archive & ar, Model * m, const unsigned int version)
		{
			// retrieve data from archive required to construct new instance
			Vector3f pos;
			ShaderFill * fill;

			ar >> pos >> fill;
			// invoke inplace constructor to initialize instance of my_class
			::new(m)Model(pos, fill);
		}


	} // namespace serialization
} // namespace boost