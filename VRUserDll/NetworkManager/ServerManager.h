#pragma once
#include "ServerNetwork.h"
#include "NetworkData.h"

class ServerManager
{

public:

    ServerManager(void);
    ~ServerManager(void);

    void update();

	void receiveFromClients();

	void sendProxyUpdate();

	void sendSceneUpdate();

	void sendPresentationMode();

	void sendInitPacket(int);

	void changePresentationMode();

	void changeActiveClient();

private:
	//holding essential location/display variables, push to all clients upon change
	Proxy * serverProxy;
	//master Scene, clients send messages to change this Scene, push to all upon change
	Scene * serverScene;
	//if PresentationMode, Proxy depends on the activeClient
	bool presentationMode;
	//ID of the currently-active client, when bool PresentationMode = true
	unsigned int activeClient;

//old
   // IDs for the clients connecting for table in ServerNetwork 
    static unsigned int client_id;

   // The ServerNetwork object 
    ServerNetwork* network;

	// data buffer
   char network_data[MAX_PACKET_SIZE];
};