#pragma once
#include <windows.h>
#include "memory_util.h"
#include <string>
#include <iostream>
float screen_position_x = 0.0f;
float screen_position_y = 0.0f;
float screen_position_foot_x = 0.0f;
float screen_position_foot_y = 0.0f;
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

				Point screen_H = GetScreenPosition(matrix, x, y, z + 30, view_port_width, view_port_height);
				Point screen_F = GetScreenPosition(matrix, x, y, z - 30, view_port_width, view_port_height);
				positions[enemyNums] = { screen_H, screen_F };
				enemyNums++;
			} 

		}
	}
}

void GetEnemyPosition(int processId) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, processId);
	char* dllBase = GetModuleBase(L"hw.dll", processId);
	char* clientBase = GetModuleBase(L"client.dll", processId);
	char* exeBase = GetModuleBase(L"cstrike.exe", processId);
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
		DWORD x1 = RPM<DWORD>(process, (DWORD)dllBase + 0x007BBD9C + 0x250);
		float v5 = RPM<float>(process, v4 + 0x8);
		float enemy_z = RPM<float>(process, (DWORD)e1 + 0x3B4);
		float enemy_y = RPM<float>(process, (DWORD)e1 + 0x3B0);
		float enemy_x = RPM<float>(process, (DWORD)e1 + 0x3AC);

		float enemy_z1 = RPM<float>(process, (DWORD)x1 + 0x3B4);
		float enemy_y1 = RPM<float>(process, (DWORD)x1 + 0x3B0);
		float enemy_x1 = RPM<float>(process, (DWORD)x1 + 0x3AC);
		system("cls");
		std::cout << "Player postion x: " << (float)v5 << std::endl;
		std::cout << "enemy postion x: " << (float)enemy_x << std::endl;
		std::cout << "enemy postion y: " << (float)enemy_y << std::endl;
		std::cout << "enemy postion z: " << (float)enemy_z << std::endl;


		std::cout << "enemy postion x: " << (float)enemy_x1 << std::endl;
		std::cout << "enemy postion y: " << (float)enemy_y1 << std::endl;
		std::cout << "enemy postion z: " << (float)enemy_z1 << std::endl;
		ReadProcessMemory(process, (LPCVOID)((DWORD)dllBase + 0xE956A0), &matrix, 16 * sizeof(float), NULL);
		std::cout << "********************************************" << std::endl;
		std::cout << matrix[0][0] << " | " << matrix[0][1] << " | " << matrix[0][2] << " | " << matrix[0][3] << std::endl;
		std::cout << matrix[1][0] << " | " << matrix[1][1] << " | " << matrix[1][2] << " | " << matrix[1][3] << std::endl;
		std::cout << matrix[2][0] << " | " << matrix[2][1] << " | " << matrix[2][2] << " | " << matrix[2][3] << std::endl;
		std::cout << matrix[3][0] << " | " << matrix[3][1] << " | " << matrix[3][2] << " | " << matrix[3][3] << std::endl;
		std::cout << "********************************************" << std::endl;

		Point screen_H = GetScreenPosition(matrix, enemy_x, enemy_y, enemy_z + 30, view_port_width, view_port_height);
		Point screen_F = GetScreenPosition(matrix, enemy_x, enemy_y, enemy_z  - 30, view_port_width, view_port_height);
		screen_position_x = screen_H.x;
		screen_position_y = screen_H.y;
		screen_position_foot_x = screen_F.x;
		screen_position_foot_y = screen_F.y;


		/*float clip_x =  matrix[0][0] * enemy_x + matrix[0][1] * enemy_y + matrix[0][2] * enemy_z + matrix[0][3];
		float clip_y = matrix[1][0] * enemy_x + matrix[1][1] * enemy_y + matrix[1][2] * enemy_z + matrix[1][3];
		float clip_z = matrix[2][0] * enemy_x + matrix[2][1] * enemy_y + matrix[2][2] * enemy_z + matrix[2][3];
		float clip_w =  matrix[3][0] * enemy_x + matrix[3][1] * enemy_y + matrix[3][2] * enemy_z + matrix[3][3];*/

		
		std::cout << "camera_x: " << screen_position_x << std::endl;
		std::cout << "camera_y: " << screen_position_y << std::endl;
	}
}