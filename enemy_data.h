#pragma once
#include <windows.h>
#include "memory_util.h"
#include <string>
#include <iostream>

int playerNums = 0;
int enemyNums = 0;
std::string player_names[16] = {};

struct Point {
	float x;
	float y;
};

struct EnemyPosition {
	Point head;
	Point foot;
	EnemyPosition() {
		head = { 0, 0 };
		foot = { 0, 0 };
	}
	EnemyPosition(Point a, Point b) {
		head = a;
		foot = b;
	}
};
EnemyPosition positions[20] = {};

// client.dll+FC94C 1 represents terrorist, 2 represents counter-terrorist.
int playerRole = 1;

int GetPlayerRole(int processId, HANDLE process) {
	char* clientBase = GetModuleBase(L"client.dll", processId);
	int role = RPM<int>(process, (DWORD)clientBase + 0xFC94C);
	return role;
}

Point GetScreenPosition(float matrix[4][4], float x, float y, float z, float view_port_width, float view_port_height) {
	float screen_position_x;
	float screen_position_y;
	float clip_x = matrix[0][0] * x + matrix[1][0] * y + matrix[2][0] * z + matrix[3][0];
	float clip_y = matrix[0][1] * x + matrix[1][1] * y + matrix[2][1] * z + matrix[3][1];
	float clip_z = matrix[0][2] * x + matrix[1][2] * y + matrix[2][2] * z + matrix[3][2];
	float clip_w = matrix[0][3] * x + matrix[1][3] * y + matrix[2][3] * z + matrix[3][3];

	if (clip_w < 0.1f)
		return {0 ,0};
	float NDC_x = clip_x / clip_w;
	float NDC_y = clip_y / clip_w;
	float NDC_z = clip_z / clip_w;
	screen_position_x = (view_port_width * NDC_x) + (NDC_x + view_port_width);
	screen_position_y = -(view_port_height * NDC_y) + (NDC_y + view_port_height);
	return { screen_position_x, screen_position_y };
}

void GetAllPlayerNames(int processId, HANDLE process) {
	int i = 0;
	while (true) {
		char name[10];
		ReadProcessMemory(process, (LPCVOID)(0x5AFFFAC + 0x250 * i), &name, 10, NULL);
		if (name[0] == '\0') {
			playerNums = i;
			return;
		}
		else {
			player_names[i] = name;
			i++;
		}
	}
	for (int i = 0; playerNums; i++) {
		std::cout << player_names[i] << std::endl;
	}
}

void GetAllEnemyPosition(int processId) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	// Base address of player list is hw.dll+7cd154.
	char* dllBase = GetModuleBase(L"hw.dll", processId);
	DWORD listBase = RPM<DWORD>(process, (DWORD)dllBase + 0x7cd154);
	float matrix[4][4] = { 0 };

	RECT rect;
	HWND hwnd = FindWindowA(NULL, "Counter-Strike");
	GetWindowRect(hwnd, &rect);

	int view_port_width = (rect.right - rect.left) / 2;
	int view_port_height = (rect.bottom - rect.top) / 2;

	while (true) {
		int myRole = GetPlayerRole(processId, process);
		GetAllPlayerNames(processId, process);
		enemyNums = 0;

		for (int i = 0; i < playerNums; i++) {
			int v1 = RPM<DWORD>(process, (DWORD)listBase + 0x7c + 0x324 * i);
			int v2 = RPM<DWORD>(process, (DWORD)v1 + 0x4);
			int v3 = RPM<DWORD>(process, (DWORD)v2 + 0x320);
			int role = RPM<DWORD>(process, (DWORD)v3 + 0x1C8);
			ReadProcessMemory(process, (LPCVOID)((DWORD)dllBase + 0xE956A0), &matrix, 16 * sizeof(float), NULL);

			if (role != myRole) {
				float x = RPM<float>(process, (DWORD)listBase + 0x468 + 0x324 * i);
				float y = RPM<float>(process, (DWORD)listBase + 0x46C + 0x324 * i);
				float z = RPM<float>(process, (DWORD)listBase + 0x470 + 0x324 * i);
				float head_z = RPM<float>(process, (DWORD)listBase + 0x47C + 0x324 * i);

				Point screen_H = GetScreenPosition(matrix, x, y, head_z, view_port_width, view_port_height);
				Point screen_F = GetScreenPosition(matrix, x, y, z, view_port_width, view_port_height);
				positions[enemyNums] = { screen_H, screen_F };
				enemyNums++;
			} 

		}
	}
}
