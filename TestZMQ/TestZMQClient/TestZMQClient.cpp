// TestZMQClient.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"


#include <zmq.h>
#include <string.h>
#include <stdio.h>
//#include <unistd.h>
#include <windows.h>
#include <assert.h>

#include "../common/CameraSock.hpp"

int main(int argc, char** argv)
{
	printf("Connecting to camera_sock server...\n");
	camera_sock::CameraClientInit();
	FILE *fp;
	fopen_s(&fp, "Q:/camera_sock.dump", "wb");
	assert(fp);
	int count = argc > 1 ? atoi(argv[1]) :100;
	int request_nbr;
	for (request_nbr = 0; request_nbr != 100; request_nbr++) {
		camera_sock::camera_t cam;
		camera_sock::CameraClientRecv(cam);
		int sz = fwrite(&cam, 1, sizeof(cam), fp);
		assert(sz == sizeof(cam));
		printf("vp:%f,%f,%f\n", cam.viewpoint[0], cam.viewpoint[1], cam.viewpoint[2]);
	}
	camera_sock::CameraClientClose();
	fclose(fp);
	return 0;
}