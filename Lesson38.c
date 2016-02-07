/**************************************
*                                     *
*      Jeff Molofee's Lesson 38       *
*          nehe.gamedev.net           *
*                2002                 *
*      Modified for LCCWin32          *
*compiler by Robert Wishlaw 2002/11/24*
**************************************/

/**         Comments manageable by Doxygen
*
*  Modified smoothly by Thierry DECHAIZE
*
*  Paradigm : obtain one source (only one !) compatible for multiple free C Compilers
*    and provide for all users an development environment on Windows (64 bits compatible),
*    the great Code::Blocks manager (version 13.12), and don't use glaux.lib or glaux.dll.
*
*	 a) Mingw 32 and 64 bits : downloadable http://sourceforge.net/projects/mingw/
*    b) Mingw 32 DEV-CPP  : included in old Dev-Cpp http://sourceforge.net/projects/devcpp/
*    c) Mingw 32 CB : included in package Code::Blocks (version 13.12) http://sourceforge.net/projects/codeblocks/files/Binaries/13.12/Windows/
*    d) Borland C (version 5.5) : downloadable  http://edn.embarcadero.com/article/20633
*    e) Digital Mars Compiler C : version 8.57 downloadable http://www.digitalmars.com
*    f) OpenWatcom : version 1.9 downloadable http://openwatcom.mirror.fr/    Version 2.0 64bits seems very buggy !!!
*	 g) Lcc and Lcc64 : downloadable http://www.cs.virginia.edu/~lcc-win32/
*	 h) Cygwin : downloadable http://www.cygwin.com/install.html (setup.exe)
*	 i) Visual Studio 2005 + 64bits: express edition for free (my version 2005 is full not express) http://www.microsoft.com/france/visual-studio/essayez/express.aspx
*    j) TDM GCC 64 bits : downloadable on http://sourceforge.net/projects/TDM-GCC
*
*  Important remark : All install in C partition, "no space" and "no special characters" in the name of directories
*            (it's reason to not install in C:\Program Files (x86) ... by default proposed on windows 64 bits)
*
*  Add resource file end resource header for restitute version + icon OpenGL.ico for fun
*  because versionning is important, same for freeware :-) !
*
*  Date : 2016/01/09
*
* \file            Lesson38.c
* \author          Jeff Molofee (NeHe) originely, Modified for LCCWin32 compiler by Robert Wishlaw 2002/11/24
* \author          After adapted by Thierry Dechaize to verify paradigm : one source for multiple C Compilers
* \version         1.0.1.2
* \date            9 janvier 2016
* \brief           Ouvre une fenêtre Windows et dessine des papillons descendants générés à partir d'images BMP définies dans un fichier de ressources (très poètiques).
* \details         Ce programme ne gére que deux événements : le clic sur le bouton "Fermé" de la fenêtre, la sortie par la touche ESC.
*
*
*/

#if defined __CYGWIN__ || defined __LCC__
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>			// Header File For Windows
#include <stdlib.h>             // Header File For "rand" function (mandatory LCC !!!)
#if defined(__LCC__)
#ifndef WINGDIAPI
#   define WINGDIAPI __stdcall
#endif
#endif

#include <gl\gl.h>				// Header File For The OpenGL32 Library
#include <gl\glu.h>				// Header File For The GLu32 Library
#include <gl\glext.h>		    // Header File For The GLext Library
/* #include <gl\glaux.h>		// Header File For The Glaux Library  */

#if defined __GNUC__ || defined __LCC__ || defined _MSC_VER || defined __TURBOC__
#include <stdbool.h>
#endif

#ifdef __WATCOMC__
#include <stdbool.h>
// typedef BOOLEAN bool;
#endif

#if defined __DMC__
typedef BOOL bool;
#endif


#ifdef __DMC__
#define GWLP_USERDATA (-21)                                             // Digital Mars Compiler don't declare GWLP_USERDATA
#ifndef LONG_PTR
#if defined(_WIN64)
typedef __int64 LONG_PTR;
#else
typedef long LONG_PTR;
#endif
#endif
#endif

#include "resource.h"											// Header File For Resource (*IMPORTANT*)

