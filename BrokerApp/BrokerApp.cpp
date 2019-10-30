// BrokerApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "../BrokerLib/ZmqBroker.h"
#include <iostream>
#include "../BrokerLib/Common.h"

int main()
{
	graph::qc::broker::ZmqBroker broker("*", 
		BACKEND_PORT, 
		"*", 
		FRONTEND_PORT, 
		REPO_PATH);


	broker.Start();

	std::cout << "Broker is ready." << std::endl;
	std::cout << "Press any key to terminate." << std::endl;

	getchar();
}

