// Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ServerManager.h"
//#include "ClientGame.h"
// used for multi-threading
#include <process.h>

//void serverLoop(void *);
void serverLoop();
ServerManager * server;

int main()
{
	server = new ServerManager();
	//_beginthread(serverLoop, 0, (void*)12);

	//serverLoop((void*)12);
	serverLoop();

	//serverLoop((void*)12);
	while (true) {
		Sleep(INFINITE);
	}
}

//void serverLoop(void * arg)
void serverLoop()
{
	while (true)
	{
		server->update();
	}
}
