#include "StdAfx.h"
#include "ClientManager.h"

#include <vector>

ClientManager::ClientManager(ClientType type, Proxy * p)
{
	
    network = new ClientNetwork();
	client_type = type;
	clientScene = new Scene();
	proxy = p;

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


//BASIC FUNCTION USED TO SEND EVERY PACKET AFTER CONSTRUCTION
void ClientManager::sendPacket(Packet packet) {
	const unsigned int packet_size = sizeof(packet);
	//IZ: This creates an appropriately-sized char array to later fill with the converted pointer
	char packet_data[packet_size];

	packet.serialize(packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}



/*************************************************
MESSAGE-SENDING PARALLELS TO SCENEMANAGER FUNCTIONS
**************************************************/
void ClientManager::AddRemovable(Model * m, bool worldMode)
{
	Packet packet;
	packet.packet_type = ADD_REMOVABLE;
	packet.m = m;
	packet.worldMode = worldMode;

	sendPacket(packet);
}

void ClientManager::AddRemovableMarker(Model * m, bool worldMode)
{
	Packet packet;
	packet.packet_type = ADD_REMOVABLE_MARKER;
	packet.m = m;
	packet.worldMode = worldMode;

	sendPacket(packet);
}

void ClientManager::AddTemp(Model * m)
{
	Packet packet;
	packet.packet_type = ADD_TEMP_MARKER;
	packet.m = m;

	sendPacket(packet);
}

void ClientManager::AddTempLine(Model * m)
{
	Packet packet;
	packet.packet_type = ADD_TEMP_LINE;
	packet.m = m;

	sendPacket(packet);
}

void ClientManager::AddRemovableStraightLine(Model * m, Vector3f start, Vector3f end, glm::quat handQ, bool worldMode)
{
	Packet packet;
	packet.packet_type = ADD_STRAIGHT_LINE;
	packet.m = m;

	//we need to make sure everything is a pointer
	std::vector<Vector3f> * lineCore;
	lineCore->push_back(start);
	lineCore->push_back(end);

	std::vector<glm::quat> * allHandQ;
	allHandQ->push_back(handQ);

	packet.lineCore = lineCore;
	packet.allHandQ = allHandQ;

	sendPacket(packet);
}

void ClientManager::AddRemovableCurvedLine(Model * m, std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, bool worldMode)
{
	Packet packet;
	packet.packet_type = ADD_CURVED_LINE;
	packet.m = m;

	std::vector<Vector3f> * core;
	core->assign(lineCore.begin(), lineCore.end());
	packet.lineCore = core;

	std::vector<glm::quat> * q;
	q->assign(allHandQ.begin(), allHandQ.end());
	packet.allHandQ = q;

	packet.worldMode = worldMode;

	sendPacket(packet);
}

void ClientManager::RemoveModel(Model * m)
{
	Packet packet;
	packet.packet_type = REMOVE_MODEL;
	packet.m = m;

	sendPacket(packet);
}




/*********************************************************
Dealing with actions (controller/touchscreen/keyboard)
**********************************************************/


void VRActions() {

}


void ClientManager::sendStringPackets(Model * m) {
	/*
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];
	*/

	Packet strPacket;
	strPacket.packet_type = STRING_PACKET;
	strPacket.m = m;

	const unsigned int packet_size = sizeof(strPacket);
	//IZ: This creates an appropriately-sized char array to later fill with the converted pointer
	char packet_data[packet_size];

	strPacket.serialize(packet_data);

	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
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
			case INIT_CONNECTION:

				printf("\n");

				//sendStringPackets(mPiece);
				addToModelVector(iter);

				break;

            default:

                printf("error in packet types\n");

                break;
        }
    }
}