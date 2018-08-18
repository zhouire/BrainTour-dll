#pragma once

#include "Serialization.h"
#include <string.h>
#include <string>

#define MAX_PACKET_SIZE 1000000
//#define MAX_PACKET_SIZE 600000

enum PacketTypes {

	INIT_CONNECTION = 0,

	SERVER_PROXY_UPDATE = 1,

	SERVER_SCENE_UPDATE = 2,

	ADD_REMOVABLE = 3,

	ADD_REMOVABLE_MARKER = 4,

	ADD_TEMP = 5,

	ADD_TEMP_LINE = 6,

	ADD_REMOVABLE_STRAIGHT_LINE = 7,

	ADD_REMOVABLE_CURVED_LINE = 8,

	REMOVE_MODEL = 9,

	MOVE_TEMP_MODEL = 10,

	REMOVE_TEMP_LINE = 11,

	REMOVE_TEMP_MARKER = 12,

	CLIENT_PROXY_UPDATE = 13,

	SERVER_PRESENTATION_MODE = 14,
};


struct Proxy {
	friend class boost::serialization::access;

	Proxy()
	{}

	//float *Position;
	float PositionX;
	float PositionY;
	float PositionZ;

	int ClipMode;
	float ClipWidth;
	float ClipPos;
	Matrix4f Pose;
	Quatf gPose;
	Vector3f gHeadPos;
	Matrix4f gHeadOrientation;

	float scale;
	Vector3f voxelSize;

	Matrix4f view;
	Matrix4f proj;

	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & PositionX;
		ar & PositionY;
		ar & PositionZ;
		ar & ClipMode;
		ar & ClipWidth;
		ar & ClipPos;
		ar & Pose;
		ar & gPose;
		ar & gHeadPos;
		ar & gHeadOrientation;
		ar & scale;
		ar & voxelSize;
		ar & view;
		ar & proj;
	}

};


//this is a small segment sent before each Packet, defining the Packet's size for deserialization
struct Size {
	Size() {}

	int size;

	void serialize(char * data) {
		memcpy(data, this, sizeof(Size));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(Size));
	}
};


struct Packet {
	friend class boost::serialization::access;

	unsigned int packet_type;

	//used in INIT_CONNECTION
	Proxy proxy;
	BasicScene scene;
	Model m;
	bool worldMode;
	std::vector<Vector3f> lineCore;
	std::vector<glm::quat> allHandQ;
	int clientId;
	bool active;

	Packet() :
		m(Vector3f(0, 0, 0), nullptr)
	{}


	template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & packet_type;
		ar & proxy;
		ar & scene;
		ar & m;
		ar & worldMode;
		ar & lineCore;
		ar & allHandQ;
		ar & clientId;
		ar & active;
	}
};
