#include "stdafx.h"
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <assert.h>

using namespace std;

LONG Xtotal = 0;
LONG Ytotal = 0;
HANDLE firstmouse;
LONG tempcounter= 0;

LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void OnRawInput(bool inForeground, HRAWINPUT hRawInput);
void SetupRawInput();

//The user-provided entry point for a graphical Windows-based application
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd){
	WNDCLASSEX wClass;
	HWND hWnd;

	ZeroMemory(&wClass, sizeof(WNDCLASSEX));
	wClass.cbClsExtra = NULL;
	wClass.cbSize = sizeof(WNDCLASSEX);
	wClass.cbWndExtra = NULL;
	wClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wClass.hIcon = NULL;
	wClass.hIconSm = NULL;
	wClass.hInstance = hInst;
	wClass.lpfnWndProc = (WNDPROC)WinProc;
	wClass.lpszClassName = "Window Class";
	wClass.lpszMenuName = NULL;
	wClass.style = CS_HREDRAW | CS_VREDRAW;

	if (!RegisterClassEx(&wClass)) {
		int nResult = GetLastError();
		MessageBox (NULL,
			"Window class creation failed",
			"Window Class Failed",
			MB_ICONERROR);
	}

	hWnd = CreateWindowEx (NULL,"Window Class","Windows application",
		WS_OVERLAPPEDWINDOW, 200, 20, 640, 380, NULL, NULL, hInst, NULL);

	if (!hWnd){
		int nResult = GetLastError(); 
		
		MessageBox(NULL,"Window creation failed",
			"Window creation failed", MB_ICONERROR);
	}
	
	ShowWindow(hWnd, nShowCmd);
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));
	AllocConsole(); // Creates a new console process when an error occurs while interaction
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	printf("Debugging Window. . . \n");
	
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}


LRESULT CALLBACK WinProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam){
	

	switch (msg){

		case WM_CREATE:{
			printf("WM_Create Called!");
		   //Registiring the Mouse
			RAWINPUTDEVICE rid;
			rid.usUsagePage = 0x01;
			rid.usUsage = 0x02;			   
			rid.dwFlags = RIDEV_INPUTSINK;
			rid.hwndTarget = hWnd;
			wprintf (L"This is the value of %d", sizeof(RAWINPUTDEVICE));
			printf("This is the value of %d", sizeof(RAWINPUTDEVICE));
				if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
					return -1;
				else
					printf("success for create WM \n");
				}
				return 0;

		case WM_DESTROY:{
			printf("success for create wm\n");
			PostQuitMessage(0);
			return 0;
			}

		case WM_INPUT:{
			OnRawInput(GET_RAWINPUT_CODE_WPARAM(wParam) == RIM_INPUT,
				(HRAWINPUT)lParam);
			break;
			}
		
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam); //calling DefWindowProc
		//break;
	}
	
}


std::vector<char> m_RawInputMessageData; // Buffer
void OnRawInput(bool inForeground, HRAWINPUT hRawInput){
	UINT dataSize;
	GetRawInputData(hRawInput, RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
	
	if (dataSize == 0)
		return;
	if (dataSize > m_RawInputMessageData.size())
		m_RawInputMessageData.resize(dataSize);

	void* dataBuf = &m_RawInputMessageData[0];
	GetRawInputData(hRawInput, RID_INPUT, dataBuf, &dataSize, sizeof(RAWINPUTHEADER));

	const RAWINPUT *raw = (const RAWINPUT*)dataBuf;
	if (raw->header.dwType == RIM_TYPEMOUSE){
		HANDLE deviceHandle = raw->header.hDevice;
		const RAWMOUSE& mouseData = raw->data.mouse;
		USHORT flags = MOUSE_MOVE_ABSOLUTE;
		short wheelDelta = (short)mouseData.usButtonData;
		LONG x = mouseData.lLastX, y = mouseData.lLastY;
			if (tempcounter == 0){
				firstmouse = deviceHandle;
				tempcounter++;
			}
			if (firstmouse != deviceHandle){
				Xtotal += x;
				Ytotal += y;
				wprintf(L"Mouse: Device=0x%08X, Flags=%04x, WheelDelta=%d, X=%d, Y=%d\n",
					deviceHandle, flags, wheelDelta, Xtotal, Ytotal);
			}
	}

	/*
	UINT numDevices;
	GetRawInputDeviceList(
	NULL, &numDevices, sizeof(RAWINPUTDEVICELIST));
	if (numDevices == 0) return;

	std::vector<RAWINPUTDEVICELIST> deviceList(numDevices);
	GetRawInputDeviceList(
	&deviceList[0], &numDevices, sizeof(RAWINPUTDEVICELIST));

	std::vector<wchar_t> deviceNameData;
	wstring deviceName;
	for (UINT i = 0; i < numDevices; ++i)
	{
	const RAWINPUTDEVICELIST& device = deviceList[i];
	if (device.dwType == RIM_TYPEMOUSE)
	{
	wprintf(L"Mouse: Handle=0x%08X\n", device.hDevice);

	UINT dataSize;
	GetRawInputDeviceInfo(
	device.hDevice, RIDI_DEVICENAME, nullptr, &dataSize);
	if (dataSize)
	{
	deviceNameData.resize(dataSize);
	UINT result = GetRawInputDeviceInfo(
	device.hDevice, RIDI_DEVICENAME, &deviceNameData[0], &dataSize);
	if (result != UINT_MAX)
	{
	deviceName.assign(deviceNameData.begin(), deviceNameData.end());
	wprintf(L"Name=%s\n", deviceName.c_str());
	}
	}

	RID_DEVICE_INFO deviceInfo;
	deviceInfo.cbSize = sizeof deviceInfo;
	dataSize = sizeof deviceInfo;
	UINT result = GetRawInputDeviceInfo(
	device.hDevice, RIDI_DEVICEINFO, &deviceInfo, &dataSize);
	if (result != UINT_MAX)
	{
	assert(deviceInfo.dwType == RIM_TYPEMOUSE);
	wprintf(
	L"  Id=%u, Buttons=%u, SampleRate=%u, HorizontalWheel=%s\n",
	deviceInfo.mouse.dwId,
	deviceInfo.mouse.dwNumberOfButtons,
	deviceInfo.mouse.dwSampleRate,
	deviceInfo.mouse.fHasHorizontalWheel ? L"1" : L"0");
	}
	}
	}
	*/
}