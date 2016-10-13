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
int mainScreen = 0;
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
	glViewport(0, 0, width, height);
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

	printf("Error (%d): '%s'", errorCode, errorMessage);

	return 0;
}

void setupWindow(Window* p_WindowOut, int p_Width, int p_Height)
{
	int borderWidth = 1;
	int eventMask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | 
		KeyPressMask | KeyReleaseMask | StructureNotifyMask;
	static int visualAttributes[] = {
			GLX_X_RENDERABLE, True,
			GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
			GLX_RENDER_TYPE, GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
			GLX_RED_SIZE, 8,
			GLX_GREEN_SIZE, 8,
			GLX_BLUE_SIZE, 8,
			GLX_ALPHA_SIZE, 8,
			GLX_DEPTH_SIZE, 24,
			GLX_STENCIL_SIZE, 8,
			GLX_DOUBLEBUFFER, True,
			None
	};

	int fbCount = 0;
	GLXFBConfig* fbc = glXChooseFBConfig(mainDisplay, XDefaultScreen(mainDisplay), visualAttributes, &fbCount);

	if (!fbc)
	{
		printf("Failed to retrieve framebuffer config through glXChooseFBConfig.\n");
	}

	GLXFBConfig frameBufferConfig = fbc[0];
	XFree(fbc);

	XVisualInfo* vi = glXGetVisualFromFBConfig(mainDisplay, frameBufferConfig);

	XSetWindowAttributes setWindowAttributes = {0};
	setWindowAttributes.colormap = XCreateColormap(mainDisplay, XRootWindow(mainDisplay, vi->screen), vi->visual, AllocNone);
	setWindowAttributes.background_pixmap = None;
	setWindowAttributes.border_pixel = 0;
	setWindowAttributes.event_mask = StructureNotifyMask;

	Window window = XCreateSimpleWindow(mainDisplay, XRootWindow(mainDisplay, vi->screen),
		0, 0, p_Width, p_Height, 0, 0, 0);

	XFree(vi);

	*p_WindowOut = window;

	mainGC = XCreateGC(mainDisplay, window, 0, 0);
	XSetForeground(mainDisplay, mainGC, (~0));
	XStoreName(mainDisplay, window, "Test Window!");
	XSelectInput(mainDisplay, window, eventMask);
	XSetWMProtocols(mainDisplay, window, &deleteMessage, 1);

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB("glXCreateContextAttribsARB");

	GLXContext ctx = 0;

	int contextAttributes[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 3,
		GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
		None
	};

	ctx = glXCreateContextAttribsARB(mainDisplay, frameBufferConfig, 0, True, contextAttributes);
	XSync(mainDisplay, False);

	glXMakeCurrent(mainDisplay, window, ctx);
	glViewport(0, 0, p_Width, p_Height);
	glClearColor(0.f, 0.f, 0.f, 1.f);

	printf("OpenGL intialized!\nGL_VERSION: '%s'\nGL_RENDERER: '%s'\nGL_VENDOR: '%s'.\n", 
		glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));

	XMapWindow(mainDisplay, window);
}

bool8 setup(Window* p_WindowOut)
{
	XSetErrorHandler(errorHandler);
	mainDisplay = XOpenDisplay(0);

	if (mainDisplay)
	{
		mainScreen = XDefaultScreen(mainDisplay);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	drawDeltaTime(p_MainWindow, p_DeltaTimeInNs);
	glXSwapBuffers(mainDisplay, *p_MainWindow);
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