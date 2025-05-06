#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <time.h>
#include <tlhelp32.h>
#include <tchar.h>
#include <shobjidl.h> 
#include <shlwapi.h>
#include <strsafe.h>

#include <string>


#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "resource.h"
#include "Helper.h"


#define ID_SECONDS_FIELD 101 
#define ID_MINUTES_FIELD 102 // Not Implemented
#define ID_EXETEXT_FIELD 103

#define ID_SELECT_BUTTON 201
#define ID_START_BUTTON 202
#define ID_STOP_BUTTON 203

#define ID_TIMER 301

const std::wstring _iniFileName_ = L"settings.ini";


HWND _hwnd_;

HBRUSH _bgBrushTimerRunning_;
HBRUSH _bgBrushTimerStopped_;

bool _isRunning_ = false;
int _durationInSec_ = 0;

std::wstring _exeName_ = L"";
DWORD _exeProcessID_ = 0;

std::wstring _iniPath_;


void SaveToIni();
void SetFieldsFromIni(HWND);


LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	std::wstring startString = L"Starting wWinMain(...)\n";
	OutputDebugString(startString.c_str());

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
	{
		OutputDebugString(L"CoInitializeEx(...) failed.\n");
		return 0;
	}

	const wchar_t CLASS_NAME[] = L"Unsavoury Auto Save";

	WNDCLASS wc = {};
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(ID_GEAR_ICON));

	RegisterClass(&wc); 

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Unsavoury Auto Save",
		WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 400, 225,
		NULL, NULL, hInstance, NULL
	);

	if (hwnd == NULL)
		return 0;

	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CoUninitialize();
	return 0;
}



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
	case WM_CREATE:


		_isRunning_ = false;

		_bgBrushTimerRunning_ = CreateSolidBrush(RGB(128, 128, 128));
		_bgBrushTimerStopped_ = CreateSolidBrush(RGB(224, 224, 224));

		CreateTextField(hwnd, ID_EXETEXT_FIELD, 50, 25, 150, 25);

		CreateTextFieldNumeric(hwnd, ID_SECONDS_FIELD, 50, 125, 100, 25);
		
		CreateWindowsButton(hwnd, L"Select EXE", ID_SELECT_BUTTON, 50, 60, 100, 30);
		
		CreateWindowsButton(hwnd, L"Start", ID_START_BUTTON, 250, 25, 100, 30);
		
		CreateWindowsButton(hwnd, L"Stop", ID_STOP_BUTTON, 250, 75, 100, 30);

		_iniPath_ = GetOwnExeDirectory() + L"\\" + _iniFileName_;
		SetFieldsFromIni(hwnd);

		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_SELECT_BUTTON:
		
			_exeName_ = GetFileNameFromDialog();
			SetTextField(hwnd, ID_EXETEXT_FIELD, _exeName_);
			OutputDebugString((L"Exe selected: " + _exeName_ + L"\n").c_str());
		
		case ID_START_BUTTON:

			KillTimer(hwnd, ID_TIMER);

			_durationInSec_ = ReadIntFromTextField(hwnd, ID_SECONDS_FIELD);
			if (_durationInSec_ < 1)
				_durationInSec_ = 1;

			SetTimer(hwnd, ID_TIMER, _durationInSec_ * 1000, NULL);
						
			wchar_t exeNameTextBuffer[256];
			GetWindowText(GetDlgItem(hwnd, ID_EXETEXT_FIELD), exeNameTextBuffer, sizeof(exeNameTextBuffer)/sizeof(wchar_t));
			_exeName_ = exeNameTextBuffer;

			SaveToIni();

			_isRunning_ = true;
			break;

		case ID_STOP_BUTTON:
			KillTimer(hwnd, ID_TIMER);
			_isRunning_ = false;
			break;
		}

		InvalidateRect(hwnd, NULL, TRUE);

		return 0;

	case WM_TIMER: 
	{
		bool targetExeIsActive;
		DWORD targetProcessID = ProcessIDByExeName(_exeName_);
		if (targetProcessID != 0 && targetProcessID == ProcessIDOfForegroundWindow())
			targetExeIsActive = true;
		else
			targetExeIsActive = false;

		if (targetExeIsActive)
		{
			SendCtrlSInput();
			InvalidateRect(hwnd, NULL, TRUE);
		}
		
		int durationToNextTimer;
		if (targetExeIsActive)
			durationToNextTimer = _durationInSec_;
		else if(_durationInSec_ / 10 < 5)
			durationToNextTimer = 5;
		else
			durationToNextTimer = _durationInSec_ / 10;

		SetTimer(hwnd, ID_TIMER, durationToNextTimer * 1000, NULL);
		return 0;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		if (_isRunning_)
			FillRect(hdc, &ps.rcPaint, _bgBrushTimerRunning_);
		else
			FillRect(hdc, &ps.rcPaint, _bgBrushTimerStopped_);

		SetTextColor(hdc, RGB(0, 0, 0));
		SetBkMode(hdc, TRANSPARENT);

		const wchar_t* text = L"Seconds between Ctrl-S:";
		TextOut(hdc, 50, 100, text, wcslen(text));

		if (_isRunning_)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			std::wstring textString = L"Last Ctrl-S at: \n\t";
			std::wstring timeString = SystemTimeToString(&st);
			TextOut(hdc, 250, 125, textString.c_str(), wcslen(textString.c_str()));
			TextOut(hdc, 250, 150, timeString.c_str(), wcslen(timeString.c_str()));
		}

		EndPaint(hwnd, &ps);
		return 0;
	}

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0); 
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);

}



void SaveToIni()
{
	bool writeExeNameSuccess = 
		WritePrivateProfileString(L"Settings", L"ExeFileName", _exeName_.c_str(), _iniPath_.c_str());
	bool writeDurationSuccess = 
		WritePrivateProfileString(L"Settings", L"DurationInSeconds", std::to_wstring(_durationInSec_).c_str(), _iniPath_.c_str());
	if (!writeExeNameSuccess || !writeDurationSuccess)
		MessageBox(_hwnd_, L"Could not save settings.", L"Error", 0);
}

void SetFieldsFromIni(HWND hwnd)
{
	wchar_t exeFileNameBuffer[MAX_PATH];
	GetPrivateProfileStringW(L"Settings", L"ExeFileName", L"", exeFileNameBuffer, MAX_PATH, _iniPath_.c_str());
	_exeName_ = exeFileNameBuffer;
	SetTextField(hwnd, ID_EXETEXT_FIELD, _exeName_);

	wchar_t durationTextBuffer[16];
	GetPrivateProfileStringW(L"Settings", L"DurationInSeconds", L"60", durationTextBuffer, 16, _iniPath_.c_str());
	_durationInSec_ = _wtoi(durationTextBuffer);
	SetNumericalTextField(hwnd, ID_SECONDS_FIELD, _durationInSec_);
}
