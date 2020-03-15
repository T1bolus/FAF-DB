#include "Includes.h"



int main()
{
	cout << "FastAsFuck Database-Server\nInitialisating..\n";
	
	//Read Settings
	fafSettings dbSetting;
	readSettings(dbSetting, "config.ini");
	
	cout << "[HybridMode] = " << boolalpha << dbSetting.hybridMode << endl;
	cout << "[ListeningPort] = " << dbSetting.serverTCPListeningPort << endl;


	//Init Database
	FAFDatabase fafdb(dbSetting.dbFile, dbSetting.dbStoragePath, dbSetting.hybridMode, dbSetting.memoryDBSize);
	fafdb.loadDatabaseFromFile();
	fafdb.startAutoSaving(dbSetting.memorySavingIntervallInSeconds);


	//Start Server
	thread tserver(startServer,ref(fafdb), dbSetting.maxClientConnections, dbSetting.serverTCPListeningPort);
	tserver.detach();

	while (true)
	{
		this_thread::sleep_for(5s);
		//cout << sizeof fafdb << endl;
	}
}