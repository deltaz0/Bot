#pragma once

#define MAX_IDS 21
#define PRIMARY 0
#define EDITX 1
#define EDITY 2
#define EDITZ 3
#define BUTTON_OK 4
#define BUTTON_CANCEL 5
#define BUTTON_XYPLANEZPOS 6
#define BUTTON_XYPLANEZNEG 7
#define BUTTON_XZPLANEYPOS 8
#define BUTTON_XZPLANEYNEG 9
#define BUTTON_YZPLANEXPOS 10
#define BUTTON_YZPLANEXNEG 11
#define TRANSX 12
#define TRANSY 13
#define TRANSZ 14
#define HORI 15
#define VERT 16
#define MID 17
#define SCALEX 18
#define SCALEY 19
#define SCALEZ 20

#include <windows.h>

class IDMan
{
	public:
		IDMan();	//Constructor
		bool getID(int name, HWND &wind);	//Takes a window name and returns the ID.
		bool removeID(int name); //Takes a window name and removes it from the list.
		void createWin(LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle,
			int x, int y, int nWidth, int nHeight,
			HWND &hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	private:
		HWND editX;
		HWND editY;
		HWND editZ;
		HWND accept;
		HWND cancel;
		HWND xyPlaneZPos;
		HWND xyPlaneZNeg;
		HWND xzPlaneYPos;
		HWND xzPlaneYNeg;
		HWND yzPlaneXPos;
		HWND yzPlaneXNeg;
		HWND transX;
		HWND transY;
		HWND transZ;
		HWND hori;
		HWND vert;
		HWND mid;
		HWND scaleX;
		HWND scaleY;
		HWND scaleZ;
		int curIdCount;
		HWND IDList[MAX_IDS];
};