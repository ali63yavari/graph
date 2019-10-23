// BrokerApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

#include "../BrokerLib/ZmqBroker.h"

int main()
{
	graph::qc::broker::ZmqBroker broker("127.0.0.1", 
		1234, 
		"127.0.0.1", 
		1235, 
		"d:\\");

	broker.Start();

	getchar();
}

