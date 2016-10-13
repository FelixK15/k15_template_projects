#define _GNU_SOURCE 1

#include "stdio.h"
#include "time.h"
#include "string.h"
#include "unistd.h"
#include "X11/Xlib.h"
#include "GL/gl.h"
#include "GL/glx.h"

#define K15_FALSE 0
#define K15_TRUE 1

#define	GLX_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB           0x2092
#define GLX_CONTEXT_FLAGS_ARB                   0x2094
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001

typedef unsigned char bool8;
typedef unsigned char byte;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

Display* mainDisplay = 0;
GC mainGC;
Atom deleteMessage = 0;
int nanoSecondsPerFrame = 16000000;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);

void handleKeyPress(XEvent* p_Event)
{
}

void handleKeyRelease(XEvent* p_Event)
{
}

void handleButtonPress(XEvent* p_Event)
{
}

void handleButtonRelease(XEvent* p_Event)
{
}

void handleMouseMotion(XEvent* p_Event)
{
}

void handleWindowResize(XEvent* p_Event)
{
	int width = p_Event->xconfigure.width;
	int height = p_Event->xconfigure.height;
}

bool8 filterEvent(XEvent* p_Event)
{
	if (p_Event->type == KeyRelease)
	{
		if (XEventsQueued(mainDisplay, QueuedAfterReading))
		{
			XEvent tempEvent;
			XPeekEvent(mainDisplay, &tempEvent);

			if (tempEvent.type == KeyPress && tempEvent.xkey.time == p_Event->xkey.time &&
				tempEvent.xkey.keycode == p_Event->xkey.keycode)
			{
				XNextEvent(mainDisplay, &tempEvent);
				return K15_TRUE;
			}
		}
	}


	return K15_FALSE;
}

void handleEvent(XEvent* p_Event)
{
	if (filterEvent(p_Event))
		return;

	if (p_Event->type == KeyPress)
		handleKeyPress(p_Event);
	else if (p_Event->type == KeyRelease)
		handleKeyRelease(p_Event);
	else if (p_Event->type == ButtonPress)
		handleButtonPress(p_Event);
	else if (p_Event->type == ButtonRelease)
		handleButtonRelease(p_Event);
	else if (p_Event->type == MotionNotify)
		handleMouseMotion(p_Event);
	else if (p_Event->type == ConfigureNotify)
		handleWindowResize(p_Event);
}

int errorHandler(Display* p_Display, XErrorEvent* p_Event)
{
	uint32 errorCode = p_Event->error_code;
	char errorMessage[256];
	XGetErrorText(p_Display, errorCode, errorMessage, 256);

	printf("Error (%d): '%s'\n", errorCode, errorMessage);

	return 0;
}

void setupWindow(Window* p_WindowOut, int p_Width, int p_Height)
{
	int borderWidth = 1;
	int eventMask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | 
		KeyPressMask | KeyReleaseMask | StructureNotifyMask;

	Window window = XCreateSimpleWindow(mainDisplay, XRootWindow(mainDisplay, XDefaultScreen(mainDisplay)),
		0, 0, p_Width, p_Height, 0, 0, 0);

	*p_WindowOut = window;

	mainGC = XCreateGC(mainDisplay, window, 0, 0);
	XSetForeground(mainDisplay, mainGC, (~0));
	XStoreName(mainDisplay, window, "Test Window!");
	XSelectInput(mainDisplay, window, eventMask);
	XSetWMProtocols(mainDisplay, window, &deleteMessage, 1);
	XMapWindow(mainDisplay, window);
}

bool8 setup(Window* p_WindowOut)
{
	XSetErrorHandler(errorHandler);
	mainDisplay = XOpenDisplay(0);

	if (mainDisplay)
	{
		deleteMessage = XInternAtom(mainDisplay, "WM_DELETE_WINDOW", False);
		setupWindow(p_WindowOut, 800, 600);
		return K15_TRUE;
	}

	return K15_FALSE;
}

void drawDeltaTime(Window* p_MainWindow, long p_DeltaTimeInNs)
{
	char buffer[256];
	sprintf(buffer, "Milliseconds:%ld", p_DeltaTimeInNs / 1000000);

	XDrawString(mainDisplay, *p_MainWindow, mainGC, 20, 20, buffer, strlen(buffer));
}

void doFrame(Window* p_MainWindow, long p_DeltaTimeInNs)
{
	XClearWindow(mainDisplay, *p_MainWindow);
	drawDeltaTime(p_MainWindow, p_DeltaTimeInNs);
	XFlush(mainDisplay);
	XSync(mainDisplay, *p_MainWindow);
}

int main(int argc, char** argv)
{
	Window mainWindow;
	setup(&mainWindow);

	struct timespec timeFrameStarted = {0};
	struct timespec timeFrameEnded = {0};
	long deltaNs = 0;

	bool8 loopRunning = K15_TRUE;
	XEvent event = {0};
	while (loopRunning)
	{
		clock_gettime(CLOCK_MONOTONIC, &timeFrameStarted);
		while (XPending(mainDisplay))
		{
			XNextEvent(mainDisplay, &event);

			if (event.type == ClientMessage && 
				event.xclient.data.l[0] == deleteMessage)
			{
				loopRunning = K15_FALSE;
			} 

			handleEvent(&event);
		}

		doFrame(&mainWindow, deltaNs);

		clock_gettime(CLOCK_MONOTONIC, &timeFrameEnded);

		deltaNs = timeFrameEnded.tv_nsec - timeFrameStarted.tv_nsec;

		if (deltaNs < nanoSecondsPerFrame)
		{
			struct timespec sleepTime = {0, nanoSecondsPerFrame - deltaNs};
			nanosleep(&sleepTime, 0);
		}
	}

	return 0;
}