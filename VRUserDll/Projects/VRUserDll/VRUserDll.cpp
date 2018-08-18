// VRUserDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

//#define STB_IMAGE_IMPLEMENTATON

#include "VRUserDll.hpp"
//#include "SceneManager.h"
#include "ClientManager.h"

//#include <process.h>

#include <algorithm>
#include <vector>

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

static void drawCube(float size)
{
	float h = size * 0.5f;
	glBegin(GL_QUADS);
	glVertex3f(-h, -h, -h);
	glVertex3f(-h, +h, -h);
	glVertex3f(+h, +h, -h);
	glVertex3f(+h, -h, -h);
	glVertex3f(-h, -h, +h);
	glVertex3f(-h, +h, +h);
	glVertex3f(+h, +h, +h);
	glVertex3f(+h, -h, +h);
	glVertex3f(-h, -h, -h);
	glVertex3f(-h, +h, -h);
	glVertex3f(-h, +h, +h);
	glVertex3f(-h, -h, +h);
	glVertex3f(+h, -h, -h);
	glVertex3f(+h, +h, -h);
	glVertex3f(+h, +h, +h);
	glVertex3f(+h, -h, +h);
	glVertex3f(-h, -h, -h);
	glVertex3f(-h, -h, +h);
	glVertex3f(+h, -h, +h);
	glVertex3f(+h, -h, -h);
	glVertex3f(-h, +h, -h);
	glVertex3f(-h, +h, +h);
	glVertex3f(+h, +h, +h);
	glVertex3f(+h, +h, -h);
	glEnd();
}



//static Scene * roomScene = nullptr;
static ClientManager * VRclient = nullptr;


namespace VRUserProxy {
	APIBundle *proxy;
	VRUSERDLL_API int OnInit(APIBundle *p, OVR::GLEContext *context) {
		proxy = p;
		OVR::GLEContext::SetCurrentContext(context);

		VRclient = new ClientManager(VR);

		return 0;
	}

	bool OnDown(unsigned int last, unsigned int current, unsigned int mask) {
		return (last&mask) == 0 && (current&mask);
	}

	unsigned int lastButtons = 0;
	ovrPosef lastHandPoses[2];
	VRUSERDLL_API void OnHandleTouch(void *recv, ovrTrackingState& trackState, ovrInputState &inputState, OVR::Matrix4f& view) {
		using namespace OVR;

		//DPrintf("VRUserProxy::OnHadleTouch\n");

		float *Position = proxy->GetPosition(recv);
		int &ClipMode = proxy->GetClipMode(recv);
		float &ClipWidth = proxy->GetClipWidth(recv);
		float &ClipPos = proxy->GetClipPos(recv);
		Matrix4f &Pose = proxy->GetPose(recv);
		Quatf &gPose = proxy->GetGlobalPose(recv);
		Vector3f &gHeadPos = proxy->GetGlobalHeadPosition(recv);
		Matrix4f &gHeadOrientation = proxy->GetGlobalHeadPose(recv);

		/*
		ovrPosef         handPoses[2];
		handPoses[ovrHand_Left] = trackState.HandPoses[ovrHand_Left].ThePose;
		handPoses[ovrHand_Right] = trackState.HandPoses[ovrHand_Right].ThePose;

		//head pose; currently unused
		ovrPosef hmdPose = trackState.HeadPose.ThePose;
		*/

		int maxLen = proxy->VolumeSize[0];
		if (maxLen < proxy->VolumeSize[1]) maxLen = proxy->VolumeSize[1];
		if (maxLen < proxy->VolumeSize[2]) maxLen = proxy->VolumeSize[2];
		float scale = proxy->ObjectScale / maxLen;

		Vector3f voxelSize;
		voxelSize.x = proxy->VoxelSize[0];
		voxelSize.y = proxy->VoxelSize[1];
		voxelSize.z = proxy->VoxelSize[2];

		//Proxy * p = new Proxy();
		(VRclient->clientProxy)->PositionX = Position[0];
		(VRclient->clientProxy)->PositionY = Position[1];
		(VRclient->clientProxy)->PositionZ = Position[2];

		(VRclient->clientProxy)->ClipMode = ClipMode;
		(VRclient->clientProxy)->ClipWidth = ClipWidth;
		(VRclient->clientProxy)->ClipPos = ClipPos;

		(VRclient->clientProxy)->Pose = Pose;
		(VRclient->clientProxy)->gPose = gPose;
		(VRclient->clientProxy)->gHeadPos = gHeadPos;
		(VRclient->clientProxy)->gHeadOrientation = gHeadOrientation;

		(VRclient->clientProxy)->scale = scale;
		(VRclient->clientProxy)->voxelSize = voxelSize;

		(VRclient->clientProxy)->view = view;

		//(VRclient->clientProxy) = p;

		
		DPrintf("VRUserProxy::GetInputState OK\n");
		int RoiMode = 0;

		VRclient->update();
		VRclient->controllerUpdate(VRclient->clientProxy, trackState, inputState, RoiMode);

		//updating the DLL proxy with the clientProxy
		Position[0] = (VRclient->clientProxy)->PositionX;
		Position[1] = (VRclient->clientProxy)->PositionY;
		Position[2] = (VRclient->clientProxy)->PositionZ;

		ClipMode = (VRclient->clientProxy)->ClipMode;
		ClipWidth = (VRclient->clientProxy)->ClipWidth;
		ClipPos = (VRclient->clientProxy)->ClipPos;

		Pose = (VRclient->clientProxy)->Pose;
		gPose = (VRclient->clientProxy)->gPose;
		gHeadPos = (VRclient->clientProxy)->gHeadPos;
		gHeadOrientation = (VRclient->clientProxy)->gHeadOrientation;


		// update Roi
		if (RoiMode == 0) {
			proxy->SetRoI(0, 0.0f);
			proxy->SetRoI(1, 1000000.0f);
			proxy->SetRoI(2, 1.0f);
		}
		else {
			float RoiDepth = 1024.0f;
			float RoiRange = 512.0f;
			proxy->SetRoI(0, RoiDepth);
			proxy->SetRoI(1, RoiDepth+RoiRange);
			proxy->SetRoI(2, 0.1f);
		}
	}



