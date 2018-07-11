// VRUserDll.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "VRUserDll.hpp"
#include "SceneManager.h"

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



//static std::vector<OVR::Vector3f> worldMarkers;
//static std::vector<OVR::Vector3f> volumeMarkers;



static Scene * roomScene = nullptr;


namespace VRUserProxy {
	APIBundle *proxy;
	VRUSERDLL_API int OnInit(APIBundle *p, OVR::GLEContext *context) {
		proxy = p;
		OVR::GLEContext::SetCurrentContext(context);
		//worldMarkers.clear();
		//volumeMarkers.clear();
		roomScene = new Scene();

		return 0;
	}
	bool OnDown(unsigned int last, unsigned int current, unsigned int mask) {
		return (last&mask) == 0 && (current&mask);
	}

	unsigned int lastButtons = 0;
	ovrPosef lastHandPoses[2];
	VRUSERDLL_API void OnHandleTouch(void *recv, ovrTrackingState& trackState, ovrInputState &inputState, OVR::Matrix4f& view) {
		using namespace OVR;

		DPrintf("VRUserProxy::OnHadleTouch\n");

		float *Position = proxy->GetPosition(recv);
		int &ClipMode = proxy->GetClipMode(recv);
		float &ClipWidth = proxy->GetClipWidth(recv);
		float &ClipPos = proxy->GetClipPos(recv);
		Matrix4f &Pose = proxy->GetPose(recv);
		Quatf &gPose = proxy->GetGlobalPose(recv);
		Vector3f &gHeadPos = proxy->GetGlobalHeadPosition(recv);
		Matrix4f &gHeadOrientation = proxy->GetGlobalHeadPose(recv);

		ovrPosef         handPoses[2];
		handPoses[ovrHand_Left] = trackState.HandPoses[ovrHand_Left].ThePose;
		handPoses[ovrHand_Right] = trackState.HandPoses[ovrHand_Right].ThePose;

		//new experimental hmd pose
		ovrPosef hmdPose = trackState.HeadPose.ThePose;

		DPrintf("VRUserProxy::GetInputState OK\n");
		int RoiMode = 0;
		Matrix4f v = view;
		v.M[0][3] = 0.f;
		v.M[1][3] = 0.f;
		v.M[2][3] = 0.f;
		v.M[3][3] = 1.f;
		v.M[3][0] = 0.f;
		v.M[3][1] = 0.f;
		v.M[3][2] = 0.f;

		//move all temp markers and remove all temp lines before any actions
		roomScene->moveTempModels(handPoses[ovrHand_Right], gPose, gHeadPos, gHeadOrientation, view);


		// left trigger
		if (inputState.IndexTrigger[ovrHand_Left] > 0.5f) {
			RoiMode = 1;
		}

		//roomScene->ControllerActions(handPoses[ovrHand_Left], handPoses[ovrHand_Right], gPose, gHeadPos, inputState, gHeadOrientation, view, false);

		// Left Stick is for Volume Clip
		if (ClipMode) {
			if (inputState.Buttons & ovrButton_LThumb) {
				float speed = 0.02f;
				ClipWidth +=
					inputState.Thumbstick[ovrHand_Left].x*speed +
					inputState.Thumbstick[ovrHand_Left].y*speed;
				ClipWidth = Clip<float>(0.01f, 1.0f, ClipWidth);
			}
			else {
				float speed = 0.02f;
				ClipPos +=
					inputState.Thumbstick[ovrHand_Left].x*speed +
					inputState.Thumbstick[ovrHand_Left].y*speed;
				ClipPos = Clip<float>(0.0f, 1.0f, ClipPos);
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
			Vector3f center = gHeadPos + Vector3f(0.f, 0.f, -(RoiDepth + RoiRange*0.5f)*gObjectScale);
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
			gPose = q0 * q1.Inverse() * gPose;
		}

		
		if (OnDown(lastButtons, inputState.Buttons, ovrButton_X))
		{
			if (!(inputState.Buttons & ovrButton_A)) {
				// Handle A button down
				ClipMode = (ClipMode + 1) % 4;
			}
		}
		

		/*
		// Foward/Back
		if (inputState.IndexTrigger[ovrHand_Right] > 0.1f) {
			DPrintf(" FW\n");
			// forward
			float speed = -0.8f;
			float forward = inputState.IndexTrigger[ovrHand_Right] * speed;
			Vector3f movement = v.Inverted().Transform(Vector3f(0.0f, 0.0f, forward));
			Position[0] += movement.x;
			Position[1] += movement.y;
			Position[2] += movement.z;
		}
		if (inputState.Buttons & ovrButton_A) {
			DPrintf(" BW\n");
			// back
			float speed = 0.8f;
			Vector3f movement = v.Inverted().Transform(Vector3f(0.0f, 0.0f, speed));
			Position[0] += movement.x;
			Position[1] += movement.y;
			Position[2] += movement.z;
		}

		if (inputState.Buttons & ovrButton_B) {
			DPrintf(" World Marker\n");
			Vector3f pos = Vector3f(handPoses[ovrHand_Right].Position);
			pos = gHeadOrientation.Inverted().Transform(pos - gHeadPos);
			worldMarkers.push_back(view.Inverted().Transform(pos));
		}
		if (inputState.Buttons & ovrButton_Y) {
			DPrintf(" Volume Marker L\n");
			Vector3f pos = Vector3f(handPoses[ovrHand_Left].Position);
			pos = gHeadOrientation.Inverted().Transform(pos - gHeadPos);
			OVR::Matrix4f rot(gPose);
			volumeMarkers.push_back(rot.Inverted().Transform(view.Inverted().Transform(pos)));
		}
		*/


		//Controller actions influencing the scene (A,B,X,Y)
		roomScene->ControllerActions(hmdPose, handPoses[ovrHand_Left], handPoses[ovrHand_Right], gPose, gHeadPos, inputState, gHeadOrientation, view);

		//R Thumb Pressed
		if (inputState.Buttons & ovrButton_RThumb) {
			// Zoom
			DPrintf(" StickZoom\n");
			float speed = -0.8f;
			float forward = inputState.Thumbstick[ovrHand_Right].y*speed;

			Vector3f movement = v.Inverted().Transform(Vector3f(0.0f, 0.0f, forward));
			Position[0] += movement.x;
			Position[1] += movement.y;
			Position[2] += movement.z;
		}

		//R Thumb stick
		else {
			DPrintf(" RThumb\n");
			// Translation
			float speed = 0.1f;
			float mx = inputState.Thumbstick[ovrHand_Right].x*speed;
			float my = inputState.Thumbstick[ovrHand_Right].y*speed;
			Vector3f movement = v.Inverted().Transform(Vector3f(mx, my, 0.f));
			Position[0] += movement.x;
			Position[1] += movement.y;
			Position[2] += movement.z;
		}

		/*
		// R Thumb
		if (inputState.Buttons & ovrButton_RThumb) {
			DPrintf(" RThumb\n");
			// Translation
			float speed = 0.1f;
			float mx = inputState.Thumbstick[ovrHand_Right].x*speed;
			float my = inputState.Thumbstick[ovrHand_Right].y*speed;
			Vector3f movement = v.Inverted().Transform(Vector3f(mx, my, 0.f));
			Position[0] += movement.x;
			Position[1] += movement.y;
			Position[2] += movement.z;
		}
		else {
			DPrintf(" Rot\n");
			// Rotation
			float aspeed = 0.1f;
			float pitch = inputState.Thumbstick[ovrHand_Right].x*aspeed;
			float yaw = inputState.Thumbstick[ovrHand_Right].y*aspeed;
			Matrix4f Pose0 = Pose;
			Matrix4f newPoseO = Pose0 * gHeadOrientation * Matrix4f(Quatf(0.f, sinf(pitch), 0.f, -cosf(pitch)) * Quatf(sinf(yaw), 0.f, 0.f, cosf(yaw)));
			Pose = newPoseO * gHeadOrientation.Inverted();

			// head pos conpensate
			Vector3f p = -(Pose.Transform(gHeadPos) - Pose0.Transform(gHeadPos));
			Position[0] += p.x;
			Position[1] += p.y;
			Position[2] += p.z;
		}
		*/



		// R Hand Trigger for MotionTracking
		if (inputState.HandTrigger[ovrHand_Right] > 0.1f) {
			DPrintf(" R Tracking\n");
			// translation
			//static float positionTrackingSpeed = option::Option("PositionTrackingSpeed", 6.0f);
			static float positionTrackingSpeed = 6.0f;
			ovrPosef pose1 = handPoses[ovrHand_Right];
			ovrPosef pose0 = lastHandPoses[ovrHand_Right];
			Vector3f movement = v.Inverted().Transform(Vector3f(pose1.Position) - Vector3f(pose0.Position)) * -(positionTrackingSpeed*inputState.HandTrigger[ovrHand_Right]);
			Position[0] += movement.x;
			Position[1] += movement.y;
			Position[2] += movement.z;
			//DPrintf("Motion: %.4f, %.4f, %.4f\n", movement.x, movement.y, movement.z);
			// rotation
			Quatf q0(pose0.Orientation);
			Quatf q1(pose1.Orientation);
			Pose = Pose * Matrix4f(q0) * Matrix4f(q1).Transposed();
		}

		//lastLeftHandTrigger = inputState.HandTrigger[ovrHand_Left];
		lastButtons = inputState.Buttons;
		lastHandPoses[ovrHand_Left] = handPoses[ovrHand_Left];
		lastHandPoses[ovrHand_Right] = handPoses[ovrHand_Right];

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
		/*
		glColor3f(0.0f, 0.0f, 1.0f);
		std::vector<OVR::Vector3f>::iterator it;
		for (it = worldMarkers.begin(); it != worldMarkers.end(); it++)
		{
			glPushMatrix();
			glTranslatef(it->x, it->y, it->z);
			drawCube(0.1f);
			glPopMatrix();
		}
		*/

		//render all models in scene (markers, lines, etc.)
		//roomScene->Render(view, proj);


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

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
		roomScene->Render(view, proj, rot);

		/*
		glColor3f(1.0f, 0.0f, 1.0f);
		for (it = volumeMarkers.begin(); it != volumeMarkers.end(); it++)
		{
			glPushMatrix();
			glTranslatef(it->x, it->y, it->z);
			drawCube(0.1f);
			glPopMatrix();
		}
		*/

		glPopMatrix();

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}
}

