// TestZMQServ.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include <zmq.h>

#include <stdio.h>
//#include <unistd.h>
#include <string.h>
#include <assert.h>

#include <string>
#include <iostream>
#include <windows.h>
#include <vector>

#include "../common/CameraSock.hpp"

#define sleep(n) Sleep(n)

int main(int argc, char **argv)
{
	camera_sock::CameraServerInit();

	FILE *fp;
	fopen_s(&fp, "Q:/camera_sock.test0.dump", "rb");

	using namespace camera_sock;
	using namespace std;
	vector<camera_t> vcam;

	while (1) {
		camera_t cam;
		int sz = fread(&cam, 1, sizeof(cam), fp);
		if (sz != sizeof(cam)) {
			break;
		}
		vcam.push_back(cam);
	}

	printf("Server started!\n");
	int index = 0;
	while (1) {
		CameraServerSend(vcam[index]);
		index = (index + 1) % vcam.size();
		sleep(500);
		printf("serv:%d vp:%f,%f,%f\n", index, vcam[index].viewpoint[0], vcam[index].viewpoint[1], vcam[index].viewpoint[2]);
	}

    return 0;
}

