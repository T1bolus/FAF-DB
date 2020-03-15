#pragma once

//Add new Setting type and standard values here
struct fafSettings
{
	//Strings
	string dbFile = "inMemorySave.faf";
	string dbStoragePath = "db\\";

	//Integers
	size_t memoryDBSize = 10000;
	size_t memorySavingIntervallInSeconds = 600;
	size_t maxClientConnections = 1000;
	unsigned short serverTCPListeningPort = 3232;

	//Booleans
	bool hybridMode = true;
};

//Add new Settings here
inline void setSettings(fafSettings &s, const string &setting, const string &para)
{
	try 
	{
		if (setting == "dbFile")							s.dbFile = para;
		if (setting == "dbStoragePath")						s.dbStoragePath = para;
		if (setting == "memoryDBSize")						s.memoryDBSize = stoull(para);
		if (setting == "memorySavingIntervallInSeconds")	s.memorySavingIntervallInSeconds = stoull(para);
		if (setting == "hybridMode")						s.hybridMode = (bool)stoi(para);
		if (setting == "maxClientConnections")				s.maxClientConnections = stoull(para);
		if (setting == "serverTCPListeningPort")			s.serverTCPListeningPort = (short)stoul(para);
	}
	catch (...)
	{
		cerr << "[SETTING-EXCEPTION] " << setting << ": Invalid option='" << para << "', using default value instead;" << endl;
	}
}



//Don't touch dis
void readSettings(fafSettings &fafSettings, const string path)
{
	//Reading Algo
	string setting, para;

	ifstream input(path);
	if (input.is_open())
	{
		while (!input.eof())
		{
			input >> setting >> para;
			setSettings(fafSettings, setting, para);
		}
	}
}