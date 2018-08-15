// VRUserDll.hpp

/*
#ifdef VRUSERDLL_EXPORTS
#define VRUSERDLL_API __declspec(dllexport)
#else
#define VRUSERDLL_API __declspec(dllimport)
#endif
*/
#define VRUSERDLL_API __declspec(dllexport)


//#define VRUSERDLL_API __declspec(dllexport)

//#include "../OculusSample0/src/Win32_GLAppUtil.h" //NG

//moved these to stdafx.h
//#include "GL/CAPI_GLE.h"
//#include "Extras/OVR_Math.h"
//#include "OVR_CAPI_GL.h"

//#include "SceneManager.h"


namespace VRUserProxy {
	struct APIBundle {
		// exported from App
		float* (*GetPosition)(void *obj);
		int& (*GetClipMode)(void *obj);
		float& (*GetClipWidth)(void *obj);
		float& (*GetClipPos)(void *obj);
		OVR::Matrix4f& (*GetPose)(void *obj);
		OVR::Quatf& (*GetGlobalPose)(void *obj);
		OVR::Vector3f& (*GetGlobalHeadPosition)(void *obj);
		OVR::Matrix4f& (*GetGlobalHeadPose)(void *obj);
		void (*SetRoI)(int index, float value);
		float ObjectScale;
		int VolumeSize[3];
		float VoxelSize[3];
	};
	// export to App
	VRUSERDLL_API int OnInit(APIBundle *p, OVR::GLEContext *context);
	//VRUSERDLL_API void OnHandleTouch(void *recv, ovrSession session, long long frameIndex, OVR::Matrix4f& view);
	VRUSERDLL_API void OnHandleTouch(void *recv, ovrTrackingState& trackingState, ovrInputState &inputState, OVR::Matrix4f& view);
	VRUSERDLL_API void DrawPreVolumeRendering(OVR::Matrix4f& view, OVR::Matrix4f& proj, OVR::Quatf& gPose);
	VRUSERDLL_API void DrawPostVolumeRendering(OVR::Matrix4f& view, OVR::Matrix4f& proj, OVR::Quatf& gPose);
}