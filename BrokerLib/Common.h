#pragma once
#include <windows.h>
#include <chrono>

#define MAX_DATA_LEN			8192 //byte
#define MIN_DATA_LEN			50 //byte
#define DEFAULT_DATA			0xAB
#define DEFAULT_TOPIC			"proxy"
#define SUBSCRIBER_CLIENT_IP	"127.0.0.1"
#define PUBLISHER_CLIENT_IP		"127.0.0.1"
#define BACKEND_PORT			9999
#define FRONTEND_PORT			9989
#define REPO_PATH				"d:\\broker"

#define PARALLEL_CHANNEL_COUNT	5

inline void Gotoxy(int x, int y)
{
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

inline void MicroSleep(int us)
{
	auto t1 = std::chrono::high_resolution_clock::now();
	while (true)
	{
		auto t2 = std::chrono::high_resolution_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		if (d > us)
			break;
	}
}