#ifndef CDS_FULLSCREEN											// CDS_FULLSCREEN Is Not Defined By Some
#define CDS_FULLSCREEN 4										// Compilers. By Defining It This Way,
#endif															// We Can Avoid Errors


typedef struct {									// Structure For Keyboard Stuff
	BOOL keyDown [256];								// Holds TRUE / FALSE For Each Key
} Keys;												// Keys

typedef struct {									// Contains Information Vital To Applications
	HINSTANCE		hInstance;						// Application Instance
	const char*		className;						// Application ClassName
} Application;										// Application

typedef struct {									// Window Creation Info
	Application*		application;				// Application Structure
	char*				title;						// Window Title
	int					width;						// Width
	int					height;						// Height
	int					bitsPerPixel;				// Bits Per Pixel
	BOOL				isFullScreen;				// FullScreen?
} GL_WindowInit;									// GL_WindowInit

typedef struct {									// Contains Information Vital To A Window
	Keys*				keys;						// Key Structure
	HWND				hWnd;						// Window Handle
	HDC					hDC;						// Device Context
	HGLRC				hRC;						// Rendering Context
	GL_WindowInit		init;						// Window Init
	BOOL				isVisible;					// Window Visible?
	DWORD				lastTickCount;				// Tick Counter
} GL_Window;										// GL_Window

void TerminateApplication(GL_Window* window);		// Terminate The Application

void ToggleFullscreen(GL_Window* window);			// Toggle Fullscreen / Windowed Mode

// These Are The Function You Must Provide

BOOL InitializeGL(GL_Window* window, Keys* keys);	// Performs All Your Initialization

void DeinitializeGL(void);							// Performs All Your DeInitialization

void UpdateGL(DWORD milliseconds);					// Perform Motion Updates

void DrawGL(void);									// Perform All Your Scene Drawing

GL_Window*	g_window;
Keys*		g_keys;

// User Defined Variables
GLuint texture[3];										// Storage For 3 Textures

#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;											// If TRUE, Then Create Fullscreen

typedef struct _OBJECT											// Create A Structure Called Object
{
	int   tex;													// Integer Used To Select Our Texture
	float x;													// X Position
	float y;													// Y Position
	float z;													// Z Position
	float yi;													// Y Increase Speed (Fall Speed)
	float spinz;												// Z Axis Spin
	float spinzi;												// Z Axis Spin Speed
	float flap;													// Flapping Triangles :)
	float fi;													// Flap Direction (Increase Value)
}OBJECT, *LPOBJECT;

static OBJECT   obj[50];										// Create 50 Objects Using The Object Structure

void SetObject(int loop)										// Sets The Initial Value Of Each Object (Random)
{
	obj[loop].tex=rand()%3;										// Texture Can Be One Of 3 Textures
	obj[loop].x=rand()%34-17.0f;								// Random x Value From -17.0f To 17.0f
	obj[loop].y=18.0f;											// Set y Position To 18 (Off Top Of Screen)
	obj[loop].z=-((rand()%30000/1000.0f)+10.0f);				// z Is A Random Value From -10.0f To -40.0f
	obj[loop].spinzi=(rand()%10000)/5000.0f-1.0f;				// spinzi Is A Random Value From -1.0f To 1.0f
	obj[loop].flap=0.0f;										// flap Starts Off At 0.0f;
	obj[loop].fi=0.05f+(rand()%100)/1000.0f;					// fi Is A Random Value From 0.05f To 0.15f
	obj[loop].yi=0.001f+(rand()%1000)/10000.0f;					// yi Is A Random Value From 0.001f To 0.101f
}

