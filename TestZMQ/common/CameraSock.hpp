extern "C" {
	struct S_camera {
		float modelview[16];
		float projection[16];
		float viewpoint[3];
	};
}

namespace camera_sock {
	typedef struct S_camera camera_t;

	extern void *camera_pub, *camera_pub_context;
	extern void *camera_sub, *camera_sub_context;
	void CameraServerInit(const char* port = "tcp://*:5555");
	void CameraServerClose();
	void CameraServerSend(float *mv, float *proj, float *vp);
	void CameraServerSend(camera_t &cam);
	void CameraClientInit(const char* port = "tcp://localhost:5555");
	void CameraClientRecv(float *mv, float *proj, float *vp);
	void CameraClientRecv(camera_t &cam);
	bool CameraClientPoll();
	void CameraClientClose();
}