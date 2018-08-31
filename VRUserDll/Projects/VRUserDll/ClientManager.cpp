#include "stdafx.h"
//#include "shared.h"

#define STB_IMAGE_IMPLEMENTATION

#include "ClientManager.h"

#include <vector>

/**********************************
this was at the head of VRUserDll.cpp
***************************************/
/*
template <class T>
inline T Clip(T d0, T d1, T d) {
	return std::min<T>(d1, std::max<T>(d0, d));
}
inline void DPrintf(const char *format, ...) {
	char buf[2048];
	va_list arg;
	va_start(arg, format);
	_vsnprintf_s(buf, sizeof(buf), format, arg);
	va_end(arg);
	printf("%s", buf); fflush(stdout);
	wchar_t wbuf[2048];
	wsprintf(wbuf, L"%S", buf);
	OutputDebugString(wbuf);
}

bool OnDown(unsigned int last, unsigned int current, unsigned int mask) {
	return (last&mask) == 0 && (current&mask);
}
*/

//Scene * tempScene = new Scene(true);
//ShaderFill * plainFill = tempScene->grid_material[0];

ClientManager::ClientManager(ClientType type)
{
    network = new ClientNetwork();
	client_type = type;

	clientProxy = new Proxy();

	clientScene = new ClientScene(true, network);

	//clientProxy = p;

	//auto-set to false, set to true when this is the active client during presentationMode
	//used during presentationMode; server sends a message setting this to true when a client
	//is defined as the active Client using its ID 
	init = false;
	
	active = false;
	presentationMode = false;

	curPacket = false;
	nextDataSize = sizeof(::Size);

    // send init packet
	
    Packet * packet = new Packet();
    packet->packet_type = INIT_CONNECTION;

	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	sendSizeData(packet_size);
	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);

	/*
	std::string buffer2 = serializeToChar(packet);
	char * packet_data2 = (char*)(buffer2.data());
	const unsigned int packet_size2 = buffer2.size();

	sendSizeData(packet_size2);
	NetworkServices::sendMessage(network->ConnectSocket, packet_data2, packet_size2);
	*/
}

ClientManager::~ClientManager(void)
{
}


std::string ClientManager::serializeToChar(Packet * packet)
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

Packet * ClientManager::deserializeToPacket(const char * buffer, int buflen)
{
	Packet * packet;
	// wrap buffer inside a stream and deserialize serial_str into obj
	boost::iostreams::basic_array_source<char> device(buffer, buflen);
	boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
	boost::archive::binary_iarchive ia(s);
	ia >> packet;

	return packet;
}

void ClientManager::sendSizeData(int packet_size) {
	::Size s;
	s.size = packet_size;

	const unsigned int s_size = sizeof(::Size);
	char s_data[s_size];

	s.serialize(s_data);

	//printf("client serializing size\n");

	NetworkServices::sendMessage(network->ConnectSocket, s_data, s_size);
}



