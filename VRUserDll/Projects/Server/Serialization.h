#pragma once

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
		}

		
		template<class Archive>
		void serialize(Archive & ar, TextureBuffer & t, const unsigned int version)
		{
			ar & t.texId;
			ar & t.fboId;
			ar & t.texSize;
		}


		template<class Archive>
		void serialize(Archive & ar, ShaderFill & s, const unsigned int version)
		{
			ar & s.program;
			ar & s.texture;
		}
		

		template<class Archive>
		void serialize(Archive & ar, VertexBuffer & v, const unsigned int version)
		{
			ar & v.buffer;
		}


		template<class Archive>
		void serialize(Archive & ar, IndexBuffer & i, const unsigned int version)
		{
			ar & i.buffer;
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

	} // namespace serialization
} // namespace boost