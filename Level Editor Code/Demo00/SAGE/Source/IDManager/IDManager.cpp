#include "IDManager.h"

//#define EDITX 1
//#define EDITY 2
//#define EDITZ 3
//#define BUTTON_OK 4
//#define BUTTON_CANCEL 5

IDMan::IDMan()
{
	for(int a = 0; a < MAX_IDS; a++)
	{
		IDList[a] = NULL;
	}
}

bool IDMan::getID(int name, HWND &wind)
{
	if(IDList[name] != NULL)
	{
		wind = IDList[name];
		return true;
	}
	else
		return false;
}

bool IDMan::removeID(int name)
{
	if(name > 0 && name < MAX_IDS)
	{
		//item is within bounds of list.
		IDList[name] = NULL;
		return true;
	}
	else
		return false;	//object out of bounds of list, return false.
}

void IDMan::createWin(LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle,
			int x, int y, int nWidth, int nHeight,
			HWND &hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	HWND *mod;
	switch((int)hMenu)
	{
		case EDITX:
			mod = &editX;
			break;
		case EDITY:
			mod = &editY;
			break;
		case EDITZ:
			mod = &editZ;
			break;
		case BUTTON_OK:
			mod = &accept;
			break;
		case BUTTON_CANCEL:
			mod = &cancel;
			break;
		case BUTTON_XYPLANEZPOS:
			mod = &xyPlaneZPos;
			break;
		case BUTTON_XYPLANEZNEG:
			mod = &xyPlaneZNeg;
			break;
		case BUTTON_XZPLANEYPOS:
			mod = &xzPlaneYPos;
			break;
		case BUTTON_XZPLANEYNEG:
			mod = &xzPlaneYNeg;
			break;
		case BUTTON_YZPLANEXPOS:
			mod = &xzPlaneYPos;
			break;
		case BUTTON_YZPLANEXNEG:
			mod = &xzPlaneYNeg;
			break;
		case TRANSX:
			mod = &transX;
			break;
		case TRANSY:
			mod = &transY;
			break;
		case TRANSZ:
			mod = &transZ;
			break;
		case HORI:
			mod = &hori;
			break;
		case VERT:
			mod = &vert;
			break;
		case MID:
			mod = &mid;
			break;
		case SCALEX:
			mod = &scaleX;
			break;
		case SCALEY:
			mod = &scaleY;
			break;
		case SCALEZ:
			mod = &scaleZ;
			break;
		default:
			break;
	}
	/*HWND temp = CreateWindow(TEXT("button"), TEXT("BACK"),
			WS_VISIBLE | WS_CHILD,
			80,40,50,20,
			parent,(HMENU) BUTTON_CANCEL, NULL, NULL);*/
			*mod = CreateWindow(lpClassName, lpWindowName,
			dwStyle,
			x,y,nWidth,nHeight,
			hWndParent,hMenu, hInstance, lpParam);
	IDList[(int)hMenu] = *mod;
}