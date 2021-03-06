//
// keyboard simulation
//

#include "stdafx.h"
#include "kbsim.h"

#define MAX_LOADSTRING 100

#define SHIFT_DOWN        1
#define SHIFT_UP          2

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

//
HWND hButton1, hButton2, hButton3, hButton4;
char* buffer;
int n;

HWND hToolTip;

//
HANDLE hThread;
HANDLE hStopEvent;
DWORD dwThreadId;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK    WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI        ThreadFunc(LPVOID lpParam);

int Paste(HWND hWnd, char **buffer);

void GetSimCode(char ch, WORD *vk, WORD *shift);

void OnPaint(HWND hWnd);
void OnCreate(HWND hWnd);
void OnDestroy(HWND hWnd);

void OnExit(HWND hWnd);

void OnPlay(HWND hWnd);
void OnStop(HWND hWnd);
void OnPaste(HWND hWnd);

// main window
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_KBSIM, szWindowClass, MAX_LOADSTRING);

    //  Registers the window class.
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KBSIM));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // Store instance handle in our global variable.
    hInst = hInstance; 

    // Create the main program window.
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_CAPTION,
		399, 399, 210, 90, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return FALSE;

    // Display the main program window.
    ShowWindow(hWnd, nCmdShow);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    UpdateWindow(hWnd);

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_KBSIM));

    MSG msg;

    // Main message loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//  Processes messages for the main window.
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDM_EXIT:	   OnExit(hWnd);						break;
		case IDC_BUTTON1:  OnPlay(hWnd);						break;
		case IDC_BUTTON2:  OnStop(hWnd);						break;
		case IDC_BUTTON3:  OnPaste(hWnd);						break;
		case IDC_BUTTON4:  OnExit(hWnd);						break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
        break;
	case WM_PAINT:   OnPaint(hWnd);									break;
	case WM_CREATE:  OnCreate(hWnd);								break;
	case WM_DESTROY: OnDestroy(hWnd);								break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

//
DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
	wchar_t szText[100];
	DWORD dwWaitResult, dwTimeOut;
	int i;
	WORD shift_status;
	WORD vk, shift;
	INPUT inp;

	EnableWindow(hButton1, FALSE);
	EnableWindow(hButton2, TRUE);
	EnableWindow(hButton3, FALSE);
	EnableWindow(hButton4, FALSE);

	// display thread status to output window
	swprintf_s(szText, 100, L"THE THREAD 0x%x HAS STARTED.\n", dwThreadId);
	OutputDebugString(szText);

    // Pause for 3 seconds.
    Sleep(3000);

	inp.type           = INPUT_KEYBOARD;
	inp.ki.dwExtraInfo = 0;
	inp.ki.dwFlags     = 0;
	inp.ki.time        = 0;
	inp.ki.wScan       = 0;
	inp.ki.wVk         = 0;

	inp.ki.wVk     = VK_SHIFT;
	inp.ki.dwFlags = 0;
	SendInput(1, &inp, sizeof(INPUT));

	shift_status = SHIFT_DOWN;

	dwTimeOut = 50;
	i = 0;
	while (i<n)
	{

	   if(buffer[i] == 13 && buffer[i+1] == 10) // carriage return (13, 10)
	   {
		   // release SHIFT key
		   	if(shift_status != SHIFT_UP)
			{
				inp.ki.wVk     = VK_SHIFT;

				inp.ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(1, &inp, sizeof(INPUT));

				shift_status = SHIFT_UP;
			}

			// Press ENTER key
			inp.ki.wVk     = VK_RETURN;
			inp.ki.dwFlags = 0;
			SendInput(1, &inp, sizeof(INPUT));

			// Release ENTER key
			inp.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &inp, sizeof(INPUT));

			// Press HOME key
			inp.ki.wVk     = VK_HOME;
			inp.ki.dwFlags = 0;
			SendInput(1, &inp, sizeof(INPUT));

			// Release HOME key
			inp.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &inp, sizeof(INPUT));

		   i += 2;
	   }
	   else
	   {
			// get keyboard equivalent
			GetSimCode(buffer[i], &vk, &shift);

			// SHIFT key
			if(shift_status != shift)
			{
				if(shift == SHIFT_DOWN)
				{
					inp.ki.wVk     = VK_SHIFT;
					inp.ki.dwFlags = 0;
					SendInput(1, &inp, sizeof(INPUT));

					shift_status = SHIFT_DOWN;
				}

				if(shift == SHIFT_UP)
				{
					inp.ki.wVk     = VK_SHIFT;
					inp.ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput(1, &inp, sizeof(INPUT));

					shift_status = SHIFT_UP;
				}
			}

			// Press tkey
			inp.ki.wVk     = vk;
			inp.ki.dwFlags = 0;
			SendInput(1, &inp, sizeof(INPUT));

			// Release key
			inp.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &inp, sizeof(INPUT));

			++i;

	   }

		// exit loop if event handle is set
		dwWaitResult = WaitForSingleObject(hStopEvent, dwTimeOut);
		if (dwWaitResult == WAIT_OBJECT_0) break;
	}

   // release SHIFT key
	if(shift_status == SHIFT_DOWN)
	{
		inp.ki.wVk     = VK_SHIFT;
		inp.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &inp, sizeof(INPUT));
	}

	// display thread status to output window
	swprintf_s(szText, 100, L"THE THREAD 0x%x HAS STOPED.\n", dwThreadId);
	OutputDebugString(szText);

	EnableWindow(hButton1, TRUE);
	EnableWindow(hButton2, FALSE);
	EnableWindow(hButton3, TRUE);
	EnableWindow(hButton4, TRUE);

	return 0;
}

