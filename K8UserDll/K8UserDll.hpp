// K8UserDll.hpp

#ifdef K8USERDLL_EXPORTS
#define K8USERDLL_API __declspec(dllexport)
#else
#define K8USERDLL_API __declspec(dllimport)
#endif


namespace K8UserProxy {
	struct APIBundle {
		// exported from App
		float* (*GetObjectDistance)();
		float* (*GetViewAngle)();
		float* (*GetZScale)();
		float* (*GetViewpoint)();
		float ObjectScale;
		int window_width;
		int window_height;
		float ObjectDistanceInit;
		float ObjectDistanceMin;
		float ObjectDistanceMax;
		int VolumeSize[3];
		float VoxelSize[3];
	};
	// export to App
	K8USERDLL_API int OnInit(APIBundle *p);
	K8USERDLL_API void SetModelview();
	K8USERDLL_API void mouse(int button, int state, int x, int y);
	K8USERDLL_API void motion(int x, int y);
	K8USERDLL_API void OnScale(int s, float speed);
	K8USERDLL_API void keyboard(unsigned char key, int x, int y);
	K8USERDLL_API void update_viewpoint();
	K8USERDLL_API void DrawPostVolumeRendering();
	K8USERDLL_API bool idle();
}
