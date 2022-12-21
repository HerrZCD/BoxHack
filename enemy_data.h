#pragma once
#include <windows.h>
#include "memory_util.h"
float screen_position_x = 0.0f;
float screen_position_y = 0.0f;

void GetEnemyPosition(int processId) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	char* dllBase = GetModuleBase(L"hw.dll", processId);
	char* clientBase = GetModuleBase(L"client.dll", processId);
	float matrix[4][4] = { 0 };

	RECT rect;
	HWND hwnd = FindWindowA(NULL, "Counter-Strike");
	GetWindowRect(hwnd, &rect);

	int view_port_width = (rect.right - rect.left) / 2;
	int view_port_height = (rect.bottom - rect.top) / 2;

	while (true) {
		std::cout << "view_port_width: " << view_port_width << std::endl;
		std::cout << "view_port_height: " << view_port_height << std::endl;
		DWORD v1 = RPM<DWORD>(process, (DWORD)dllBase + 0x006E92AC);
		DWORD v2 = RPM<DWORD>(process, v1 + 0x7C);
		DWORD v3 = RPM<DWORD>(process, v2 + 0x5D4);
		DWORD v4 = RPM<DWORD>(process, v3 + 0x4);

		// Enemy
		DWORD e1 = RPM<DWORD>(process, (DWORD)dllBase + 0x007BBD9C);
		float v5 = RPM<float>(process, v4 + 0x8);
		float enemy_z = RPM<float>(process, (DWORD)e1 + 0x3B4);
		float enemy_y = RPM<float>(process, (DWORD)e1 + 0x3B0);
		float enemy_x = RPM<float>(process, (DWORD)e1 + 0x3AC);
		system("cls");
		std::cout << "Player postion x: " << (float)v5 << std::endl;
		std::cout << "enemy postion x: " << (float)enemy_x << std::endl;
		std::cout << "enemy postion y: " << (float)enemy_y << std::endl;
		std::cout << "enemy postion z: " << (float)enemy_z << std::endl;
		ReadProcessMemory(process, (LPCVOID)((DWORD)dllBase + 0x78E5A4), &matrix, 16 * sizeof(float), NULL);
		std::cout << "********************************************" << std::endl;
		std::cout << matrix[0][0] << " | " << matrix[0][1] << " | " << matrix[0][2] << " | " << matrix[0][3] << std::endl;
		std::cout << matrix[1][0] << " | " << matrix[1][1] << " | " << matrix[1][2] << " | " << matrix[1][3] << std::endl;
		std::cout << matrix[2][0] << " | " << matrix[2][1] << " | " << matrix[2][2] << " | " << matrix[2][3] << std::endl;
		std::cout << matrix[3][0] << " | " << matrix[3][1] << " | " << matrix[3][2] << " | " << matrix[3][3] << std::endl;
		std::cout << "********************************************" << std::endl;

		int camera_z = matrix[0][2] * enemy_x + matrix[1][2] * enemy_y + matrix[2][2] * enemy_z + matrix[3][2];
		float ratio = (float)1 / camera_z;
		screen_position_x = (matrix[0][0] * enemy_x + matrix[1][0] * enemy_y + matrix[2][0] * enemy_z + matrix[3][0]) * ratio * (float)view_port_width;
		screen_position_x = (matrix[0][1] * enemy_x + matrix[1][1] * enemy_y + matrix[2][1] * enemy_z + matrix[3][1]) * ratio * (float)view_port_height;
		std::cout << "camera_x: " << screen_position_x << std::endl;
		std::cout << "camera_y: " << screen_position_x << std::endl;


		Sleep(100);
	}
}