void ClientManager::update()
{
    //Packet packet;
	//Packet * packet = new Packet();
	Packet * packet = nullptr;
	//Scene * curScene;
	::Size size;

    int data_length = network->receivePackets(network_data);

    if (data_length <= 0) 
    {
        //no data recieved
        return;
    }

    int i = 0;

	while (i < (unsigned int)data_length)
	{
		//this part deals with receiving data of varying sizes

		if (!curPacket) {
			//if the remaining data + data in tempBuf < enough data to construct Size object
			//just add the remaining data to the tempBuf, increment i, skip rest of loop
			if ((data_length - i + tempBuf.size()) < sizeof(::Size)) {
				//just append the rest of the data, add to i, and skip everything else
				tempBuf.append(&(network_data[i]), (data_length - i - 1));
				//add enough to overflow the while condition
				i += sizeof(::Size);

				continue;
			}

			//if we have enough data in the network_data and tempBuf, append enough data to tempBuf to
			//construct a Size object, deserialize tempBuf into Size size, use value in size to update
			//nextDataSize, clear tempBuf (because we have already deserialized), increment i, flip the 
			//curPacket switch (because we constructed a full object), and skip rest of loop
			else {
				//dynamic, so keep track of starting value for computations
				int s = tempBuf.size();

				tempBuf.append(&(network_data[i]), (sizeof(::Size) - s));
				size.deserialize((char*)(tempBuf.data()));
				nextDataSize = size.size;
				curPacket = !curPacket;

				i += (sizeof(::Size) - s);

				tempBuf.clear();

				continue;
			}
		}

		else {
			//if the remaining data + data in tempBuf < enough data to construct the next Packet
			//add remaining data to tempBuf, increment i, skip rest of loop
			if ((data_length - i + tempBuf.size()) < nextDataSize) {
				tempBuf.append(&(network_data[i]), (data_length - i - 1));
				i += nextDataSize;
				continue;
			}

			//if we have enough data to construct the next full Packet, append enough data to tempBuf to
			//construct the Packet, deserialize tempBuf into Packet packet, clear tempBuf, flip the 
			//curPacket switch, increment i, and MOVE ON to rest of loop
			else {
				int s = tempBuf.size();

				tempBuf.append(&(network_data[i]), (nextDataSize - s));
				packet = deserializeToPacket((tempBuf.data()), nextDataSize);
				curPacket = !curPacket;

				i += (nextDataSize - s);

				tempBuf.clear();
			}
		}

		Model * n = new Model(Vector3f(0, 0, 0), nullptr, -1, 0);
		*n = packet->m;

		switch (packet->packet_type) {
			case INIT_CONNECTION:
				if (!init) {
					clientId = packet->clientId;
					init = true;

					clientScene->client_id = clientId;
				}

				break;

			case SERVER_SCENE_UPDATE:
				/*
				curScene = packet.scene;
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
				*/

				updateClientSceneFromBasic(packet->scene);
				//clientScene->targetModelRefresh();

				break;

			case SERVER_PROXY_UPDATE:
				if (!active) {
					Proxy curProxy = packet->proxy;

					//clientProxy->Position[0] = curProxy->Position[0];
					//clientProxy->Position[1] = curProxy->Position[1];
					//clientProxy->Position[2] = curProxy->Position[2];

					clientProxy->PositionX = curProxy.PositionX;
					clientProxy->PositionY = curProxy.PositionY;
					clientProxy->PositionZ = curProxy.PositionZ;

					clientProxy->ClipMode = curProxy.ClipMode;
					clientProxy->ClipWidth = curProxy.ClipWidth;
					clientProxy->ClipPos = curProxy.ClipPos;

					clientProxy->Pose = curProxy.Pose;
					clientProxy->gPose = curProxy.gPose;
					clientProxy->gHeadPos = curProxy.gHeadPos;
					clientProxy->gHeadOrientation = curProxy.gHeadOrientation;

					clientProxy->scale = curProxy.scale;
					clientProxy->voxelSize = curProxy.voxelSize;

					clientProxy->view = curProxy.view;
					clientProxy->proj = curProxy.proj;
				}

				break;

			case STC_ADD_REMOVABLE:
				clientScene->Scene::AddRemovable(n, packet->worldMode);
				if (!clientScene->targetModelRefreshed) {
					clientScene->targetModelRefresh(n);
				}

				break;

			/*
			case STC_ADD_TEMP:
				clientScene->Scene::AddTemp(n);

				break;
			*/

			case STC_ADD_REMOVABLE_MARKER:
				clientScene->Scene::AddRemovableMarker(n, packet->worldMode);
				if (!clientScene->targetModelRefreshed) {
					clientScene->targetModelRefresh(n);
				}

				break;

			case STC_ADD_TEMP_LINE:
				clientScene->Scene::AddTempLine(n, packet->worldMode);

				break;

			case STC_ADD_REMOVABLE_STRAIGHT_LINE:
				clientScene->Scene::AddRemovableStraightLine(n, packet->lineCore[0], packet->lineCore[1], packet->allHandQ[0], packet->worldMode);
				if (!clientScene->targetModelRefreshed) {
					clientScene->targetModelRefresh(n);
				}

				break;

			case STC_ADD_REMOVABLE_CURVED_LINE:
				clientScene->Scene::AddRemovableCurvedLine(n, packet->lineCore, packet->allHandQ, packet->worldMode);
				if (!clientScene->targetModelRefreshed) {
					clientScene->targetModelRefresh(n);
				}

				break;

			case STC_REMOVE_MODEL:
			{
				Model * rm = nullptr;
				for (Model* m : clientScene->ModelPtrSet) {
					if ((m->client_creator == packet->clientId) && (m->id == packet->modelId)) {
						rm = m;
						break;
					}
				}

				clientScene->Scene::RemoveModel(rm);

				break;
			}

			/*
			case STC_MOVE_TEMP_MODEL:
			{
				Model * rm = nullptr;
				for (auto m : clientScene->tempWorldMarkers) {
					if (((m.first)->client_creator == packet->clientId) && ((m.first)->id == packet->modelId)) {
						rm = m.first;
						break;
					}
				}

				clientScene->Scene::moveTempModel(rm, packet->lineCore[0]);

				break;
			}
			*/

			case STC_REMOVE_TEMP_LINE:
			{
				Model * rm = nullptr;

				for (auto m : clientScene->tempWorldLines) {
					if (((m.first)->client_creator == packet->clientId) && ((m.first)->id == packet->modelId)) {
						rm = m.first;
						break;
					}
				}

				if (!rm) {
					for (auto m : clientScene->tempVolumeLines) {
						if (((m.first)->client_creator == packet->clientId) && ((m.first)->id == packet->modelId)) {
							rm = m.first;
							break;
						}
					}
				}

				clientScene->Scene::removeTempLine(rm);

				break;
			}

			/*
			case STC_REMOVE_TEMP_MARKER:
			{
				Model * rm = nullptr;
				for (auto m : clientScene->tempWorldMarkers) {
					if (((m.first)->client_creator == packet->clientId) && ((m.first)->id == packet->modelId)) {
						rm = m.first;
						break;
					}
				}

				clientScene->Scene::removeTempMarker(rm);

				break;
			}
			*/



			default:

				printf("error in packet types\n");

				break;
		}
	}

	//clientScene->targetModelRefresh();
}


