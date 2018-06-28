#include "../common/CameraSock.hpp"
#include <zmq.h>
#include <assert.h>

namespace camera_sock {

	void *camera_pub = 0, *camera_pub_context = 0;
	void *camera_sub = 0, *camera_sub_context = 0;

	void CameraServerInit(const char *port) {
		void *context = zmq_ctx_new();
		void *pub = zmq_socket(context, ZMQ_PUB);
		int rc = zmq_bind(pub, port);
		assert(rc == 0);
		camera_pub_context = context;
		camera_pub = pub;
	}

	void CameraServerClose() {
	}

	void CameraServerSend(float *mv, float *proj, float *vp) {
		camera_t cam;
		memcpy(cam.modelview, mv, sizeof(float) * 16);
		memcpy(cam.projection, proj, sizeof(float) * 16);
		memcpy(cam.viewpoint, vp, sizeof(float) * 3);

		zmq_send(camera_pub, &cam, sizeof(camera_t), 0);
	}
	void CameraServerSend(camera_t &cam) {
		zmq_send(camera_pub, &cam, sizeof(camera_t), 0);
	}

	void CameraClientInit(const char *port) {
		void *context = zmq_ctx_new();
		void *sub = zmq_socket(context, ZMQ_SUB);
		int rc = zmq_connect(sub, port);
		assert(rc == 0);
		rc = zmq_setsockopt(sub, ZMQ_SUBSCRIBE, "", 0);
		assert(rc == 0);
		camera_sub = sub;
		camera_sub_context = context;
	}

	void CameraClientRecv(float *mv, float *proj, float *vp) {
		camera_t cam;
		zmq_recv(camera_sub, &cam, sizeof(camera_t), 0);

		memcpy(mv, cam.modelview, sizeof(float) * 16);
		memcpy(proj, cam.projection, sizeof(float) * 16);
		memcpy(vp, cam.viewpoint, sizeof(float) * 3);
	}
	void CameraClientRecv(camera_t &cam) {
		zmq_recv(camera_sub, &cam, sizeof(camera_t), 0);
	}
	bool CameraClientPoll() {
		zmq_pollitem_t poll;
		poll.socket = camera_sub;
		poll.fd = 0;
		poll.events = ZMQ_POLLIN;
		int rc = zmq_poll(&poll, 1, 0);
		return rc == 1;
	}

	void CameraClientClose() {
		if (camera_sub) {
			zmq_close(camera_sub);
			zmq_ctx_destroy(camera_sub_context);
			camera_sub = 0;
			camera_sub_context = 0;
		}
	}
}