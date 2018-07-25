#include "StdAfx.h"
#include "ServerManager.h"


unsigned int ServerManager::client_id; 

ServerManager::ServerManager(void)
{
    // id's to assign clients for our table
    client_id = 0;

	//initialize our serverside variables
	serverProxy = new Proxy();
	serverScene = new Scene(false);
	//default presentationMode is false; set the activeClient when we switch into presentationMode
	presentationMode = false;

    // set up the server network to listen 
    network = new ServerNetwork(); 
}

ServerManager::~ServerManager(void)
{
}

void ServerManager::update()
{
    // get new clients
   if(network->acceptNewClient(client_id))
   {
	   sendInitPacket(client_id);
       printf("client %d has been connected to the server\n",client_id);

       client_id++;
   }

   receiveFromClients();
}

void ServerManager::receiveFromClients()
{
	Packet packet;

    // go through all clients
    std::map<unsigned int, SOCKET>::iterator iter;

    for(iter = network->sessions.begin(); iter != network->sessions.end(); iter++)
    {
		int data_length = network->receiveData(iter->first, network_data);

        if (data_length <= 0) 
        {
            //no data recieved
            continue;
        }

        int i = 0;
        while (i < (unsigned int)data_length) 
        {
            /*packet.deserialize(&(network_data[i]));
            i += sizeof(Packet);*/

			packet.deserialize(&(network_data[i]));
			i += sizeof(Packet);

            switch (packet.packet_type) {

                case INIT_CONNECTION:

                    printf("server received init packet from client\n");

					sendSceneUpdate();
					sendPresentationMode();

					if (presentationMode) {
						sendProxyUpdate();
					}
                    //sendActionPackets();

                    break;

				case ADD_REMOVABLE:
					serverScene->AddRemovable(packet.m, packet.worldMode);
					sendSceneUpdate();
					break;

				case ADD_TEMP:
					serverScene->AddTemp(packet.m);
					sendSceneUpdate();
					break;

				case ADD_TEMP_LINE:
					serverScene->AddTempLine(packet.m, packet.worldMode);
					sendSceneUpdate();
					break;

				case ADD_REMOVABLE_MARKER:
					serverScene->AddRemovableMarker(packet.m, packet.worldMode);
					sendSceneUpdate();
					break;

				case ADD_REMOVABLE_STRAIGHT_LINE:
				{
					Vector3f start = (*packet.lineCore)[0];
					Vector3f end = (*packet.lineCore)[1];
					glm::quat handQ = (*packet.allHandQ)[0];

					serverScene->AddRemovableStraightLine(packet.m, start, end, handQ, packet.worldMode);
					sendSceneUpdate();
					break;
				}

				case ADD_REMOVABLE_CURVED_LINE:
					serverScene->AddRemovableCurvedLine(packet.m, *packet.lineCore, *packet.allHandQ, packet.worldMode);
					sendSceneUpdate();
					break;

				case REMOVE_MODEL:
					serverScene->RemoveModel(packet.m);
					sendSceneUpdate();
					break;

				case MOVE_TEMP_MODEL:
					serverScene->moveTempModel(packet.m, (*packet.lineCore)[0]);
					sendSceneUpdate();
					break;

				case REMOVE_TEMP_LINE:
					serverScene->removeTempLine(packet.m);
					sendSceneUpdate();
					break;

				case REMOVE_TEMP_MARKER:
					serverScene->removeTempMarker(packet.m);
					sendSceneUpdate();
					break;

				case CLIENT_PROXY_UPDATE:
					*(serverProxy) = *(packet.proxy);
					sendProxyUpdate();
					break;

				/*
				case STRING_PACKET:
				{
					int pt = packet.packet_type;
					//std::string str = *(packet.str);
					Model * m = packet.m;
					std::string str = m->S;
					int v = m->V[pt];

					printf("heyoi %s %i \n", str.c_str(), v);

					sendActionPackets();

					break;
				}

				case VECTOR_ADDITION:
				{
					(centralModel->V).push_back(packet.i);
					std::vector<int> v = centralModel->V;
					printf("added num %i, %i \n\n", v[v.size() - 1], v.size());

					sendActionPackets();

					break;
				}
				*/

                default:

                    printf("error in packet types\n");

                    break;
            }
        }
    }
}

//broadcast the newest client_id to the whole network, but only the newest-connected one
//which has not received an init packet will recognize and adopt it.
void ServerManager::sendInitPacket(int client_id)
{
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = SERVER_SCENE_UPDATE;
	packet.clientId = client_id;

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

void ServerManager::sendSceneUpdate()
{
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = SERVER_SCENE_UPDATE;
	packet.s = serverScene;

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}


void ServerManager::sendProxyUpdate()
{
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = SERVER_PROXY_UPDATE;
	packet.proxy = serverProxy;
	
	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}


void ServerManager::sendPresentationMode()
{
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = SERVER_PROXY_UPDATE;
	//repurpose the worldMode bool to send presentation mode
	packet.worldMode = presentationMode;

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}

/*
void ServerManager::sendActionPackets()
{
    // send action packet
    const unsigned int packet_size = sizeof(Packet);
    char packet_data[packet_size];

    Packet packet;
    packet.packet_type = ACTION_EVENT;

    packet.serialize(packet_data);

    network->sendToAll(packet_data,packet_size);
}


//fake string packet, just a tester
void ServerManager::sendStringPackets()
{
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = STRING_PACKET;

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}


void ServerManager::sendModelUpdate()
{
	const unsigned int packet_size = sizeof(Packet);
	char packet_data[packet_size];

	Packet packet;
	packet.packet_type = MODEL_UPDATE;
	packet.m = centralModel;

	packet.serialize(packet_data);

	network->sendToAll(packet_data, packet_size);
}
*/