// ilagay ang laman ng clipboard sa buffer, ito ay null terminated
int Paste(HWND hWnd, char **buffer)
{
	HGLOBAL   hglb;
	LPSTR lpstr;
	int i, n;

	n = 0;

	if (!IsClipboardFormatAvailable(CF_TEXT))
	{
		OutputDebugString(L"HINDI TEXT ANG LAMAN NG CLIPBOARD\n");
		return 0;
	}

	if (!OpenClipboard(hWnd)) 
	{
		OutputDebugString(L"DI KO MA-OPEN ANG CLIPBOARD\n");
		return 0;
	}


	if((hglb = GetClipboardData(CF_TEXT)) == NULL)
	{
		OutputDebugString(L"WALANG LAMAN ANG CLIPBOARD\n");
		return 0;
	}

	lpstr = (LPSTR)GlobalLock(hglb); 
 
	n = strlen(lpstr);

	// i-release bago mag-allocate
	if(*buffer != NULL)
	{
		delete[] *buffer;
		*buffer = NULL;
	}

	*buffer = new char[n + 1];

	// kopyahin sa ibang variable (buffer)
	// kasi lpstr ay para sa clipboard pedeng mawala
	for(i=0;i<n;++i)
		*(*buffer + i) = lpstr[i];

	*(*buffer + n) = '\0';  // null terminated

	GlobalUnlock(hglb);

	CloseClipboard();

	return n;
}

