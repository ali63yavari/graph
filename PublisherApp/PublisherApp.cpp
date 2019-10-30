// PublisherApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <windows.h>

#include "../BrokerLib/BrokerMessage.h"
#include "../BrokerLib/ZmqPublisherClient.h"
#include "../BrokerLib/FileRepository.h"
#include "../BrokerLib/Common.h"


int cycle_rate = 0;
long long msg_counter = 0;
int last_data_len = 0;

int main()
{
	graph::qc::broker::clients::ZmqPublisherClient publisher(PUBLISHER_CLIENT_IP, FRONTEND_PORT);

	graph::qc::broker::models::BrokerMessage bm;
	bm.topic = DEFAULT_TOPIC;
	
	std::cout << "Publisher is ready." << std::endl;
	std::cout << "Press Ctrl+C to terminate." << std::endl;

	std::srand(std::time(nullptr));
	const auto range = MAX_DATA_LEN - MIN_DATA_LEN;

	 auto blank_line = std::string(60, ' '); 

	auto th = std::thread([&]()
	{
		while(true)
		{
			Gotoxy(10, 5);
			std::cout << blank_line ;
			Gotoxy(10, 6);
			std::cout << blank_line;
			Gotoxy(10, 7);
			std::cout << blank_line;
			
			Gotoxy(10, 5);
			std::cout << "Message Count:\t" << msg_counter;
			Gotoxy(10, 6);
			std::cout << "Estimated Data Rate:\t" << 1.0*1e6/cycle_rate << " msg/s";
			Gotoxy(10, 7);
			std::cout << "Last Data Len:\t" << last_data_len << " byte";

			zmq_sleep(1);
		}
	});
	

	while (true)
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		const auto data_len = std::rand() % range + MIN_DATA_LEN;
		last_data_len = data_len;
		std::vector<char> v(data_len, DEFAULT_DATA);
		bm.data_len = data_len;
		bm.data = &v.data()[0];

		bm.IncreaseCounter();
		publisher.PublishMessage(bm.topic, &bm);

		MicroSleep(70); //70
		auto t2 = std::chrono::high_resolution_clock::now();
		cycle_rate = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		msg_counter = bm.msg_id;
	}
}
