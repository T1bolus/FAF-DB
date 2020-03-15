#pragma once

void updateStatus(size_t *clients, size_t clientLength)
{
	size_t connectedClients = 0;
	while (true)
	{
		connectedClients = 0;
		this_thread::sleep_for(60s);

		//Counting connected sessions
		for (size_t i = 0; i < clientLength; i++)
		{
			if (clients[i] != INVALID_SOCKET)
				connectedClients++;
		}

		cout << "Status: " << "Connected Clients: " << connectedClients << endl;
	}
}