//
void GetSimCode(char ch, WORD *vk, WORD *shift)
{
	switch(ch)
	{
	case 'A': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'B': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'C': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'D': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'E': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'F': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'G': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'H': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'I': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'J': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'K': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'L': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'M': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'N': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'O': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'P': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'Q': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'R': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'S': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'T': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'U': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'V': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'W': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'X': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'Y': *vk = ch; *shift = SHIFT_DOWN; break;
	case 'Z': *vk = ch; *shift = SHIFT_DOWN; break;

	case 'a': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'b': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'c': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'd': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'e': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'f': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'g': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'h': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'i': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'j': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'k': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'l': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'm': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'n': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'o': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'p': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'q': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'r': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 's': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 't': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'u': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'v': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'w': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'x': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'y': *vk = toupper(ch); *shift = SHIFT_UP; break;
	case 'z': *vk = toupper(ch); *shift = SHIFT_UP; break;

	case ' ': *vk = ch; *shift = SHIFT_UP; break;
	case '0': *vk = ch; *shift = SHIFT_UP; break;
	case '1': *vk = ch; *shift = SHIFT_UP; break;
	case '2': *vk = ch; *shift = SHIFT_UP; break;
	case '3': *vk = ch; *shift = SHIFT_UP; break;
	case '4': *vk = ch; *shift = SHIFT_UP; break;
	case '5': *vk = ch; *shift = SHIFT_UP; break;
	case '6': *vk = ch; *shift = SHIFT_UP; break;
	case '7': *vk = ch; *shift = SHIFT_UP; break;
	case '8': *vk = ch; *shift = SHIFT_UP; break;
	case '9': *vk = ch; *shift = SHIFT_UP; break;

	case '!': *vk = '1'; *shift = SHIFT_DOWN; break;
	case '@': *vk = '2'; *shift = SHIFT_DOWN; break;
	case '#': *vk = '3'; *shift = SHIFT_DOWN; break;
	case '$': *vk = '4'; *shift = SHIFT_DOWN; break;
	case '%': *vk = '5'; *shift = SHIFT_DOWN; break;
	case '^': *vk = '6'; *shift = SHIFT_DOWN; break;
	case '&': *vk = '7'; *shift = SHIFT_DOWN; break;
	case '*': *vk = '8'; *shift = SHIFT_DOWN; break;
	case '(': *vk = '9'; *shift = SHIFT_DOWN; break;
	case ')': *vk = '0'; *shift = SHIFT_DOWN; break;

	case '`':  *vk = VK_OEM_3;      *shift = SHIFT_UP; break;
	case '-':  *vk = VK_OEM_MINUS;  *shift = SHIFT_UP; break;
	case '=':  *vk = VK_OEM_PLUS;   *shift = SHIFT_UP; break;
	case '[':  *vk = VK_OEM_4;      *shift = SHIFT_UP; break;
	case ']':  *vk = VK_OEM_6;      *shift = SHIFT_UP; break;
	case '\\': *vk = VK_OEM_5;      *shift = SHIFT_UP; break;
	case ';':  *vk = VK_OEM_1;      *shift = SHIFT_UP; break;
	case '\'': *vk = VK_OEM_7;      *shift = SHIFT_UP; break;
	case ',':  *vk = VK_OEM_COMMA;  *shift = SHIFT_UP; break;
	case '.':  *vk = VK_OEM_PERIOD; *shift = SHIFT_UP; break;
	case '/':  *vk = VK_OEM_2;      *shift = SHIFT_UP; break;

	case '~':  *vk = VK_OEM_3;      *shift = SHIFT_DOWN; break;
	case '_':  *vk = VK_OEM_MINUS;  *shift = SHIFT_DOWN; break;
	case '+':  *vk = VK_OEM_PLUS;   *shift = SHIFT_DOWN; break;
	case '{':  *vk = VK_OEM_4;      *shift = SHIFT_DOWN; break;
	case '}':  *vk = VK_OEM_6;      *shift = SHIFT_DOWN; break;
	case '|':  *vk = VK_OEM_5;      *shift = SHIFT_DOWN; break;
	case ':':  *vk = VK_OEM_1;      *shift = SHIFT_DOWN; break;
	case '"':  *vk = VK_OEM_7;      *shift = SHIFT_DOWN; break;
	case '<':  *vk = VK_OEM_COMMA;  *shift = SHIFT_DOWN; break;
	case '>':  *vk = VK_OEM_PERIOD; *shift = SHIFT_DOWN; break;
	case '?':  *vk = VK_OEM_2;      *shift = SHIFT_DOWN; break;

	case  9 :  *vk = VK_TAB;        *shift = SHIFT_UP;   break;

	}
}

