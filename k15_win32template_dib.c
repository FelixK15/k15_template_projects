#define _CRT_SECURE_NO_WARNINGS

//#define K15_GREYSCALE

#include <windows.h>
#include <stdio.h>

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "Gdi32.lib")

#define K15_FALSE 0
#define K15_TRUE 1

typedef unsigned char bool8;
typedef unsigned char byte;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

void printErrorToFile(const char* pFileName)
{
	DWORD errorId = GetLastError();
	char* textBuffer = 0;
	DWORD writtenChars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, errorId, 
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPSTR)&textBuffer, 512, 0);

	if (writtenChars > 0)
	{
		FILE* file = fopen(pFileName, "w");

		if (file)
		{
			fwrite(textBuffer, writtenChars, 1, file);			
			fflush(file);
			fclose(file);
		}
	}
}

void allocateDebugConsole()
{
	AllocConsole();
	AttachConsole(ATTACH_PARENT_PROCESS);
	freopen("CONOUT$", "w", stdout);
}

#ifdef K15_GREYSCALE
uint8* pBackBufferPixels = 0;
#else
uint32* pBackBufferPixels = 0;
#endif

BITMAPINFO* pBackBufferBitmapInfo = 0;
HBITMAP backBufferBitmap = 0;

int screenWidth = 800;
int screenHeight = 600;

void createBackBuffer(HWND hwnd, int width, int height)
{		
	if (pBackBufferBitmapInfo != NULL)
	{
		free(pBackBufferBitmapInfo);
		pBackBufferBitmapInfo = NULL;
		pBackBufferPixels = NULL;
	}

	if (backBufferBitmap != NULL)
	{
		DeleteObject(backBufferBitmap);
		backBufferBitmap = NULL;
	}

#ifdef K15_GREYSCALE
	pBackBufferBitmapInfo = malloc( sizeof(BITMAPINFO) + sizeof(RGBQUAD) * 256 );
	RGBQUAD* pRGBValues = (RGBQUAD*)((uint8*)pBackBufferBitmapInfo + sizeof(BITMAPINFO));

	for(int i = 0; i < 256; ++i)
	{
		pRGBValues[i].rgbRed 	= (uint8)i;
		pRGBValues[i].rgbBlue 	= (uint8)i;
		pRGBValues[i].rgbGreen 	= (uint8)i;
		pRGBValues[i].rgbReserved = 0;
	}

	pBackBufferBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
	pBackBufferBitmapInfo->bmiHeader.biWidth = width;
	pBackBufferBitmapInfo->bmiHeader.biHeight = -(int)height;
	pBackBufferBitmapInfo->bmiHeader.biPlanes = 1;
	pBackBufferBitmapInfo->bmiHeader.biBitCount = 8;
	pBackBufferBitmapInfo->bmiHeader.biCompression = BI_RGB;
#else
	pBackBufferBitmapInfo = malloc( sizeof(BITMAPINFO) );
	pBackBufferBitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
	pBackBufferBitmapInfo->bmiHeader.biWidth = width;
	pBackBufferBitmapInfo->bmiHeader.biHeight = -(int)height;
	pBackBufferBitmapInfo->bmiHeader.biPlanes = 1;
	pBackBufferBitmapInfo->bmiHeader.biBitCount = 32;
	pBackBufferBitmapInfo->bmiHeader.biCompression = BI_RGB;
	//FK: XRGB
#endif

	HDC deviceContext = GetDC(hwnd);
	backBufferBitmap = CreateDIBSection( deviceContext, pBackBufferBitmapInfo, DIB_RGB_COLORS, &pBackBufferPixels, NULL, 0 );   
	if (backBufferBitmap == NULL && pBackBufferPixels == NULL)
	{
		MessageBoxA(0, "Error during CreateDIBSection.", "Error!", 0);
	}

	screenWidth = width;
	screenHeight = height;
}

void K15_WindowCreated(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

}

void K15_WindowClosed(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

}

void K15_KeyInput(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

}

void K15_MouseButtonInput(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

}

void K15_MouseMove(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

}

