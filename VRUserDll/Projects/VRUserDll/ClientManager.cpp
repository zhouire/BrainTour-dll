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
	Packet * packet = new Packet();
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

            default:

                printf("error in packet types\n");

                break;
        }
    }
}


/*
unsigned int lastButtons = 0;
ovrPosef lastHandPoses[2];

void ClientManager::controllerUpdate(Proxy * p, ovrTrackingState trackState, ovrInputState inputState, int & RoiMode)
{	
	ovrPosef         handPoses[2];
	handPoses[ovrHand_Left] = trackState.HandPoses[ovrHand_Left].ThePose;
	handPoses[ovrHand_Right] = trackState.HandPoses[ovrHand_Right].ThePose;

	//head pose; currently unused
	ovrPosef hmdPose = trackState.HeadPose.ThePose;


	//int RoiMode = 0;
	Matrix4f v = clientProxy->view;
	v.M[0][3] = 0.f;
	v.M[1][3] = 0.f;
	v.M[2][3] = 0.f;
	v.M[3][3] = 1.f;
	v.M[3][0] = 0.f;
	v.M[3][1] = 0.f;
	v.M[3][2] = 0.f;

	clientScene->moveTempModels(handPoses[ovrHand_Right], clientProxy->gPose, clientProxy->gHeadPos, clientProxy->gHeadOrientation, clientProxy->view);
	
	
	// left trigger
	if (inputState.IndexTrigger[ovrHand_Left] > 0.5f) {
		RoiMode = 1;
	}

	// Left Stick is for Volume Clip
	if (clientProxy->ClipMode) {
		if (inputState.Buttons & ovrButton_LThumb) {
			float speed = 0.02f;
			clientProxy->ClipWidth +=
				inputState.Thumbstick[ovrHand_Left].x*speed +
				inputState.Thumbstick[ovrHand_Left].y*speed;
			clientProxy->ClipWidth = Clip<float>(0.01f, 1.0f, clientProxy->ClipWidth);
		}
		else {
			float speed = 0.02f;
			clientProxy->ClipPos +=
				inputState.Thumbstick[ovrHand_Left].x*speed +
				inputState.Thumbstick[ovrHand_Left].y*speed;
			clientProxy->ClipPos = Clip<float>(0.0f, 1.0f, clientProxy->ClipPos);
		}
	}
	
	// Index Trigger + Rotation (ROI Rotation)
#if 0
	if (inputState.IndexTrigger[ovrHand_Left]>0.5f) {
		Quatf q1(handPoses[ovrHand_Left].Orientation);
		Quatf q0(lastHandPoses[ovrHand_Left].Orientation);
		Matrix4f Pose0 = Pose;
		//Pose = Pose * Matrix4f(q1).Inverted() * Matrix4f(q0);
		Pose = Matrix4f(q0) * Matrix4f(q1).Inverted() * Pose;
		Vector3f center = gHeadPos + Vector3f(0.f, 0.f, -(RoiDepth + RoiRange * 0.5f)*gObjectScale);
		Vector3f p = -(Pose.Transform(center) - Pose0.Transform(center));
		Position[0] += p.x;
		Position[1] += p.y;
		Position[2] += p.z;
	}
#endif

	// Hand Trigger + Rotation (Object Rot)
	else if (inputState.HandTrigger[ovrHand_Left]>0.5f) {
		Quatf q0(handPoses[ovrHand_Left].Orientation);
		Quatf q1(lastHandPoses[ovrHand_Left].Orientation);
		//gPose = gPose * q1.Inverse() * q0;
		clientProxy->gPose = q0 * q1.Inverse() * clientProxy->gPose;
	}


	if (OnDown(lastButtons, inputState.Buttons, ovrButton_X))
	{
		if (!(inputState.Buttons & ovrButton_A)) {
			// Handle A button down
			clientProxy->ClipMode = (clientProxy->ClipMode + 1) % 4;
		}
	}



	//Controller actions influencing the scene (A,B,X,Y)
	clientScene->ControllerActions(handPoses[ovrHand_Left], handPoses[ovrHand_Right], clientProxy->gPose, clientProxy->gHeadPos, inputState, clientProxy->gHeadOrientation, 
		clientProxy->view, clientProxy->scale, clientProxy->voxelSize);

	//R Thumb Pressed

	if (inputState.Buttons & ovrButton_RThumb) {
		// Zoom
		DPrintf(" StickZoom\n");
		float speed = -0.8f;
		float forward = inputState.Thumbstick[ovrHand_Right].y*speed;

		Vector3f movement = v.Inverted().Transform(Vector3f(0.0f, 0.0f, forward));
		//clientProxy->Position[0] += movement.x;
		//clientProxy->Position[1] += movement.y;
		//clientProxy->Position[2] += movement.z;
		clientProxy->PositionX += movement.x;
		clientProxy->PositionY += movement.y;
		clientProxy->PositionZ += movement.z;
	}

	//R Thumb stick
	else {
		DPrintf(" RThumb\n");
		// Translation
		float speed = 0.1f;
		float mx = inputState.Thumbstick[ovrHand_Right].x*speed;
		float my = inputState.Thumbstick[ovrHand_Right].y*speed;
		Vector3f movement = v.Inverted().Transform(Vector3f(mx, my, 0.f));
		//clientProxy->Position[0] += movement.x;
		//clientProxy->Position[1] += movement.y;
		//clientProxy->Position[2] += movement.z;
		clientProxy->PositionX += movement.x;
		clientProxy->PositionY += movement.y;
		clientProxy->PositionZ += movement.z;
	}



	// R Hand Trigger for MotionTracking
	if (inputState.HandTrigger[ovrHand_Right] > 0.1f) {
		DPrintf(" R Tracking\n");
		// translation
		//static float positionTrackingSpeed = option::Option("PositionTrackingSpeed", 6.0f);
		static float positionTrackingSpeed = 6.0f;
		ovrPosef pose1 = handPoses[ovrHand_Right];
		ovrPosef pose0 = lastHandPoses[ovrHand_Right];
		Vector3f movement = v.Inverted().Transform(Vector3f(pose1.Position) - Vector3f(pose0.Position)) * -(positionTrackingSpeed*inputState.HandTrigger[ovrHand_Right]);
		//clientProxy->Position[0] += movement.x;
		//clientProxy->Position[1] += movement.y;
		//clientProxy->Position[2] += movement.z;
		clientProxy->PositionX += movement.x;
		clientProxy->PositionY += movement.y;
		clientProxy->PositionZ += movement.z;
		//DPrintf("Motion: %.4f, %.4f, %.4f\n", movement.x, movement.y, movement.z);
		// rotation
		Quatf q0(pose0.Orientation);
		Quatf q1(pose1.Orientation);
		clientProxy->Pose = clientProxy->Pose * Matrix4f(q0) * Matrix4f(q1).Transposed();
	}

	//lastLeftHandTrigger = inputState.HandTrigger[ovrHand_Left];
	lastButtons = inputState.Buttons;
	lastHandPoses[ovrHand_Left] = handPoses[ovrHand_Left];
	lastHandPoses[ovrHand_Right] = handPoses[ovrHand_Right];

	if (presentationMode) {
		sendClientProxyUpdate();
	}
}
*/


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

	basic.worldModels = c.worldModels;
	basic.tempWorldMarkers = c.tempWorldMarkers;
	basic.tempWorldLines = c.tempWorldLines;
	basic.volumeModels = c.volumeModels;
	basic.tempVolumeLines = c.tempVolumeLines;
	basic.removableMarkers = c.removableMarkers;
	basic.removableStraightLines = c.removableStraightLines;
	basic.removableCurvedLines = c.removableCurvedLines;

	return basic;
}

void ClientManager::updateClientSceneFromBasic(BasicScene b) {
	clientScene->worldModels = b.worldModels;
	clientScene->tempWorldMarkers = b.tempWorldMarkers;
	clientScene->tempWorldLines = b.tempWorldLines;
	clientScene->volumeModels = b.volumeModels;
	clientScene->tempVolumeLines = b.tempVolumeLines;
	clientScene->removableMarkers = b.removableMarkers;
	clientScene->removableStraightLines = b.removableStraightLines;
	clientScene->removableCurvedLines = b.removableCurvedLines;
}