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
	ClientManager(ClientType);
	~ClientManager(void);

	ClientNetwork * network;
	Proxy * clientProxy;
	ClientType client_type;
	ClientScene * clientScene;

	bool init;
	float clientId;
	bool active;
	bool presentationMode;

    char network_data[MAX_PACKET_SIZE];

    void update();
	
	void controllerUpdate(Proxy *, ovrTrackingState, ovrInputState, int&);

	void sendClientProxyUpdate();
};

