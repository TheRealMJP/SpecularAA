//=================================================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code and content licensed under Microsoft Public License (Ms-PL)
//
//=================================================================================================

#pragma once

#include "PCH.h"

#include "Exceptions.h"

namespace SampleFramework11
{

class Window
{

// Public types
public:

    typedef std::tr1::function<LRESULT (HWND, UINT, WPARAM, LPARAM)> MsgFunction;
	typedef std::tr1::function<INT_PTR (HWND, UINT, WPARAM, LPARAM)> DlgFunction;

// Constructor and destructor
public:

	Window (	HINSTANCE hinstance,
				LPCWSTR name = L"SampleCommon Window",
				DWORD style = WS_CAPTION|WS_OVERLAPPED|WS_SYSMENU,
				DWORD exStyle = WS_EX_APPWINDOW,
				DWORD clientWidth = 1280,
				DWORD clientHeight = 720,
				LPCWSTR iconResource = NULL,
				LPCWSTR smallIconResource = NULL,
				LPCWSTR menuResource = NULL,
				LPCWSTR accelResource = NULL );
	~Window ();



// Public methods
public:

	HWND		GetHwnd			() const;
	HMENU		GetMenu			() const;
	HINSTANCE	GetHinstance	() const;
	void		MessageLoop		();

	BOOL		IsAlive	        () const;
	BOOL		IsMinimized		() const;
	LONG_PTR	GetWindowStyle	() const;
	LONG_PTR	GetExtendedStyle() const;
	void		SetWindowStyle	(DWORD newStyle );
	void		SetExtendedStyle(DWORD newExStyle );
	void		Maximize		();
	void		SetWindowPos	(INT posX, INT posY);
	void		GetWindowPos	(INT& posX, INT& posY) const;
	void		ShowWindow		(bool show = true);
	void		SetClientArea	(INT clientX, INT clientY);
	void		GetClientArea	(INT& clientX, INT& clientY) const;
	void		SetWindowTitle	(LPCWSTR title);
	void		SetScrollRanges	(	INT scrollRangeX,
									INT scrollRangeY,
									INT posX,
									INT posY		 );
	void		Destroy			( );

	INT			CreateMessageBox (LPCWSTR message, LPCWSTR title = NULL, UINT type = MB_OK);

	void		SetUserMessageFunction	(UINT message, MsgFunction msgFunction);
	HWND		CreateDialogBox			(LPCWSTR templateName, DlgFunction userDialogFunction);
	INT_PTR		CreateModalDialogBox	(LPCWSTR templateName, DlgFunction userDialogFunction);

	operator HWND() { return hwnd; }		//conversion operator

// Private methods
private:
	void					MakeWindow			( LPCWSTR iconResource, LPCWSTR smallIconResource, LPCWSTR menuResource );

	LRESULT					MessageHandler		( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static LRESULT WINAPI	WndProc				( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

	void					HandleMouseInput	( UINT msg, WPARAM wParam, LPARAM lParam );

	INT_PTR					DialogFunction		( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
	static INT_PTR CALLBACK	DlgProc				( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

// Private members
private:

	// Window properties
	HWND			hwnd;			// The window handle
	HINSTANCE		hinstance;		// The HINSTANCE of the application
	std::wstring	appName;		// The name of the application
	DWORD			style;			// The current window style
	DWORD			exStyle;		// The extended window style
	HACCEL			accelTable;		// Accelerator table handle

	std::map<UINT, MsgFunction>	userMessages;			// User message map
	std::map<HWND, DlgFunction> userDlgFunctions;
	DlgFunction modalDlgFunction;
};

}