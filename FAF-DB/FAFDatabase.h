/*
FAF-Database Class
Fast, ThreadSafe, Hybrid In-Memory-Storage Database
*/



#pragma once
#include <string>
#include <thread>
#include <shared_mutex>


class FAFDatabase
{
public:
	FAFDatabase(std::string inMemoryStorageFilePath = "inMemorySave.faf", std::string storageDBPath = "", bool hybridMode = true, size_t inMemorySize = 10000);

	//Database Modifier Basic Functions
	std::string get(const std::string &key);
	void insert(const std::string &key, const std::string &data, bool forceStorage); //Create key-data pair, fails when key already exist
	void update(const std::string &key, const std::string &data); //Update key data, create new key when key doesn't exist
	bool remove(const std::string &key, bool forceMemory); //Delete key-data pair
	
	//Advanced Modifier Functions
	bool increase(const std::string &key); //Increase Integer Value
	bool decrease(const std::string &key); //Decrease Integer Value
	void add(const std::string &key, const std::string &data); //Add data und remain the old ones
	std::string swapElements(const std::string &key); //swap element inMemory/storage
	std::string getAllInMemoryKeys(); //get all existing inMemory Keys

	//Load/Save Methods
	void loadDatabaseFromFile();
	void forceSave();
	void startAutoSaving(size_t time);

private:
	void autoSavingThread(size_t time);

	std::string inMemoryStorageFilePath;
	std::string storageDBPath;
	bool hybridMode;
	size_t inMemorySize;
	shared_mutex mmtx;						// memory mutex for writing, updating & deleting data, thread safe
	map<std::string, std::string> db;		//In-Memory Database - O(log n)
};

inline FAFDatabase::FAFDatabase(std::string inMemoryStorageFilePath, std::string storageDBPath, bool hybridMode, size_t inMemorySize) : inMemoryStorageFilePath(inMemoryStorageFilePath), storageDBPath(storageDBPath), hybridMode(hybridMode)
{
	this->inMemorySize = inMemorySize ? inMemorySize : 1;
}

inline std::string FAFDatabase::get(const std::string &key)
{
	string retBuffer = "";

	//Check Ram
	mmtx.lock_shared();
	auto it = db.find(key);
	if (it != db.end())
		retBuffer = it->second;
	mmtx.unlock_shared();

	//Check storage
	if (it == db.end() && hybridMode)
	{
		retBuffer = swapElements(key);
	}

	if (debugMode)
		cout << "GET: " << key << endl;

	return retBuffer;
}

inline void FAFDatabase::insert(const std::string &key, const std::string &data, bool forceStorage = false)
{
	if (key.size() == 0 || key.size() > 250) return;

	size_t sCopy;

	mmtx.lock();
	sCopy = db.size(); //Copy size for thread safty usage
	if ((sCopy < inMemorySize && forceStorage == false) || hybridMode == false)
		db.insert(pair<std::string, std::string>(key, data));
	mmtx.unlock();

	if ((sCopy >= inMemorySize || forceStorage) && hybridMode) //Harddisk Storage
	{
		fstream fileHandler(storageDBPath + key);
		if (!fileHandler.is_open()) //If file not found
		{
			do
			{
				fileHandler.open(storageDBPath + key, ios_base::out); //reopen again as output, bad design?
			} while (!fileHandler.is_open());

			fileHandler << data;
		}
		fileHandler.close();
	}
	

	if (debugMode)
		cout << "INSERT: " << key << endl;
}

inline void FAFDatabase::update(const std::string &key, const std::string &data)
{
	if (key.size() == 0 || key.size() > 250) return;

	size_t sCopy;
	bool foundKey;

	mmtx.lock();
	foundKey = db.find(key) != db.end();
	sCopy = db.size(); //Copy size for thread safty usage
	if (foundKey || sCopy < inMemorySize || hybridMode == false)
		db[key] = data;
	mmtx.unlock();

	if (sCopy >= inMemorySize && foundKey == false && hybridMode) //Harddisk Storage
	{
		ofstream fileHandler; //output
		
		do
		{
			fileHandler.open(storageDBPath + key); //reopen again as output, bad design?
		} while (!fileHandler.is_open());

		fileHandler << data;
		fileHandler.close();

		swapElements(key);
	}

	if (debugMode)
		cout << "UPDATE: " << key << endl;
}

inline bool FAFDatabase::remove(const std::string &key, bool forceMemory = false)
{
	if (key.size() == 0 || key.size() > 250) return false;

	if (debugMode)
		cout << "REMOVE: " << key << endl;

	bool removed = false;
	mmtx.lock();
	removed = (bool)db.erase(key);
	mmtx.unlock();


	if (forceMemory == false && hybridMode)
	{
		removed = std::experimental::filesystem::remove(storageDBPath + key) || removed; //just for testing
		//removed = DeleteFileA(string(dbPath + dbStoragePath + key).c_str());
	}

	return removed;
}

