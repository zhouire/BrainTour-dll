#pragma once

#include "stdafx.h"

#include "ClientNetwork.h"
//#include "shared.h"

//Message-sending version of Scene
struct ClientScene : public Scene
{
	ClientNetwork * network;

	//constructor
	ClientScene(bool client, ClientNetwork * n) : Scene(client)
	{
		network = n;
	}


	std::string serializeToChar(Packet * packet)
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

	Packet * deserializeToPacket(const char * buffer, int buflen)
	{
		Packet * packet;
		// wrap buffer inside a stream and deserialize serial_str into obj
		boost::iostreams::basic_array_source<char> device(buffer, buflen);
		boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(device);
		boost::archive::binary_iarchive ia(s);
		ia >> packet;

		return packet;
	}

	void sendSizeData(int packet_size) {
		::Size s;
		s.size = packet_size;

		const unsigned int s_size = sizeof(::Size);
		char s_data[s_size];

		s.serialize(s_data);

		//printf("client serializing size\n");

		NetworkServices::sendMessage(network->ConnectSocket, s_data, s_size);
	}



	//BASIC FUNCTION USED TO SEND EVERY PACKET AFTER CONSTRUCTION
	void sendPacket(Packet * packet) {
		std::string buffer = serializeToChar(packet);
		char * packet_data = (char*)(buffer.data());
		const unsigned int packet_size = buffer.size();

		sendSizeData(packet_size);
		NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
	}

	void AddRemovable(Model * m, bool worldMode) {
		Packet * packet = new Packet();
		packet->packet_type = ADD_REMOVABLE;
		packet->m = *m;
		packet->worldMode = worldMode;

		sendPacket(packet);
	}

	/*
	void AddTemp(Model * n)
	{
		//for client-side recordkeeping
		//localTempWorldMarkers.insert(std::pair<Model*, int>(n, 1));

		Packet * packet = new Packet();
		packet->packet_type = ADD_TEMP;
		packet->m = *n;

		sendPacket(packet);
	}
	*/

	void AddTempLine(Model * n, bool worldMode)
	{
		
		/*//for client-side recordkeeping
		if (worldMode) {
			localTempWorldLines.insert(std::pair<Model*, int>(n, 1));
		}

		else {
			localTempVolumeLines.insert(std::pair<Model*, int>(n, 1));
		}
		*/

		Packet * packet = new Packet();
		packet->packet_type = ADD_TEMP_LINE;
		packet->m = *n;
		packet->worldMode = worldMode;

		sendPacket(packet);
	}

	void AddRemovableMarker(Model * n, bool worldMode)
	{
		Packet * packet = new Packet();
		packet->packet_type = ADD_REMOVABLE_MARKER;
		packet->m = *n;
		packet->worldMode = worldMode;

		sendPacket(packet);
	}

	void AddRemovableStraightLine(Model * n, Vector3f start, Vector3f end, glm::quat handQ, bool worldMode)
	{
		Packet * packet = new Packet();
		packet->packet_type = ADD_REMOVABLE_STRAIGHT_LINE;
		packet->m = *n;
		packet->worldMode = worldMode;

		//we need to make sure everything is a pointer
		//also don't forget to convert the lineCore back to start, end when the Server receives it
		std::vector<Vector3f> lineCore;
		lineCore.push_back(start);
		lineCore.push_back(end);

		std::vector<glm::quat> allHandQ;
		allHandQ.push_back(handQ);

		packet->lineCore = lineCore;
		packet->allHandQ = allHandQ;

		sendPacket(packet);
	}

	void AddRemovableCurvedLine(Model * n, std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, bool worldMode)
	{
		Packet * packet = new Packet();
		packet->packet_type = ADD_REMOVABLE_CURVED_LINE;
		packet->m = *n;

		std::vector<Vector3f> core;
		core.assign(lineCore.begin(), lineCore.end());
		packet->lineCore = core;

		std::vector<glm::quat> q;
		q.assign(allHandQ.begin(), allHandQ.end());
		packet->allHandQ = q;

		packet->worldMode = worldMode;

		sendPacket(packet);
	}

	void RemoveModel(Model * n) {
		Packet * packet = new Packet();
		packet->packet_type = REMOVE_MODEL;
		//packet->m = *n;
		packet->clientId = n->client_creator;
		packet->modelId = n->id;

		sendPacket(packet);
	}

	/*
	void moveTempModel(Model * m, Vector3f newPos) {
		Packet * packet = new Packet();
		packet->packet_type = MOVE_TEMP_MODEL;
		//packet->m = *m;
		packet->clientId = m->client_creator;
		packet->modelId = m->id;

		std::vector<Vector3f> lineCore;
		lineCore.push_back(newPos);

		packet->lineCore = lineCore;

		sendPacket(packet);
	}
	*/

	void removeTempLine(Model * m) {
		//for local client record-keeping
		//localTempWorldLines.erase(m);
		//localTempVolumeLines.erase(m);

		Packet * packet = new Packet();
		packet->packet_type = REMOVE_TEMP_LINE;
		//packet->m = *m;
		packet->clientId = m->client_creator;
		packet->modelId = m->id;

		sendPacket(packet);
	}

	/*
	void removeTempMarker(Model * m) {
		//for local client record-keeping
		//localTempWorldMarkers.erase(m);

		Packet * packet = new Packet();
		packet->packet_type = REMOVE_TEMP_MARKER;
		//packet->m = *m;
		packet->clientId = m->client_creator;
		packet->modelId = m->id;

		sendPacket(packet);
	}
	*/

	//this function deletes every pointer key from a map of Model*, int
	//this is used for deallocating memory, especially when the Scene updates

	
	void deleteMapPointerKeys(std::map<Model*, int> & map, std::set<Model*> & deletedPtrs)
	{
		for (auto const m : map) {
			Model * keyCopy = m.first;
			map.erase(m.first);

			//if (keyCopy) {
			if ((deletedPtrs.insert(keyCopy)).second){
				delete keyCopy;
			}
		}
	}

	//this function takes a different type of map
	void deleteMapPointerKeys(std::map<Model*, LineComponents> & map, std::set<Model*> & deletedPtrs)
	{
		for (auto const m : map) {
			Model * keyCopy = m.first;
			map.erase(m.first);

			if ((deletedPtrs.insert(keyCopy)).second) {
				delete keyCopy;
			}
		}
	}

	//this function deletes all the pointers in the ModelPtrSet to effectively deallocate memory
	void deleteModelPtrs()
	{
		for (Model * m : ModelPtrSet) {
			delete m;
		}
		
		ModelPtrSet.clear();
	}


	//new version of targetModelRefresh that is called after add functions, when a new targetModel has been chosen
	void targetModelRefresh(Model * m) {
		if (!targetModelRefreshed) {
			if ((targetModelClient == m->client_creator) && (targetModelId == m->id)) {
				//RemoveModel function, not the message-sending version
				Scene::RemoveModel(targetModel);
				ModelPtrSet.erase(targetModel);
				delete targetModel;

				targetModel = m;
				targetModelRefreshed = true;
			}
		}
	}
	

	/*
	void deleteMapPointerKeys(std::map<Model*, int> & map)
	{
		for (auto & m : map) {
			if (m.first) {
				delete m.first;
			}
		}

		map.clear();
	}

	//this function takes a different type of map
	void deleteMapPointerKeys(std::map<Model*, LineComponents> & map)
	{
		for (auto & m : map) {
			delete m.first;
		}

		map.clear();
	}
	*/
	

};