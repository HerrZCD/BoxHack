#pragma once

#include <windows.h>
#include <dwmapi.h>
#include "d3d.h"

MARGINS g_MARGINS;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        Cleanup();
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        ValidateRect(hWnd, NULL);
        return 0;
    }
    Render();
    return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND CreateTransparentWindow(HWND game) {
	const char* window_name = "chundong's window";
	WNDCLASSEXA window_class;
	ZeroMemory(&window_class, sizeof(window_class));
	window_class.cbSize = sizeof(window_class);
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	window_class.hInstance = GetModuleHandle(NULL);
	window_class.lpszClassName = window_name;
	window_class.style = CS_VREDRAW | CS_HREDRAW;
    window_class.lpfnWndProc = WndProc;
    window_class.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	RegisterClassExA(&window_class);
    RECT rect;
    HWND hwnd = FindWindowA(NULL, "Counter-Strike");
    GetWindowRect(hwnd, &rect);

    int view_port_width = rect.right - rect.left;
    int view_port_height = rect.bottom - rect.top;
    HWND window = CreateWindowExA(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, window_name, window_name, WS_POPUP, 0, 0, 1280, 960, NULL, NULL, GetModuleHandle(NULL), NULL);
    SetLayeredWindowAttributes(window, 0, RGB(0, 1, 0), LWA_COLORKEY);
    ShowWindow(window, SW_SHOW);
	UpdateWindow(window);
    InitD3D(window);
	return window;
}

void MessageHandle(HWND game, HWND window) {
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT) {
        RECT rect;
        HWND hwnd = FindWindowA(NULL, "Counter-Strike");
        GetWindowRect(hwnd, &rect);
        MoveWindow(window, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
		if (PeekMessageA(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}
	}
}