	VRUSERDLL_API void DrawPreVolumeRendering(OVR::Matrix4f& view, OVR::Matrix4f& proj, OVR::Quatf& gPose)
	{	
		//update proj in our Proxy object
		(VRclient->clientProxy)->proj = proj;

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadTransposeMatrixf(&proj.M[0][0]);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadTransposeMatrixf(&view.M[0][0]);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glColor3f(0.5f, 0.5f, 0.5f);
		glBegin(GL_LINES);
		int i, j;
		for (i = 0; i < 10; i++)
		{
			for (j = 0; j < 10; j++)
			{
				glVertex3f((0.0f - 4.5f) * 4.0f, (j - 4.5f) * 4.0f, 0);
				glVertex3f((9.0f - 4.5f) * 4.0f, (j - 4.5f) * 4.0f, 0);
				glVertex3f((i - 4.5f) * 4.0f, (0.0f - 4.5f) * 4.0f, 0);
				glVertex3f((i - 4.5f) * 4.0f, (9.0f - 4.5f) * 4.0f, 0);
			}
		}
		glEnd();

		OVR::Matrix4f rot(gPose);
		glMultTransposeMatrixf(&rot.M[0][0]);
		int maxLen = proxy->VolumeSize[0];
		if (maxLen < proxy->VolumeSize[1]) maxLen = proxy->VolumeSize[1];
		if (maxLen < proxy->VolumeSize[2]) maxLen = proxy->VolumeSize[2];
		float scale = proxy->ObjectScale / maxLen; // this maps volume size in voxels to world/object coordinates
		glColor3f(1.0f, 1.0f, 0.0f);
		glBegin(GL_LINES);
		float w = 0.5f * proxy->VolumeSize[0] * scale; // (half) width
		float h = 0.5f * proxy->VolumeSize[1] * scale; // (half) height
		float d = 0.5f * proxy->VolumeSize[2] * scale; // (half) depth
		glVertex3f(-w, -h, -d); glVertex3f(+w, -h, -d);
		glVertex3f(-w, +h, -d); glVertex3f(+w, +h, -d);
		glVertex3f(-w, -h, +d); glVertex3f(+w, -h, +d);
		glVertex3f(-w, +h, +d); glVertex3f(+w, +h, +d);

		glVertex3f(-w, -h, -d); glVertex3f(-w, +h, -d);
		glVertex3f(+w, -h, -d); glVertex3f(+w, +h, -d);
		glVertex3f(-w, -h, +d); glVertex3f(-w, +h, +d);
		glVertex3f(+w, -h, +d); glVertex3f(+w, +h, +d);

		glVertex3f(-w, -h, -d); glVertex3f(-w, -h, +d);
		glVertex3f(+w, -h, -d); glVertex3f(+w, -h, +d);
		glVertex3f(-w, +h, -d); glVertex3f(-w, +h, +d);
		glVertex3f(+w, +h, -d); glVertex3f(+w, +h, +d);
		glEnd();
		// Vector (x, y, z) in object coordinates goes through (x/scale, y/scale, z/scale) voxels
		// whose physical dimensions are (x/scale * proxy->VoxelSize[0], y/scale * proxy->VoxelSize[1], z/scale * proxy->VoxelSize[2]) nanometers.
		// Typically, voxels are not cubes (VoxelSize[2] > VoxelSize[0,1]), and therefore vectors need to be in object (volume) coordinates
		// rather than world coordinates when calculating phyiscal lengths.

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	VRUSERDLL_API void DrawPostVolumeRendering(OVR::Matrix4f& view, OVR::Matrix4f& proj, OVR::Quatf& gPose)
	{
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadTransposeMatrixf(&proj.M[0][0]);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadTransposeMatrixf(&view.M[0][0]);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glColor3f(1.0f, 0.0f, 0.0f);
		int i, j;
		for (i = 0; i < 10; i++)
		{
			for (j = 0; j < 10; j++)
			{
				glPushMatrix();
				glTranslatef((i - 4.5f) * 1.0f, (j - 4.5f) * 1.0f, 0);
				drawCube(0.1f);
				glPopMatrix();
			}
		}


		OVR::Matrix4f rot(gPose);
		glMultTransposeMatrixf(&rot.M[0][0]);
		glColor3f(0.0f, 1.0f, 0.0f);
		for (i = 0; i < 10; i++)
		{
			for (j = 0; j < 10; j++)
			{
				glPushMatrix();
				glTranslatef((i - 4.5f) * 1.0f, (j - 4.5f) * 1.0f, 0);
				drawCube(0.1f);
				glPopMatrix();
			}
		}

		(VRclient->clientScene)->Render(view, proj, rot);

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
}

