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

	//old
	void sendActionPackets();

	void sendStringPackets(Model *);

	void addToModelString(std::string *);

	void addToModelVector(int);

    char network_data[MAX_PACKET_SIZE];

    void update();
	//old

};

