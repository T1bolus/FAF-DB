#pragma once


void clientConnection(size_t &socket, FAFDatabase &db)
{

#define bufSize 256
	char buf[bufSize] = "";
	string recString;
	int len = 0;
	string ans;

	while (true)
	{
		recString = "";
		do
		{
			len = recv(socket, buf, bufSize - 1, 0);
			if (len == SOCKET_ERROR)
			{
				closesocket(socket);
				socket = INVALID_SOCKET;
				return;
			}

			buf[len] = '\0';
			recString += buf;
		} while (len == bufSize - 1);


		if (recString.length() < 2)
		{
			closesocket(socket);
			socket = INVALID_SOCKET;

#if debugMode == true
			cout << "Malformed Packet!\n";
#endif

			return;  //malformed packet
		}

#if debugMode == true
		cout << recString << "\n";
#endif


		//GET NEW DATA
		if (recString[0] == 'g'  && recString[1] == ';')
		{
			string key = recString.substr(2);

			ans = ";" + db.get(key);

			send(socket, ans.c_str(), (int)ans.size(), 0);
		}

		//SET NEW DATA
		else if (recString[0] == 's'  && recString[1] == ';')
		{
			size_t delim = findDelim(recString);
			if (delim == string::npos) return;

			const string key = recString.substr(2, delim - 2);
			const string data = recString.substr(delim + 1);

			db.insert(key, data);

			send(socket, "s;1", (int)strlen("s;1"), 0);
		}

		//UPDATE NEW DATA
		else if (recString[0] == 'u'  && recString[1] == ';')
		{
			const size_t delim = findDelim(recString);
			if (delim == string::npos) return;

			string key = recString.substr(2, delim - 2);
			string data = recString.substr(delim + 1);

			db.update(key, data);

			send(socket, "u;1", (int)strlen("u;1"), 0);
		}

		//DELETE DATA
		else if (recString[0] == 'd'  && recString[1] == ';')
		{
			string key = recString.substr(2);

			const int deleted = db.remove(key);

			ans = "d;" + to_string(deleted);
			send(socket, ans.c_str(), (int)ans.size(), 0);
		}

		//FORCE SAVE
		else if (recString[0] == 'f'  && recString[1] == ';')
		{
			db.forceSave();

			ans = "f;";
			send(socket, ans.c_str(), (int)ans.size(), 0);
		}

		//INCREASE
		else if (recString[0] == 'i' && recString[1] == 'n' && recString[2] == ';')
		{
			string key = recString.substr(3);
			bool suc = db.increase(key);

			ans = "in;" + to_string((int)suc);
			send(socket, ans.c_str(), (int)ans.size(), 0);
		}

		//DECREASE
		else if (recString[0] == 'd' && recString[1] == 'e' && recString[2] == ';')
		{
			string key = recString.substr(3);
			bool suc = db.decrease(key);

			ans = "de;" + to_string((int)suc);
			send(socket, ans.c_str(), (int)ans.size(), 0);
		}

		//ADD DATA
		else if (recString[0] == 'a'  && recString[1] == ';')
		{
			const size_t delim = findDelim(recString);
			if (delim == string::npos) return;

			string key = recString.substr(2, delim - 2);
			string data = recString.substr(delim + 1);

			db.add(key, data);

			send(socket, "a;1", (int)strlen("a;1"), 0);
		}

		//GET ALL IN-Memory KEYS
		else if (recString[0] == 'k'  && recString[1] == ';')
		{
			string key = recString.substr(2);

			ans = db.getAllInMemoryKeys();

			send(socket, ans.c_str(), (int)ans.size(), 0);
		}

		else
			send(socket, "UNKNOWN COMMAND;", 16, 0);
	}
}