inline std::string FAFDatabase::swapElements(const std::string &key)
{
	std::string retBuffer;

	ifstream getFromStorage(storageDBPath + key);
	if (getFromStorage.is_open())
	{
		retBuffer = std::string((std::istreambuf_iterator<char>(getFromStorage)), std::istreambuf_iterator<char>());
		getFromStorage.close();

		//Swaping from storage to memory
		mmtx.lock_shared();
		size_t sCopy = db.size(); //Copy size for thread safty usage
		mmtx.unlock_shared();

		//Insert new data to memory
		//remove(key, false); //remove old storage data
		insert(key, retBuffer, false); //insert old storage data to memory

		if (sCopy >= inMemorySize)
		{
			//Pseudo Random Generator
			default_random_engine generator;
			uniform_int_distribution<size_t> distribution(0, sCopy ? sCopy - 1 : 0);

			//Beginning swapping and find random element to swap
			string randomKey, randomData;
			mmtx.lock_shared();
			auto itemToSwap = db.begin();
			std::advance(itemToSwap, distribution(generator));
			randomKey = itemToSwap->first;
			randomData = itemToSwap->second;
			mmtx.unlock_shared();

			if (itemToSwap != db.end()) //Empty Memory-Database
			{
				insert(randomKey, randomData, true); //insert into storage
				remove(randomKey, true); //remove only from memory
			}
		}
	}

	return retBuffer;
}

inline void FAFDatabase::startAutoSaving(size_t time)
{
	thread starterThread(&FAFDatabase::autoSavingThread, this, time);
	starterThread.detach();
}

void FAFDatabase::autoSavingThread(size_t time)
{
	while (true)
	{
		this_thread::sleep_for(time * 1s); //Sleeptime

		mmtx.lock_shared(); //Locking for saving
		ofstream output(inMemoryStorageFilePath);
		for (auto i : db)
		{
			output << i.first << ";" << i.second.length() << ";" << i.second;
		}
		output.close();
		mmtx.unlock_shared(); //Release locking
	}
}

void FAFDatabase::forceSave()
{
	mmtx.lock_shared(); //Locking for saving
	ofstream output(inMemoryStorageFilePath);
	for (auto i : db)
	{
		output << i.first << ";" << i.second.length() << ";" << i.second;
	}
	output.close();
	mmtx.unlock_shared(); //Release locking
}

void FAFDatabase::loadDatabaseFromFile()
{
	ifstream input(inMemoryStorageFilePath);

	input >> noskipws;

	string key, data;
	char c = ' ';
	size_t length;

	while (input.is_open() && !input.eof())
	{
		key = "", data = "";

		input >> c; //First char
		do
		{
			key += c;
			input >> c;
		} while (c != ';' && !input.eof());

		input >> length;
		input >> c; //Semicolon
		//if end of file
		if (input.eof()) return;

		data.resize(length);
		input.read(const_cast<char *>(data.c_str()), length); //read specific length

		insert(key, data);
	}
	input.close();
}

inline bool FAFDatabase::increase(const std::string &key)
{
	try
	{
		string sNumber = get(key);
		if (sNumber != "") //If Key-Data is empty
		{
			size_t number = stoull(sNumber); //if not a number -> exception -> return false
			if (number == (size_t)-1) throw -1;
			number++; //increase value
			update(key, to_string(number));
		}
		else //Set to 1
		{
			update(key, "1");
		}

		return true;
	}
	catch (...)
	{
		if (debugMode)
			cout << "INCREASE FAILED: " << key << endl;
	}
	return false;
}

inline bool FAFDatabase::decrease(const std::string &key)
{
	try
	{
		string sNumber = get(key);
		if (sNumber != "") //If Key-Data is empty
		{
			size_t number = stoull(sNumber); //if not a number -> exception -> return false
			if (number == 0) throw -1;
			number--; //decrease value
			update(key, to_string(number));
		}
		else //Set to 0
		{
			update(key, "0");
		}

		return true;
	}
	catch (...)
	{
		if (debugMode)
			cout << "INCREASE FAILED: " << key << endl;
	}
	return false;
}

inline void FAFDatabase::add(const std::string & key, const std::string & data)
{
	std::string content = this->get(key);
	content += data;
	this->update(key, content);
}

inline std::string FAFDatabase::getAllInMemoryKeys()
{
	std::string allKeys = "";

	for (auto i : db)
		allKeys += i.first + ";";

	return allKeys;
}