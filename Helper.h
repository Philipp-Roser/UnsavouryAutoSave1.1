#pragma once

#include <string>

#if UNICODE
#define CreateWindowsButton CreateWindowsButtonW
#endif
void CreateWindowsButtonW
(HWND parentHWND, std::wstring buttonText, unsigned int button_id, int xPos, int yPos, int width, int height);

#if UNICODE 
#define CreateTextField CreateTextFieldW
#endif 
void CreateTextFieldW
(HWND parentHWND, unsigned long text_field_id, int xPos, int yPos, int width, int height, std::wstring defaultText = L"");


#if UNICODE 
#define CreateTextFieldNumeric CreateTextFieldNumericW
#endif
void CreateTextFieldNumericW
(HWND parentHWND, unsigned long text_field_id, int xPos, int yPos, int width, int height, std::wstring defaultText = L"");


std::wstring ReadTextField256(HWND hwnd, int field_id);
void SetTextField(HWND hwnd, int field_id, std::wstring text);

int ReadIntFromTextField(HWND hwnd, int field_id);
void SetNumericalTextField(HWND hwnd, int field_id, int value);

std::wstring SystemTimeToString(SYSTEMTIME*);

void SendCtrlSInput();

std::wstring GetFileNameFromDialog();
std::wstring GetOwnExeDirectory();
DWORD ProcessIDOfForegroundWindow();
DWORD ProcessIDByExeName(std::wstring);