//
void OnPaint(HWND hWnd)
{
	PAINTSTRUCT ps;
	HDC hDC;

	hDC = BeginPaint(hWnd, &ps);
	EndPaint(hWnd, &ps);
}

//
void OnCreate(HWND hWnd)
{
	hButton1 = CreateWindowEx(0, L"BUTTON",L"", WS_CHILD | BS_BITMAP | WS_VISIBLE, 10, 10, 30, 25, hWnd, (HMENU)IDC_BUTTON1, hInst, NULL);
	hButton2 = CreateWindowEx(0 ,L"BUTTON",L"", WS_CHILD | BS_BITMAP | WS_VISIBLE, 45, 10, 30, 25, hWnd, (HMENU)IDC_BUTTON2, hInst, NULL);
	hButton3 = CreateWindowEx(0, L"BUTTON",L"", WS_CHILD | BS_BITMAP | WS_VISIBLE, 80, 10, 30, 25, hWnd, (HMENU)IDC_BUTTON3, hInst, NULL);
	hButton4 = CreateWindowEx(0, L"BUTTON",L"", WS_CHILD | BS_BITMAP | WS_VISIBLE,150, 10, 30, 25, hWnd, (HMENU)IDC_BUTTON4, hInst, NULL);

	HANDLE hBitmap;

	hBitmap = LoadImage(NULL, L"play.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	SendMessageW(hButton1, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

	hBitmap = LoadImage(NULL, L"stop.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	SendMessageW(hButton2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

	hBitmap = LoadImage(NULL, L"paste.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	SendMessageW(hButton3, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

	hBitmap = LoadImage(NULL, L"power.bmp", IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE | LR_LOADFROMFILE);
	SendMessageW(hButton4, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hBitmap);

	EnableWindow(hButton1, FALSE);
	EnableWindow(hButton2, FALSE);
	EnableWindow(hButton3, TRUE);
	EnableWindow(hButton4, TRUE);

	// initialize handle
	hThread    = NULL;
	hStopEvent = NULL;

	// create event
	hStopEvent = CreateEvent(NULL,  // no security attributes
		TRUE,                       // manual-reset event(TRUE-manual FALSE-auto)
		FALSE,                      // initial state is non-signaled (TRUE-signal FALSE-nonsignal)
		L"kbsimstopevent"); // object name

	// error checking, display in output window
    if (hStopEvent == NULL) 
        OutputDebugString(L"Create event failed.\n");

}

//
void OnDestroy(HWND hWnd)
{
	// close all handles
	if(hThread != NULL)    CloseHandle(hThread);
	if(hStopEvent != NULL) CloseHandle(hStopEvent);

	// i-release bago mag-exit
	if(buffer != NULL)
		delete[] buffer;

	// exit program
	PostQuitMessage(0);
}

//
void OnExit(HWND hWnd)
{
	DestroyWindow(hWnd);
}

// start simulation
void OnPlay(HWND hWnd)
{
	// reset event handle to prevent stopping the loop
	ResetEvent(hStopEvent);

	// close thread handle
	if(hThread != NULL) CloseHandle(hThread);

	// create thread
	hThread = CreateThread(NULL,         // no security attributes 
						   0,            // use default stack size 
						   ThreadFunc,   // thread function 
						   NULL,         // argument to thread function 
						   0,            // use default creation flags 
						   &dwThreadId); // returns the thread identifier 

	// error checking display in output window
   if (hThread == NULL) 
      OutputDebugString(L"Create thread failed.\n" ); 

}

// stop simulation
void OnStop(HWND hWnd)
{
	SetEvent(hStopEvent);
}

//
void OnPaste(HWND hWnd)
{
	n = Paste(hWnd, &buffer);

	if (n == 0)
	{
		EnableWindow(hButton1, FALSE);
		EnableWindow(hButton2, FALSE);
		EnableWindow(hButton3, TRUE);
	}
	else
	{
		EnableWindow(hButton1, TRUE);
		EnableWindow(hButton2, FALSE);
		EnableWindow(hButton3, TRUE);
	}
}

//
