﻿#include "memory_util.h"
#include <iostream>
#include "draw_window.h"
#include "enemy_data.h"
#include <thread>

int main() {
	// client.dll+12A9E8 metrics
	std::cout << "input process id: " << std::endl;
	int processId;
	std::cin >> processId;

	HWND game = FindWindowA(NULL, "Counter-Strike");
	HWND window = CreateTransparentWindow(game);
	// std::thread calc_thread(GetEnemyPosition, processId);
	std::thread name_thread(GetAllEnemyPosition, processId);
	MessageHandle(game, window);
	// calc_thread.join();
	name_thread.join();
}