#pragma once

//CPP STUFF
#include <iostream>
#include <string>
#include <streambuf>
#include <map>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cstddef> //size_t
#include <random>

using namespace std; //OMG, NO WAY :O
					 //I can handle dis shit, so stfu :P

#define debugMode true

//WIN STUFF
#include <Windows.h>
#pragma comment( lib,"ws2_32.lib" )




//General 
//#define INVALID_SOCKET 0 //linux



//Includes
#include "Settings.h"
#include "Status.h"
#include "FAFDatabase.h"
#include "Help.h"
#include "ClientManagment.h"
#include "Server.h"


