#pragma once
#include <winsock2.h>
#include <Windows.h>
#include "ClientNetwork.h"
#include "NetworkData.h"

#include "ClientScene.h"

enum ClientType {
	VR,
	Screen,
};

class ClientManager
{
public:
	ClientManager(ClientType, Proxy *);
	~ClientManager(void);

	ClientNetwork * network;
	Proxy * proxy;
	ClientType client_type;
	ClientScene * clientScene;

	/*
	void sendPacket(Packet);
	void AddRemovable(Model*, bool);
	void AddRemovableMarker(Model*, bool);
	void AddTemp(Model*);
	void AddTempLine(Model*);
	void AddRemovableStraightLine(Model*, Vector3f, Vector3f, glm::quat, bool);
	void AddRemovableCurvedLine(Model*, std::vector<Vector3f>, std::vector<glm::quat>, bool);
	void RemoveModel(Model*);

	void VRActions();
	*/

	//old
	void sendActionPackets();

	void sendStringPackets(Model *);

	void addToModelString(std::string *);

	void addToModelVector(int);

    char network_data[MAX_PACKET_SIZE];

    void update();
	//old

};