void LoadGLTextures()											// Creates Textures From Bitmaps In The Resource File
{
	HBITMAP hBMP;												// Handle Of The Bitmap
	BITMAP	BMP;												// Bitmap Structure
	int loop=0;

	// The ID Of The 3 Bitmap Images We Want To Load From The Resource File
	BYTE	Texture[]={	IDB_BUTTERFLY1,	IDB_BUTTERFLY2,	IDB_BUTTERFLY3 };

	glGenTextures(sizeof(Texture), &texture[0]);				// Generate 3 Textures (sizeof(Texture)=3 ID's)

	for (loop=0; loop<sizeof(Texture); loop++)				// Loop Through All The ID's (Bitmap Images)
	{
		hBMP=(HBITMAP)LoadImage(GetModuleHandle(NULL),MAKEINTRESOURCE(Texture[loop]), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
		if (hBMP)												// Does The Bitmap Exist?
		{														// If So...
			GetObject(hBMP,sizeof(BMP), &BMP);					// Get The Object
																// hBMP: Handle To Graphics Object
																// sizeof(BMP): Size Of Buffer For Object Information
																// Buffer For Object Information
			glPixelStorei(GL_UNPACK_ALIGNMENT,4);				// Pixel Storage Mode (Word Alignment / 4 Bytes)
			glBindTexture(GL_TEXTURE_2D, texture[loop]);		// Bind Our Texture
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// Linear Filtering
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR); // Mipmap Linear Filtering

			// Generate Mipmapped Texture (3 Bytes, Width, Height And Data From The BMP)
			gluBuild2DMipmaps(GL_TEXTURE_2D, 3, BMP.bmWidth, BMP.bmHeight, GL_BGR_EXT, GL_UNSIGNED_BYTE, BMP.bmBits);
			DeleteObject(hBMP);									// Delete The Bitmap Object
		}
	}
}

void TerminateApplication(GL_Window* window)							// Terminate The Application
{
	PostMessage (window->hWnd, WM_QUIT, 0, 0);							// Send A WM_QUIT Message
	g_isProgramLooping = FALSE;											// Stop Looping Of The Program
}

void ToggleFullscreen(GL_Window* window)								// Toggle Fullscreen/Windowed
{
	PostMessage (window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				// Send A WM_TOGGLEFULLSCREEN Message
}

void ReshapeGL (int width, int height)									// Reshape The Window When It's Moved Or Resized
{
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				// Reset The Current Viewport
	glMatrixMode (GL_PROJECTION);										// Select The Projection Matrix
	glLoadIdentity ();													// Reset The Projection Matrix
	gluPerspective (45.0f, (GLfloat)(width)/(GLfloat)(height),			// Calculate The Aspect Ratio Of The Window
					1.0f, 1000.0f);
	glMatrixMode (GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity ();													// Reset The Modelview Matrix
}

BOOL InitializeGL(GL_Window* window, Keys* keys)					// Any GL Init Code & User Initialiazation Goes Here
{
	int         loop=0;

	g_window	= window;
	g_keys		= keys;

	// Start Of User Initialization
	LoadGLTextures();											// Load The Textures From Our Resource File

	glClearColor (0.0f, 0.0f, 0.0f, 0.5f);						// Black Background
	glClearDepth (1.0f);										// Depth Buffer Setup
	glDepthFunc (GL_LEQUAL);									// The Type Of Depth Testing (Less Or Equal)
	glDisable(GL_DEPTH_TEST);									// Disable Depth Testing
	glShadeModel (GL_SMOOTH);									// Select Smooth Shading
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);			// Set Perspective Calculations To Most Accurate
	glEnable(GL_TEXTURE_2D);									// Enable Texture Mapping
	glBlendFunc(GL_ONE,GL_SRC_ALPHA);							// Set Blending Mode (Cheap / Quick)
	glEnable(GL_BLEND);											// Enable Blending

	for (loop=0; loop<50; loop++)							// Loop To Initialize 50 Objects
	{
		SetObject(loop);										// Call SetObject To Assign New Random Values
	}

	return TRUE;												// Return TRUE (Initialization Successful)
}

void DeinitializeGL(void)										// Any User DeInitialization Goes Here
{
}

void UpdateGL(DWORD milliseconds)								// Perform Motion Updates Here
{
	if (g_keys->keyDown [VK_ESCAPE] == TRUE)					// Is ESC Being Pressed?
	{
		TerminateApplication (g_window);						// Terminate The Program
	}

	if (g_keys->keyDown [VK_F1] == TRUE)						// Is F1 Being Pressed?
	{
		ToggleFullscreen (g_window);							// Toggle Fullscreen Mode
	}
}

void DrawGL(void)												// Draw The Scene
{
    int loop=0;

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer

	for (loop=0; loop<50; loop++)							    // Loop Of 50 (Draw 50 Objects)
	{
		glLoadIdentity ();										// Reset The Modelview Matrix
		glBindTexture(GL_TEXTURE_2D, texture[obj[loop].tex]);	// Bind Our Texture
		glTranslatef(obj[loop].x,obj[loop].y,obj[loop].z);		// Position The Object
		glRotatef(45.0f,1.0f,0.0f,0.0f);						// Rotate On The X-Axis
		glRotatef((obj[loop].spinz),0.0f,0.0f,1.0f);			// Spin On The Z-Axis

		glBegin(GL_TRIANGLES);									// Begin Drawing Triangles
			// First Triangle														    _____
			glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);				//	(2)|    / (1)
			glTexCoord2f(0.0f,1.0f); glVertex3f(-1.0f, 1.0f, obj[loop].flap);	//	   |  /
			glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);				//	(3)|/

			// Second Triangle
			glTexCoord2f(1.0f,1.0f); glVertex3f( 1.0f, 1.0f, 0.0f);				//	       /|(1)
			glTexCoord2f(0.0f,0.0f); glVertex3f(-1.0f,-1.0f, 0.0f);				//	     /  |
			glTexCoord2f(1.0f,0.0f); glVertex3f( 1.0f,-1.0f, obj[loop].flap);	//	(2)/____|(3)

		glEnd();												// Done Drawing Triangles

		obj[loop].y-=obj[loop].yi;								// Move Object Down The Screen
		obj[loop].spinz+=obj[loop].spinzi;						// Increase Z Rotation By spinzi
		obj[loop].flap+=obj[loop].fi;							// Increase flap Value By fi

		if (obj[loop].y<-18.0f)									// Is Object Off The Screen?
		{
			SetObject(loop);									// If So, Reassign New Values
		}

		if ((obj[loop].flap>1.0f) || (obj[loop].flap<-1.0f))	// Time To Change Flap Direction?
		{
			obj[loop].fi=-obj[loop].fi;							// Change Direction By Making fi = -fi
		}
	}

	Sleep(15);													// Create A Short Delay (15 Milliseconds)

	glFlush ();													// Flush The GL Rendering Pipeline
}

BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel)	// Change The Screen Resolution
{
	DEVMODE dmScreenSettings;											// Device Mode
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));					// Make Sure Memory Is Cleared
	dmScreenSettings.dmSize				= sizeof (DEVMODE);				// Size Of The Devmode Structure
	dmScreenSettings.dmPelsWidth		= width;						// Select Screen Width
	dmScreenSettings.dmPelsHeight		= height;						// Select Screen Height
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;					// Select Bits Per Pixel
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													// Display Change Failed, Return False
	}
	return TRUE;														// Display Change Was Successful, Return True
}

BOOL CreateWindowGL (GL_Window* window)									// This Code Creates Our OpenGL Window
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							// Define Our Window Style
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						// Define The Window's Extended Style

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		0,                      										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		0,																// No Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	GLuint PixelFormat;													// Will Hold The Selected Pixel Format
	RECT windowRect;                                                    // Define Our Window Coordinates

    pfd.cColorBits = window->init.bitsPerPixel;

    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = window->init.width;
    windowRect.bottom = window->init.height;

	if (window->init.isFullScreen == TRUE)								// Fullscreen Requested, Try Changing Video Modes
	{
		if (ChangeScreenResolution (window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			// Fullscreen Mode Failed.  Run In Windowed Mode Instead
			MessageBox (HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							// Set isFullscreen To False (Windowed Mode)
		}
		else															// Otherwise (If Fullscreen Mode Was Successful)
		{
			ShowCursor (FALSE);											// Turn Off The Cursor
			windowStyle = WS_POPUP;										// Set The WindowStyle To WS_POPUP (Popup Window)
			windowExtendedStyle |= WS_EX_TOPMOST;						// Set The Extended Window Style To WS_EX_TOPMOST
		}																// (Top Window Covering Everything Else)
	}
	else																// If Fullscreen Was Not Selected
	{
		// Adjust Window, Account For Window Borders
		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	// Create The OpenGL Window
	window->hWnd = CreateWindowEx (windowExtendedStyle,					// Extended Style
								   window->init.application->className,	// Class Name
								   window->init.title,					// Window Title
								   windowStyle,							// Window Style
								   0, 0,								// Window X,Y Position
								   windowRect.right - windowRect.left,	// Window Width
								   windowRect.bottom - windowRect.top,	// Window Height
								   HWND_DESKTOP,						// Desktop Is Window's Parent
								   0,									// No Menu
								   window->init.application->hInstance, // Pass The Window Instance
								   window);

	if (window->hWnd == 0)												// Was Window Creation A Success?
	{
		return FALSE;													// If Not Return False
	}

	window->hDC = GetDC (window->hWnd);									// Grab A Device Context For This Window
	if (window->hDC == 0)												// Did We Get A Device Context?
	{
		// Failed
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	PixelFormat = ChoosePixelFormat (window->hDC, &pfd);				// Find A Compatible Pixel Format
	if (PixelFormat == 0)												// Did We Find A Compatible Format?
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	if (SetPixelFormat (window->hDC, PixelFormat, &pfd) == FALSE)		// Try To Set The Pixel Format
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	window->hRC = wglCreateContext (window->hDC);						// Try To Get A Rendering Context
	if (window->hRC == 0)												// Did We Get A Rendering Context?
	{
		// Failed
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	// Make The Rendering Context Our Current Rendering Context
	if (wglMakeCurrent (window->hDC, window->hRC) == FALSE)
	{
		// Failed
		wglDeleteContext (window->hRC);									// Delete The Rendering Context
		window->hRC = 0;												// Zero The Rendering Context
		ReleaseDC (window->hWnd, window->hDC);							// Release Our Device Context
		window->hDC = 0;												// Zero The Device Context
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
		return FALSE;													// Return False
	}

	ShowWindow (window->hWnd, SW_NORMAL);								// Make The Window Visible
	window->isVisible = TRUE;											// Set isVisible To True

	ReshapeGL (window->init.width, window->init.height);				// Reshape Our GL Window

	ZeroMemory (window->keys, sizeof (Keys));							// Clear All Keys

	window->lastTickCount = GetTickCount ();							// Get Tick Count

	return TRUE;														// Window Creating Was A Success
																		// Initialization Will Be Done In WM_CREATE
}

BOOL DestroyWindowGL (GL_Window* window)								// Destroy The OpenGL Window & Release Resources
{
	if (window->hWnd != 0)												// Does The Window Have A Handle?
	{
		if (window->hDC != 0)											// Does The Window Have A Device Context?
		{
			wglMakeCurrent (window->hDC, 0);							// Set The Current Active Rendering Context To Zero
			if (window->hRC != 0)										// Does The Window Have A Rendering Context?
			{
				wglDeleteContext (window->hRC);							// Release The Rendering Context
				window->hRC = 0;										// Zero The Rendering Context

			}
			ReleaseDC (window->hWnd, window->hDC);						// Release The Device Context
			window->hDC = 0;											// Zero The Device Context
		}
		DestroyWindow (window->hWnd);									// Destroy The Window
		window->hWnd = 0;												// Zero The Window Handle
	}

	if (window->init.isFullScreen)										// Is Window In Fullscreen Mode
	{
		ChangeDisplaySettings (NULL,0);									// Switch Back To Desktop Resolution
		ShowCursor (TRUE);												// Show The Cursor
	}
	return TRUE;														// Return True
}

// Process Window Message Callbacks
LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Get The Window Context
#ifdef __DMC__                                                          // Digital Mars Compiler don't declare GetWindowLongPtr
	GL_Window* window = (GL_Window*)(GetWindowLong (hWnd, GWLP_USERDATA));
#else
	GL_Window* window = (GL_Window*)(GetWindowLongPtr (hWnd, GWLP_USERDATA));
#endif

	switch (uMsg)														// Evaluate Window Message
	{
		case WM_SYSCOMMAND:												// Intercept System Commands
		{
			switch (wParam)												// Check System Calls
			{
				case SC_SCREENSAVE:										// Screensaver Trying To Start?
				case SC_MONITORPOWER:									// Monitor Trying To Enter Powersave?
				return 0;												// Prevent From Happening
			}
			break;														// Exit
		}


		case WM_CREATE:													// Window Creation
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			// Store Window Structure Pointer
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLong (hWnd, GWLP_USERDATA, (LONG)(window));
		}
		return 0;														// Return

		case WM_CLOSE:													// Closing The Window
			TerminateApplication(window);								// Terminate The Application
		return 0;														// Return

		case WM_SIZE:													// Size Action Has Taken Place
			switch (wParam)												// Evaluate Size Action
			{
				case SIZE_MINIMIZED:									// Was Window Minimized?
					window->isVisible = FALSE;							// Set isVisible To False
				return 0;												// Return

				case SIZE_MAXIMIZED:									// Was Window Maximized?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return

				case SIZE_RESTORED:										// Was Window Restored?
					window->isVisible = TRUE;							// Set isVisible To True
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		// Reshape Window - LoWord=Width, HiWord=Height
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYDOWN:												// Update Keyboard Buffers For Keys Pressed
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = TRUE;					// Set The Selected Key (wParam) To True
				return 0;												// Return
			}
		break;															// Break

		case WM_KEYUP:													// Update Keyboard Buffers For Keys Released
			if ((wParam >= 0) && (wParam <= 255))						// Is Key (wParam) In A Valid Range?
			{
				window->keys->keyDown [wParam] = FALSE;					// Set The Selected Key (wParam) To False
				return 0;												// Return
			}
		break;															// Break

		case WM_TOGGLEFULLSCREEN:										// Toggle FullScreen Mode On/Off
			g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
		break;															// Break
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);					// Pass Unhandled Messages To DefWindowProc
}

BOOL RegisterWindowClass (Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEX windowClass;												// Window Class
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof (WNDCLASSEX);					// Size Of The windowClass Structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				// WindowProc Handles Messages
	windowClass.hInstance		= application->hInstance;				// Set The Instance
 	windowClass.hbrBackground	= GetStockObject(BLACK_BRUSH);			// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= application->className;				// Sets The Applications Classname
	if (RegisterClassEx (&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}

// Program Entry (WinMain)
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure
	DWORD				tickCount;										// Used For The Tick Counter

	// Fill Out Application Data
	application.className = "OpenGL";									// Application Class Name
	application.hInstance = hInstance;									// Application Instance

	// Fill Out Window
	ZeroMemory (&window, sizeof (GL_Window));							// Make Sure Memory Is Zeroed
	window.keys					= &keys;								// Window Key Structure
	window.init.application		= &application;							// Window Application
	window.init.title			= "NeHe's Resource File Tutorial";		// Window Title
	window.init.width			= 640;									// Window Width
	window.init.height			= 480;									// Window Height
	window.init.bitsPerPixel	= 16;									// Bits Per Pixel
	window.init.isFullScreen	= TRUE;									// Fullscreen? (Set To TRUE)

	ZeroMemory (&keys, sizeof (Keys));									// Zero keys Structure

	// Ask The User If They Want To Start In FullScreen Mode?
	if (MessageBox (HWND_DESKTOP, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
		window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	}

	// Register A Class For Our Window To Use
	if (RegisterWindowClass (&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBox (HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default
	while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		if (CreateWindowGL (&window) == TRUE)							// Was Window Creation Successful?
		{
			// At This Point We Should Have A Window That Is Setup To Render OpenGL
			if (InitializeGL(&window, &keys) == FALSE)					// Call User Intialization
			{
				// Failure
				TerminateApplication(&window);							// Close Window, This Will Handle The Shutdown
			}
			else														// Otherwise (Start The Message Pump)
			{	// Initialize was a success
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage (&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						if (window.isVisible == FALSE)					// If Window Is Not Visible
						{
							WaitMessage ();								// Application Is Minimized Wait For A Message
						}
						else											// If Window Is Visible
						{
							// Process Application Loop
							tickCount = GetTickCount ();				// Get The Tick Count
							UpdateGL(tickCount - window.lastTickCount);	// Update The Counter
							window.lastTickCount = tickCount;			// Set Last Count To Current Count
							DrawGL();									// Draw Our Scene

							SwapBuffers (window.hDC);					// Swap Buffers (Double Buffering)
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If (Initialize (...

			// Application Is Finished
			DeinitializeGL();											// User Defined DeInitialization

			DestroyWindowGL (&window);									// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBox (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

	UnregisterClass (application.className, application.hInstance);		// UnRegister Window Class
	return 0;
}																		// End Of WinMain()

