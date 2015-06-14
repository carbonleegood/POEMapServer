// Test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
struct Point
{
	int x;
	int y;
	Point(int x, int y)
	{
		this->x = x;
		this->y = y;
	};
	Point()
	{
		x = 0;
		y = 0;
	};
	bool operator ==(Point p)
	{
		if (this->x == p.x&&this->y == p.y)
		{
			return true;
		}
		return false;
	}
	bool operator !=(Point p)
	{
		if (this->x == p.x&&this->y == p.y)
		{
			return false;
		}
		return true;
	}
	bool operator <(Point p) const
	{
		if (this->x == p.x)
		{
			return this->y<p.y;
		}
		else
			return this->x<p.x;
	};
	Point& operator =(const Point& p)
	{
		this->x = p.x;
		this->y = p.y;
		return *this;
	}
};

#define _UpdateMap CrakerDajiji
extern "C"__declspec(dllimport) int WINAPI _UpdateMap(bool GroupModel);
//extern "C" __declspec(dllimport) void WINAPI UpdateMap(bool bGroup);
extern "C" __declspec(dllimport) int WINAPI GetExplorePoint(unsigned short& x, unsigned short& y);
extern "C" __declspec(dllimport) void WINAPI SetExploredPoint(unsigned short x, unsigned short y);
//extern "C"  __declspec(dllimport) void WINAPI DrawMap();
extern "C"  __declspec(dllimport) void WINAPI GetMapData();
extern "C" __declspec(dllimport) int WINAPI FindPath(unsigned short x, unsigned short y,unsigned short tx, unsigned short ty);

//extern "C" __declspec(dllimport) DWORD WINAPI  GetGGPid();
extern "C" __declspec(dllimport) DWORD WINAPI  GetGGPid(char* UID);

#define GetGroupExplorePoint CrakerMujj
extern "C"__declspec(dllimport) int WINAPI CrakerMujj(unsigned short& x, unsigned short& y, int nGroup);
#include <iostream>
using namespace std;


#define _MoveToPoint CutCrakerDajiji
extern "C" __declspec(dllimport) int WINAPI _MoveToPoint(unsigned short x, unsigned short y, unsigned short px, unsigned short py);

#define _SetPassAbleArea jjjjjjj
extern "C" __declspec(dllimport) int WINAPI _SetPassAbleArea(unsigned short x, unsigned short y);

int _tmain(int argc, _TCHAR* argv[])
{
	//cout << "error" << endl;
	//return 0;
//	printf("aaaa\n");
	HMODULE hDll=::LoadLibrary(_T("MapServer.dll"));
	//::OutputDebugStringW(L"ggggg44444");
//	GetMapData();
//	UpdateMap();
//	FindPath(775,305,600,760);
//	DrawMap();
	/*char buff[32] = { 0 };
	DWORD dwPid = GetGGPid(buff);
	std::cout << dwPid << endl;*/

	_UpdateMap(false);
	_SetPassAbleArea(2253, 612);
	WORD x = 2720;
	WORD y = 219;
	_MoveToPoint(2253,612, 2262, 602);
	//GetGroupExplorePoint(x, y, 2);
	return 0;
}

