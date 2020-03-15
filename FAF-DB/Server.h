#pragma once

void startServer(FAFDatabase &db, size_t maxClients, unsigned short tcpListeningPort)
{
	long rc;
	SOCKET acceptSocket;
	SOCKADDR_IN addr;
	SOCKADDR_IN accept_addr;
	int len = sizeof(struct sockaddr);
	size_t nClients = maxClients; //MAX SIMULTANIUS CONNECTIONS

	size_t *clients = new size_t[nClients];


	// start winsockets
	WSADATA wsa; //Windows
	WSAStartup(MAKEWORD(2, 2), &wsa);


	// create acception socket
	acceptSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (acceptSocket == INVALID_SOCKET)
	{
		cout << "[SERVER] Failed to create Socket:" << WSAGetLastError() << endl;
		return;
	}

	// bind socket
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(tcpListeningPort);
	addr.sin_addr.s_addr = INADDR_ANY;
	rc = ::bind(acceptSocket, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));

	if (rc == SOCKET_ERROR)
	{
		cout << "[SERVER] Failed to bind Socket on this port:" << WSAGetLastError() << endl;
		return;
	}

	for (size_t i = 0; i < nClients; i++)
	{
		clients[i] = INVALID_SOCKET;
	}


	// start listening mode
	rc = listen(acceptSocket, 100);
	if (rc == SOCKET_ERROR)
	{
		cout << "[SERVER] Failed to listen:" << WSAGetLastError() << endl;
		return;
	}

#if debugMode == true
	thread us(updateStatus, clients, nClients);
#endif


	cout << "[SERVER] Ready to Race!\n";

	while (true)
	{
		//Use free socket
		for (size_t i = 0; i < nClients; i++)
		{
			if (clients[i] == INVALID_SOCKET)
			{
				//accept new connection
				clients[i] = accept(acceptSocket, (struct sockaddr *)&accept_addr, &len);
				if (clients[i] == INVALID_SOCKET)
				{
					cout << "[SERVER] Failed to accept:" << WSAGetLastError() << endl;
					return;
				}
				else
				{
					thread newOne(clientConnection, ref(clients[i]), ref(db));
					newOne.detach(); //detach to free the thread and to selfdestroying after finishing;
				}
				break;
			}
		}
	}
}