// PublisherApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "../BrokerLib/BrokerMessage.h"
#include "../BrokerLib/ZmqPublisherClient.h"
#include "../BrokerLib/FileRepository.h"

int main()
{
	graph::qc::broker::clients::ZmqPublisherClient publisher("127.0.0.1", 1234);

	graph::qc::broker::models::BrokerMessage bm;
	bm.topic = "test1";
	std::vector<char> v(170, 0xde);
	bm.data_len = 170;
	bm.data = &v.data()[0];
	
	publisher.PublishMessage(bm.topic, bm);
}
