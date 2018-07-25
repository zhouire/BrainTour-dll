#pragma once
#include <string.h>
#include <string>

#define MAX_PACKET_SIZE 1000000

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

};


struct Proxy {

	Proxy()
	{}

	float *Position;
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
};


struct Packet {
	unsigned int packet_type;

	//used in INIT_CONNECTION
	Proxy * proxy;
	Scene * s;
	Model * m;
	bool worldMode;
	std::vector<Vector3f> * lineCore;
	std::vector<glm::quat> * allHandQ;
	int clientId;
	bool active;




    void serialize(char * data) {
        memcpy(data, this, sizeof(Packet));
    }

    void deserialize(char * data) {
        memcpy(this, data, sizeof(Packet));
    }
};

/*
struct StringPacket{

	unsigned int packet_type;
	std::string str;

	void serialize(char * data) {
		memcpy(data, this, sizeof(StringPacket));
	}

	void deserialize(char * data) {
		memcpy(this, data, sizeof(StringPacket));
	}
};
*/