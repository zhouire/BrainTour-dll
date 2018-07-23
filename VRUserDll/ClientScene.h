#pragma once

//Message-sending version of Scene
struct ClientScene : public Scene
{
	ClientNetwork * network;

	//constructor
	ClientScene(bool client, ClientNetwork * n) : Scene(client)
	{
		network = n;
	}

	//BASIC FUNCTION USED TO SEND EVERY PACKET AFTER CONSTRUCTION
	void sendPacket(Packet packet) {
		const unsigned int packet_size = sizeof(packet);
		//IZ: This creates an appropriately-sized char array to later fill with the converted pointer
		char packet_data[packet_size];

		packet.serialize(packet_data);

		NetworkServices::sendMessage(network->ConnectSocket, packet_data, packet_size);
	}

	void AddRemovable(Model * m, bool worldMode) {
		Packet packet;
		packet.packet_type = ADD_REMOVABLE;
		packet.m = m;
		packet.worldMode = worldMode;

		sendPacket(packet);
	}

	void AddTemp(Model * n)
	{
		//for client-side recordkeeping
		localTempWorldMarkers.insert(std::pair<Model*, int>(n, 1));

		Packet packet;
		packet.packet_type = ADD_TEMP;
		packet.m = n;

		sendPacket(packet);
	}

	void AddTempLine(Model * n, bool worldMode)
	{
		//for client-side recordkeeping
		if (worldMode) {
			localTempWorldLines.insert(std::pair<Model*, int>(n, 1));
		}

		else {
			localTempVolumeLines.insert(std::pair<Model*, int>(n, 1));
		}

		Packet packet;
		packet.packet_type = ADD_TEMP_LINE;
		packet.m = n;
		packet.worldMode = worldMode;

		sendPacket(packet);
	}

	void AddRemovableMarker(Model * n, bool worldMode)
	{
		Packet packet;
		packet.packet_type = ADD_REMOVABLE_MARKER;
		packet.m = n;
		packet.worldMode = worldMode;

		sendPacket(packet);
	}

	void AddRemovableStraightLine(Model * n, Vector3f start, Vector3f end, glm::quat handQ, bool worldMode)
	{
		Packet packet;
		packet.packet_type = ADD_REMOVABLE_STRAIGHT_LINE;
		packet.m = n;

		//we need to make sure everything is a pointer
		//also don't forget to convert the lineCore back to start, end when the Server receives it
		std::vector<Vector3f> * lineCore;
		lineCore->push_back(start);
		lineCore->push_back(end);

		std::vector<glm::quat> * allHandQ;
		allHandQ->push_back(handQ);

		packet.lineCore = lineCore;
		packet.allHandQ = allHandQ;

		sendPacket(packet);
	}

	void AddRemovableCurvedLine(Model * n, std::vector<Vector3f> lineCore, std::vector<glm::quat> allHandQ, bool worldMode)
	{
		Packet packet;
		packet.packet_type = ADD_REMOVABLE_CURVED_LINE;
		packet.m = n;

		std::vector<Vector3f> * core;
		core->assign(lineCore.begin(), lineCore.end());
		packet.lineCore = core;

		std::vector<glm::quat> * q;
		q->assign(allHandQ.begin(), allHandQ.end());
		packet.allHandQ = q;

		packet.worldMode = worldMode;

		sendPacket(packet);
	}

	void RemoveModel(Model * n) {
		Packet packet;
		packet.packet_type = REMOVE_MODEL;
		packet.m = n;

		sendPacket(packet);
	}

	void moveTempModel(Model * m, Vector3f newPos) {
		Packet packet;
		packet.packet_type = MOVE_TEMP_MODEL;
		packet.m = m;
		packet.pos = newPos;

		sendPacket(packet);
	}

	void removeTempLine(Model * m) {
		Packet packet;
		packet.packet_type = REMOVE_TEMP_LINE;
		packet.m = m;

		sendPacket(packet);
	}

	void removeTempMarker(Model * m) {
		Packet packet;
		packet.packet_type = REMOVE_TEMP_MARKER;
		packet.m = m;

		sendPacket(packet);
	}

};