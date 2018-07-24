#include "StdAfx.h"
#include "ClientManager.h"

#include <vector>

ClientManager::ClientManager(ClientType type, Proxy * p)
{
	
    network = new ClientNetwork();
	client_type = type;
	clientScene = new ClientScene(true, network);
	proxy = p;

	//auto-set to false, set ot true when this is the active client during presentationMode
	//used during presentationMode; server sends a message setting this to true when a client
	//is defined as the active Client using its ID 
	bool active = false;

    // send init packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = INIT_CONNECTION;
	packet.proxy = proxy;

    packet.serialize(packet_data);

    NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}


ClientManager::~ClientManager(void)
{
}


void ClientManager::update()
{
    Packet packet;

    int data_length = network->receivePackets(network_data);

    if (data_length <= 0) 
    {
        //no data recieved
        return;
    }

    int i = 0;
    while (i < (unsigned int)data_length) 
    {		
		packet.deserialize(&(network_data[i]));
        i += sizeof(Packet);

        switch (packet.packet_type) {
			case SERVER_SCENE_UPDATE:
				Scene * curScene = packet.s;
				//rendering variables
				clientScene->worldModels = curScene->worldModels;
				clientScene->tempWorldMarkers = curScene->tempWorldMarkers;
				clientScene->tempWorldLines = curScene->tempWorldLines;
				clientScene->volumeModels = curScene->volumeModels;
				clientScene->tempVolumeLines = curScene->tempVolumeLines;
				//other essential variables
				clientScene->removableMarkers = curScene->removableMarkers;
				clientScene->removableStraightLines = curScene->removableStraightLines;
				clientScene->removableCurvedLines = curScene->removableCurvedLines;

				break;

            default:

                printf("error in packet types\n");

                break;
        }
    }
}