void ClientManager::sendClientProxyUpdate() {
	Packet * packet = new Packet();
	packet->packet_type = CLIENT_PROXY_UPDATE;
	packet->proxy = *clientProxy;

	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	sendSizeData(packet_size);
	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}


void ClientManager::sendClientDebug(unsigned int i) {
	Packet * packet = new Packet();
	packet->packet_type = CLIENT_DEBUG;
	packet->clientId = i;

	std::string buffer = serializeToChar(packet);
	char * packet_data = (char*)(buffer.data());
	const unsigned int packet_size = buffer.size();

	sendSizeData(packet_size);
	NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
}


BasicScene ClientManager::convertClientSceneToBasic(ClientScene c) {
	BasicScene basic;

	basic.ModelPtrSet = c.ModelPtrSet;

	basic.worldModels = c.worldModels;
	//basic.tempWorldMarkers = c.tempWorldMarkers;
	basic.tempWorldLines = c.tempWorldLines;
	basic.volumeModels = c.volumeModels;
	basic.tempVolumeLines = c.tempVolumeLines;
	basic.removableMarkers = c.removableMarkers;
	basic.removableStraightLines = c.removableStraightLines;
	basic.removableCurvedLines = c.removableCurvedLines;

	return basic;
}

void ClientManager::updateClientSceneFromBasic(BasicScene b) {
	//std::set<Model*> deleted;

	//clientScene->deleteMapPointerKeys(clientScene->worldModels, deleted);
	clientScene->worldModels = b.worldModels;
	//clientScene->deleteMapPointerKeys(b.worldModels);

	//clientScene->deleteMapPointerKeys(clientScene->tempWorldMarkers, deleted);
	//clientScene->tempWorldMarkers = b.tempWorldMarkers;
	//clientScene->deleteMapPointerKeys(b.tempWorldMarkers);
	
	//clientScene->deleteMapPointerKeys(clientScene->tempWorldLines, deleted);
	clientScene->tempWorldLines = b.tempWorldLines;
	//clientScene->deleteMapPointerKeys(b.tempWorldLines);

	//clientScene->deleteMapPointerKeys(clientScene->volumeModels, deleted);
	clientScene->volumeModels = b.volumeModels;
	//clientScene->deleteMapPointerKeys(b.volumeModels);

	//clientScene->deleteMapPointerKeys(clientScene->tempVolumeLines, deleted);
	clientScene->tempVolumeLines = b.tempVolumeLines;
	//clientScene->deleteMapPointerKeys(b.tempVolumeLines);

	//clientScene->deleteMapPointerKeys(clientScene->removableMarkers, deleted);
	clientScene->removableMarkers = b.removableMarkers;
	//clientScene->deleteMapPointerKeys(b.removableMarkers);

	//clientScene->deleteMapPointerKeys(clientScene->removableStraightLines, deleted);
	clientScene->removableStraightLines = b.removableStraightLines;
	//clientScene->deleteMapPointerKeys(b.removableStraightLines);

	//clientScene->deleteMapPointerKeys(clientScene->removableCurvedLines, deleted);
	clientScene->removableCurvedLines = b.removableCurvedLines;
	//clientScene->deleteMapPointerKeys(b.removableCurvedLines);

	clientScene->deleteModelPtrs();
	clientScene->ModelPtrSet = b.ModelPtrSet;
}



