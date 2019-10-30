// SubscriberApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include "../BrokerLib/ZmqSubscriberClient.h"
#include "../BrokerLib/Common.h"


int main()
{	
	graph::qc::broker::clients::ZmqSubscriberClient subs(SUBSCRIBER_CLIENT_IP, BACKEND_PORT, DEFAULT_TOPIC, nullptr);

	std::cout << "Subscriber is ready." << std::endl;
	std::cout << "Press Ctrl+C to terminate." << std::endl;

	const auto blank_line = std::string(60, ' ');

	uint64_t last_id{0};
	uint64_t msg_counter{0};

	while(true)
	{
		auto b = subs.WaitForDataChanged(100);
		if (!b)
		{
			zmq_sleep(1);
			continue;
		}

		subs.GetData(last_id, msg_counter);

		Gotoxy(10, 5);
		std::cout << blank_line;
		Gotoxy(10, 6);
		std::cout << blank_line;
		Gotoxy(10, 7);
		std::cout << blank_line;

		Gotoxy(10, 5);
		std::cout << "Message Count:\t" << msg_counter;
		Gotoxy(10, 6);
		std::cout << "Last Published Id:\t" << last_id;		
		Gotoxy(10, 7);
		std::cout << "Difference:\t" << last_id - msg_counter;

		zmq_sleep(1);
	}
}