void K15_MouseWheel(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{

}

void K15_WindowResized(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	WORD newWidth = (WORD)(lparam);
	WORD newHeight = (WORD)(lparam >> 16);

	createBackBuffer(hwnd, newWidth, newHeight);
}

LRESULT CALLBACK K15_WNDPROC(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	bool8 messageHandled = K15_FALSE;

	switch (message)
	{
	case WM_CREATE:
		K15_WindowCreated(hwnd, message, wparam, lparam);
		break;

	case WM_CLOSE:
		K15_WindowClosed(hwnd, message, wparam, lparam);
		PostQuitMessage(0);
		messageHandled = K15_TRUE;
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		K15_KeyInput(hwnd, message, wparam, lparam);
		break;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		K15_MouseButtonInput(hwnd, message, wparam, lparam);
		break;

	case WM_MOUSEMOVE:
		K15_MouseMove(hwnd, message, wparam, lparam);
		break;

	case WM_MOUSEWHEEL:
		K15_MouseWheel(hwnd, message, wparam, lparam);
		break;

	case WM_SIZE:
		K15_WindowResized(hwnd, message, wparam, lparam);
		break;
	}

	if (messageHandled == K15_FALSE)
	{
		return DefWindowProc(hwnd, message, wparam, lparam);
	}

	return 0;
}

HWND setupWindow(HINSTANCE instance, int width, int height)
{
	WNDCLASS wndClass = {0};
	wndClass.style = CS_HREDRAW | CS_OWNDC | CS_VREDRAW;
	wndClass.hInstance = instance;
	wndClass.lpszClassName = "K15_Win32Template";
	wndClass.lpfnWndProc = K15_WNDPROC;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wndClass);

	HWND hwnd = CreateWindowA("K15_Win32Template", "Win32 Template",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		width, height, 0, 0, instance, 0);

	if (hwnd == INVALID_HANDLE_VALUE)
	{
		MessageBox(0, "Error creating Window.\n", "Error!", 0);
	}
	else
	{
		ShowWindow(hwnd, SW_SHOW);
		createBackBuffer(hwnd, width, height);
	}
	return hwnd;
}

uint32 getTimeInMilliseconds(LARGE_INTEGER PerformanceFrequency)
{
	LARGE_INTEGER appTime = {0};
	QueryPerformanceFrequency(&appTime);

	appTime.QuadPart *= 1000; //to milliseconds

	return (uint32)(appTime.QuadPart / PerformanceFrequency.QuadPart);
}

void setup()
{
}

void drawBackBuffer(HWND hwnd)
{
	HDC deviceContext = GetDC( hwnd );
	StretchDIBits( deviceContext, 0, 0, screenWidth, screenHeight, 0, 0, screenWidth, screenHeight, 
		pBackBufferPixels, pBackBufferBitmapInfo, DIB_RGB_COLORS, SRCCOPY );  

#ifdef K15_GREYSCALE
	memset(pBackBufferPixels, 0, screenWidth * screenHeight);
#else
	memset(pBackBufferPixels, 0, screenWidth * screenHeight * sizeof(int));
#endif
}

void doFrame(uint32 DeltaTimeInMS)
{

}

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nShowCmd)
{
	LARGE_INTEGER performanceFrequency;
	QueryPerformanceFrequency(&performanceFrequency);

	allocateDebugConsole();

	HWND hwnd = setupWindow(hInstance, 800, 600);

	if (hwnd == INVALID_HANDLE_VALUE)
	{
		return -1;
	}

	setup();

	uint32 timeFrameStarted = 0;
	uint32 timeFrameEnded = 0;
	uint32 deltaMs = 0;

	bool8 loopRunning = K15_TRUE;
	MSG msg = {0};

	while (loopRunning)
	{
		timeFrameStarted = getTimeInMilliseconds(performanceFrequency);

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if (msg.message == WM_QUIT)
			{
				loopRunning = K15_FALSE;
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (!loopRunning)
		{
			break;
		}

		doFrame(deltaMs);
		drawBackBuffer(hwnd);

		timeFrameEnded = getTimeInMilliseconds(performanceFrequency);
		deltaMs = timeFrameEnded - timeFrameStarted;
	}

	DestroyWindow(hwnd);

	return 0;
}