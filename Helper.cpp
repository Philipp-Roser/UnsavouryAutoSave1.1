#include <windows.h>
#include <wchar.h>
#include <string>
#include <Shlwapi.h>
#include <TlHelp32.h>

#include "Helper.h"
#include <ShObjIdl.h>


void CreateWindowsButtonW
(HWND parentHWND, std::wstring buttonText, unsigned int button_id, int xPos, int yPos, int width, int height)
{
	CreateWindow(
		L"Button",
		buttonText.c_str(),
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		xPos, yPos, width, height,
		parentHWND,
		(HMENU)button_id,
		(HINSTANCE)GetWindowLongPtr(parentHWND, GWLP_HINSTANCE),
		NULL
	);
}

void CreateTextFieldW
(HWND parentHWND, unsigned long text_field_id, int xPos, int yPos, int width, int height, std::wstring defaultText)
{
	CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		defaultText.c_str(),
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
		xPos, yPos, width, height,
		parentHWND,
		(HMENU)text_field_id,
		(HINSTANCE)GetWindowLongPtr(parentHWND, GWLP_HINSTANCE),
		NULL
	);
}

void CreateTextFieldNumericW
(HWND parentHWND, unsigned long text_field_id, int xPos, int yPos, int width, int height, std::wstring defaultText)
{
	CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",
		defaultText.c_str(),
		WS_CHILD | WS_VISIBLE | ES_NUMBER | ES_AUTOHSCROLL,
		xPos, yPos, width, height,
		parentHWND,
		(HMENU)text_field_id,
		(HINSTANCE)GetWindowLongPtr(parentHWND, GWLP_HINSTANCE),
		NULL
	);
}


DWORD ProcessIDOfForegroundWindow()
{
	HWND foreground_hwnd = GetForegroundWindow();
	DWORD processID = 0;
	GetWindowThreadProcessId(foreground_hwnd, &processID);

	return processID;
}


std::wstring ReadTextField256(HWND hwnd, int field_id)
{
	wchar_t buffer[256];
	GetDlgItemText(hwnd, field_id, buffer, sizeof(buffer) / sizeof(wchar_t));

	return std::wstring(buffer);
}


void SetTextField(HWND hwnd, int field_id, std::wstring text)
{
	SetWindowText(GetDlgItem(hwnd, field_id), text.c_str());
}


std::wstring SystemTimeToString(SYSTEMTIME* st)
{
	wchar_t timeString[16];
	swprintf(timeString, sizeof(timeString) / sizeof(wchar_t),
		L"%02d:%02d:%02d\n", st->wHour, st->wMinute, st->wSecond);
	return timeString;
}


void SendCtrlSInput()
{
	INPUT input[4] = {};

	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = VK_CONTROL;

	input[1].ki.wVk = 'S';
	input[1].type = INPUT_KEYBOARD;

	input[2].type = INPUT_KEYBOARD;
	input[2].ki.wVk = 'S';
	input[2].ki.dwFlags = KEYEVENTF_KEYUP;

	input[3].type = INPUT_KEYBOARD;
	input[3].ki.wVk = VK_CONTROL;
	input[3].ki.dwFlags = KEYEVENTF_KEYUP;

	SendInput(4, input, sizeof(INPUT));

	return;
}

DWORD ProcessIDByExeName(std::wstring exeName)
{
	PROCESSENTRY32 pe32 = {}; // PROCESSENTRY32 is a struct that contains info about running processes
	pe32.dwSize = sizeof(PROCESSENTRY32); // required for windows to know what type of struct this is

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // creates snapshot of all currently running processes
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	DWORD processID = 0;
	if (Process32First(hSnapshot, &pe32)) // pe32 struct is given info on first process
	{
		do {
			if (_wcsicmp(pe32.szExeFile, exeName.c_str()) == 0) // string compmarison (wide strings, case ignored)
			{
				processID = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32)); // iterate through processes until we hit zero (end of list)
	}

	CloseHandle(hSnapshot);

	return processID;
}


std::wstring GetFileNameFromDialog()
{
	std::wstring fileName;

	IFileDialog* pFileDialog;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileDialog));

	if (FAILED(hr))
		return L"";

	COMDLG_FILTERSPEC filter[] = { {L"Executable Files", L"*.exe", }, {L"All Files", L"*.*"} };

	pFileDialog->SetFileTypes(ARRAYSIZE(filter), filter);
	pFileDialog->SetTitle(L"Select EXE");

	hr = pFileDialog->Show(NULL); // This throws an exception

	if (SUCCEEDED(hr))
	{
		IShellItem* pItem;
		hr = pFileDialog->GetResult(&pItem);

		if (SUCCEEDED(hr))
		{
			PWSTR pszFilePath;
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath); // pszFilePath is now desired file path
			if (SUCCEEDED(hr))
			{
				fileName = PathFindFileName(pszFilePath);

				CoTaskMemFree(pszFilePath);
			}
			pItem->Release();
		}
	}

	pFileDialog->Release();

	return fileName;
}


void SetNumericalTextField(HWND hwnd, int field_id, int value)
{
	wchar_t valueBuffer[16];
	swprintf(valueBuffer, sizeof(valueBuffer) / sizeof(wchar_t), L"%d", value);
	SetWindowText(GetDlgItem(hwnd, field_id), valueBuffer);
}


std::wstring GetOwnExeDirectory()
{
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);
	PathRemoveFileSpec(exePath);
	return exePath;
}


int ReadIntFromTextField(HWND hwnd, int text_field_id)
{
	wchar_t buffer[32];
	GetDlgItemText(hwnd, text_field_id, buffer, sizeof(buffer) / sizeof(wchar_t));

	return _wtoi(buffer);
}