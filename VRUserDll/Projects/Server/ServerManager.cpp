#include "StdAfx.h"
//#include "shared.h"
#include "ServerManager.h"


//#include <boost/serialization/export.hpp>
//BOOST_CLASS_EXPORT_GUID(ShaderFill, "ShaderFill")


unsigned int ServerManager::client_id; 

//ServerManager::ServerManager(void)
ServerManager::ServerManager()
{
	// id's to assign clients for our table
    client_id = 0;
	curPacket = new bool;
	nextDataSize = new int;

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


std::string ServerManager::serializeToChar(Packet * packet)
{
	// serialize obj into an std::string
	std::string serial_str;
	boost::iostreams::back_insert_device<std::string> inserter(serial_str);
	boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
	boost::archive::binary_oarchive oa(s);

	oa << packet; //serializable object

				  // don't forget to flush the stream to finish writing into the buffer
	s.flush();

	return serial_str;
}


Packet * ServerManager::deserializeToPacket(const char * buffer, int buflen)
{
	Packet * packet;
	// wrap buffer inside a stream and deserialize serial_str into obj
	boost::iostreams::basic_array_source<char> device(buffer, buflen);
	boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
	boost::archive::binary_iarchive ia(s);
	ia >> packet;

	return packet;
}

void ServerManager::sendSizeData(int packet_size) {
	::Size s;
	s.size = packet_size;

	const unsigned int s_size = sizeof(::Size);
	char s_data[s_size];

	s.serialize(s_data);

	//printf("client serializing size\n");

	network->sendToAll(s_data, s_size);
}


void ServerManager::update()
{
    // get new clients
   if(network->acceptNewClient(client_id))
   {
	   sendInitPacket(client_id);
       printf("client %d has been connected to the server\n",client_id);

	   curPacket[client_id] = false;
	   nextDataSize[client_id] = sizeof(::Size);
	   tempBuf.push_back("");

       client_id++;
   }

   if (client_id > 0) {
	   receiveFromClients();
   }
}



void ServerManager::receiveFromClients()
{
	Packet * packet = new Packet();
	::Size size;


	// go through all clients
	for (auto iter = network->sessions.begin(); iter != network->sessions.end(); /* not hoisted; no inc.*/)
	{
		bool client_exit = false;

		int data_length = network->receiveData(iter->first, network_data);

		if (data_length <= 0)
		{
			//no data recieved
			continue;
		}

		printf("%i\n", data_length);

		int i = 0;
		while (i < (unsigned int)data_length)
		{
			//this part deals with receiving data of varying sizes

			if (!(curPacket[iter->first])) {
				//if the remaining data + data in tempBuf < enough data to construct Size object
				//just add the remaining data to the tempBuf, increment i, skip rest of loop
				if ((data_length - i + tempBuf[iter->first].size()) < sizeof(::Size)) {
					//just append the rest of the data, add to i, and skip everything else
					tempBuf[iter->first].append(&(network_data[i]), (data_length - i - 1));
					//add enough to overflow the while condition
					i += sizeof(::Size);

					continue;
				}

				//if we have enough data in the network_data and tempBuf, append enough data to tempBuf to
				//construct a Size object, deserialize tempBuf into Size size, use value in size to update
				//nextDataSize, clear tempBuf (because we have already deserialized), increment i, flip the 
				//curPacket switch (because we constructed a full object), and skip rest of loop
				else {
					int s = tempBuf[iter->first].size();

					tempBuf[iter->first].append(&(network_data[i]), (sizeof(::Size) - s));
					size.deserialize((char*)(tempBuf[iter->first].data()));
					nextDataSize[iter->first] = size.size;
					curPacket[iter->first] = true;

					i += (sizeof(::Size) - s);

					tempBuf[iter->first].clear();

					continue;
				}
			}
			
			else {
				//if the remaining data + data in tempBuf < enough data to construct the next Packet
				//add remaining data to tempBuf, increment i, skip rest of loop
				if ((data_length - i + tempBuf[iter->first].size()) < nextDataSize[iter->first]) {
					tempBuf[iter->first].append(&(network_data[i]), (data_length - i - 1));
					i += nextDataSize[iter->first];
					continue;
				}

				//if we have enough data to construct the next full Packet, append enough data to tempBuf to
				//construct the Packet, deserialize tempBuf into Packet packet, clear tempBuf, flip the 
				//curPacket switch, increment i, and MOVE ON to rest of loop
				
				else {
					int s = tempBuf[iter->first].size();

					tempBuf[iter->first].append(&(network_data[i]), (nextDataSize[iter->first] - s));
					printf("size is %i \n", nextDataSize[iter->first]);
					packet = deserializeToPacket((tempBuf[iter->first].data()), (nextDataSize[iter->first]));
					curPacket[iter->first] = false;

					i += (nextDataSize[iter->first] - s);

					tempBuf[iter->first].clear();
				}
				
			}
			
			
			Model * n = new Model(Vector3f(0, 0, 0), nullptr);
			*n = packet->m;

            switch (packet->packet_type) {

                case INIT_CONNECTION:

                    printf("server received init packet from client\n");

					//sendInitPacket(client_id - 1);

					sendSceneUpdate();
					//sendPresentationMode();

					if (presentationMode) {
						sendProxyUpdate();
					}

                    break;

				case ADD_REMOVABLE:
					printf("%i : server received add removable\n", iter->first);
					serverScene->AddRemovable(n, packet->worldMode);
					sendSceneUpdate();
					break;

				case ADD_TEMP:
					printf("%i : server received add temp\n", iter->first);
					serverScene->AddTemp(n);
					sendSceneUpdate();
					break;

				case ADD_TEMP_LINE:
					printf("%i : server received add temp line\n", iter->first);
					serverScene->AddTempLine(n, packet->worldMode);
					sendSceneUpdate();
					break;

				case ADD_REMOVABLE_MARKER:
					printf("%i : server received add removable marker\n", iter->first);
					serverScene->AddRemovableMarker(n, packet->worldMode);
					sendSceneUpdate();
					break;

				case ADD_REMOVABLE_STRAIGHT_LINE:
				{
					printf("%i : server received add removable straight line\n", iter->first);

					Vector3f start = (packet->lineCore)[0];
					Vector3f end = (packet->lineCore)[1];
					glm::quat handQ = (packet->allHandQ)[0];

					serverScene->AddRemovableStraightLine(n, start, end, handQ, packet->worldMode);
					sendSceneUpdate();
					break;
				}

				case ADD_REMOVABLE_CURVED_LINE:
					printf("%i : server received add removable curved line\n", iter->first);
					serverScene->AddRemovableCurvedLine(n, packet->lineCore, packet->allHandQ, packet->worldMode);
					sendSceneUpdate();
					break;

				case REMOVE_MODEL:
					printf("%i : server received remove model\n", iter->first);
					serverScene->RemoveModel(n);
					sendSceneUpdate();
					break;

				case MOVE_TEMP_MODEL:
					printf("%i : server received move temp model\n", iter->first);
					serverScene->moveTempModel(n, (packet->lineCore)[0]);
					sendSceneUpdate();
					break;

				case REMOVE_TEMP_LINE:
					printf("%i : server received remove temp line\n", iter->first);
					serverScene->removeTempLine(n);
					sendSceneUpdate();
					break;

				case REMOVE_TEMP_MARKER:
					printf("%i : server received remove temp marker\n", iter->first);
					serverScene->removeTempMarker(n);
					sendSceneUpdate();
					break;

				case CLIENT_PROXY_UPDATE:
					printf("%i : server received client proxy update\n", iter->first);
					*(serverProxy) = packet->proxy;
					sendProxyUpdate();
					break;

				case CLIENT_DEBUG:
					printf("%i : client debug %i\n", iter->first, packet->clientId);

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
	//const unsigned int packet_size = sizeof(Packet);
	//char packet_data[packet_size];

	Packet * packet = new Packet();
	packet->packet_type = INIT_CONNECTION;
	packet->clientId = client_id;

	//packet.serialize(packet_data);
	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	printf("init packet size: %i \n", packet_size);

	sendSizeData(packet_size);
	network->sendToAll(packet_data, packet_size);
}

void ServerManager::sendSceneUpdate()
{
	//const unsigned int packet_size = sizeof(Packet);
	//char packet_data[packet_size];

	Packet * packet = new Packet();
	packet->packet_type = SERVER_SCENE_UPDATE;
	packet->scene = convertServerSceneToBasic(*serverScene);

	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	sendSizeData(packet_size);
	network->sendToAll(packet_data, packet_size);
}


void ServerManager::sendProxyUpdate()
{
	//const unsigned int packet_size = sizeof(Packet);
	//char packet_data[packet_size];

	Packet * packet = new Packet();
	packet->packet_type = SERVER_PROXY_UPDATE;
	packet->proxy = *serverProxy;
	
	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	sendSizeData(packet_size);
	network->sendToAll(packet_data, packet_size);
}


void ServerManager::sendPresentationMode()
{
	//const unsigned int packet_size = sizeof(Packet);
	//char packet_data[packet_size];

	Packet * packet = new Packet();
	packet->packet_type = SERVER_PRESENTATION_MODE;
	//repurpose the worldMode bool to send presentation mode
	packet->worldMode = presentationMode;

	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	sendSizeData(packet_size);
	network->sendToAll(packet_data, packet_size);
}


BasicScene ServerManager::convertServerSceneToBasic(Scene s) 
{
	BasicScene basic;

	basic.worldModels = s.worldModels;
	basic.tempWorldMarkers = s.tempWorldMarkers;
	basic.tempWorldLines = s.tempWorldLines;
	basic.volumeModels = s.volumeModels;
	basic.tempVolumeLines = s.tempVolumeLines;
	basic.removableMarkers = s.removableMarkers;
	basic.removableStraightLines = s.removableStraightLines;
	basic.removableCurvedLines = s.removableCurvedLines;

	return basic;
}


void ServerManager::updateServerSceneFromBasic(BasicScene b)
{
	serverScene->worldModels = b.worldModels;
	serverScene->tempWorldMarkers = b.tempWorldMarkers;
	serverScene->tempWorldLines = b.tempWorldLines;
	serverScene->volumeModels = b.volumeModels;
	serverScene->tempVolumeLines = b.tempVolumeLines;
	serverScene->removableMarkers = b.removableMarkers;
	serverScene->removableStraightLines = b.removableStraightLines;
	serverScene->removableCurvedLines = b.removableCurvedLines;
}