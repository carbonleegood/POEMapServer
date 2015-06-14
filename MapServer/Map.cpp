#include "StdAfx.h"
#include "Map.h"
#include <vector>
#include <atlimage.h>
#include <fstream>
//#include <Winnt.h>
#ifdef ENCRYPT_VMP
#include "VMProtectSDK.h"
#endif

using namespace std;
#define SUCCESS 0
#define ChangeStep 15
Map GameMap;
SimpleMap PollutantMap;
extern TCHAR szModulePath[MAX_PATH];
int Map::Connect()
{
//	string host = "localhost";
//	int port = 9998;
	//socket.reset(new TSocket(host, port));

	socket.reset(new TPipe("xsxsxs"));
	transport.reset(new TBufferedTransport(socket));
	protocol.reset(new TBinaryProtocol(transport));
	pClient = new GameFuncCallClient(protocol);
	try
	{
		transport->open();
		bConnect = true;
	}
	catch (TTransportException& ttx)
	{
		bConnect = false;
	}
	return 0;
}
Map::Map()
{
	CrackCount = 0;
	

	openList = nullptr; //1 dimensional array holding ID# of open list items
	openX = nullptr; //1d array stores the x location of an item on the open list
	openY = nullptr; //1d array stores the y location of an item on the open list
	Fcost = nullptr;	//1d array to store F cost of a cell on the open list
	Hcost = nullptr;
	whichList = nullptr;//[mapWidth+1][mapHeight+1];  
	parentX = nullptr;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
	parentY = nullptr;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
	Gcost = nullptr;//[mapWidth+1][mapHeight+1]; 	//2d array to store G cost for each cell.
	memset(pathLength, 0, (numberPeople + 1)*sizeof(int));     //stores length of the found path for critter
	memset(pathLocation, 0, (numberPeople + 1)*sizeof(int));   //stores current position along the chosen path for critter		
	memset(pathBank, 0, (numberPeople + 1)*sizeof(int*));
//	bInitMap = false;

	XLine = 0;
	YRow = 0;
	HalfX = 0;

	walkability = nullptr;
}
void Map::DrawMap()
{
	CImage SavePic;
	SavePic.Create(YRow, XLine, 24);
	for (int y = 0; y < YRow; ++y)
	{
		for (int x = 0; x < XLine; ++x)
		{
			if (walkability[y][x] == unwalkable)//不可通行
			{
				SavePic.SetPixel(y, x, RGB(0, 254, 0));
			}
			else
			{
				SavePic.SetPixel(y, x, RGB(0, 0, 254));
			}
		}
	}
	//for (auto Item : ShadowPointList)
	//{
	//	SavePic.SetPixel(Item.y, Item.x, RGB(254, 0, 0));
	//}
	//fstream fs;
	//fs.open("d:\\path.txt", ios::binary | ios::in);
	//short x = 0;
	//short y = 0;
	//fs.read((char*)&x, 2);
	//fs.read((char*)&y, 2);
	//SavePic.SetPixel(y, x, RGB(254, 254, 254));
	//short bx = x;
	//short by = y;
	//while (fs.read((char*)&x, 2))
	//{
	//	fs.read((char*)&y, 2);
	//	SavePic.SetPixel(y, x, RGB(254, 254,254));
	//	//cout << "x:" << x << ",y:" << y << endl;
	//}
	//fs.close();
	////画路径
	//if (FindPath(by, bx, y, x) == found)
	//{
	//	vector<Point> path;
	//	GetBestPath(path);
	//	for (auto item : path)
	//	{
	//		SavePic.SetPixel(item.y, item.x, RGB(0, 0, 0));
	//	}
	//}
	SavePic.Save(L"d:\\saveMapggg.bmp");
	//SavePic.Save(L"d:\\saveMap.jpg", Gdiplus::ImageFormatJPEG);
}
float CalcDis(Point& p1, Point& p2);
void  Map::GroupShadowPoint()
{
//	DWORD dwBegin = ::GetTickCount();
	dbTotalShadowSize.clear();
	ShadowPointGroup.clear();
	//遍历所有POINT,取第一个
	int i = 0;
	std::set<Point> temp;
	while (true)
	{
		ShadowPointGroup.push_back(temp);
		dbTotalShadowSize.push_back(0.0);
		auto iterHead=ShadowPointList.begin();

		ShadowPointGroup[i].insert(*iterHead);
		int cx = iterHead->x;
		int cy = iterHead->y;
		Point cp;
		cp.x = iterHead->x;
		cp.y = iterHead->y;
		ShadowPointList.erase(iterHead);

		int nCurGroupNum = i;
		for (auto Iter = ShadowPointList.begin(); Iter != ShadowPointList.end();)
		{
			Point tp=*Iter;
			float dis=CalcDis(tp, cp);
			if (dis > 850.0)
			{
				++Iter;
				continue;
			}
			//如果A*可达,则分组
		//	if (found == FindPath(Iter->y,Iter->x,cy,cx))
		//	{
				//另一组插入
				ShadowPointGroup[i].insert(*Iter);
				//本组删除
				Iter = ShadowPointList.erase(Iter);
				continue;
		//	}
		//	++Iter;
		}
		if (ShadowPointList.empty())
			break;
		dbTotalShadowSize[i] = ShadowPointGroup[i].size();
		++i;
	}
	//分好组之后
	/*DWORD dwEnd = ::GetTickCount();
	DWORD Diff = dwEnd - dwBegin;
	CString time;
	time.Format(L"%d", Diff);
	MessageBox(NULL, time, NULL, MB_OK);*/
}
int Map::GetGroupCount()
{
	return ShadowPointGroup.size();
}
int Map::GetGroup(int x, int y)
{
	int nGroup = -1;
	for (int i = 0; i < ShadowPointGroup.size(); ++i)
	{
		if (ShadowPointGroup[i].empty())
			continue;
		auto Iter = ShadowPointGroup[i].begin();
		if (found == FindPath(Iter->y, Iter->x, y, x))
		{
			nGroup = i;
			break;
		}
	}
	return nGroup;
}
__inline void Map::AdjustPoint(Point& item)
{
	
		int x = item.x;
		int y = item.y;

		int X = 0;
		int tx = 0;
		int ty = 0;
		if (walkability[y][x - 10])
			X = 2;
		if (walkability[y][x + 10])//X调整
			++X;
		switch (X)
		{
		case 1:
			tx = x + 5;
			break;
		case 2:
			tx = x - 5;
			break;
		case 0://不动
		case 3://不动
			tx = x;
			break;
		}

		//Y调整
		int Y = 0;
		if (walkability[y - 10][x])
			Y = 2;
		if (walkability[y + 10][x])//X调整
			++Y;
		switch (Y)
		{
		case 1:
			ty = y + 5;
			break;
		case 2:
			ty = y - 5;
			break;
		case 0://不动
		case 3://不动
			ty = y;
			break;
		}

		if (walkability[ty][tx])//调整
		{
			item.x = tx;
			item.y = ty;
		}

	//左右调整

	//上下调整

	//对角调整
}
int Map::GetNextGroupExplorePoint(Point& CurPos, int nGroup)
{
	float NearstDis = 10000000.0;
	if (ShadowPointGroup[nGroup].size() < 2)
		return -1;
	//本组找不到点了

	//本组距离近的,直接删除
	auto IterNearstPoint = ShadowPointGroup[nGroup].end();
	//
	for (auto Iter = ShadowPointGroup[nGroup].begin(); Iter != ShadowPointGroup[nGroup].end();)
	{
		int X = abs(Iter->x - CurPos.x);
		int Y = abs(Iter->y - CurPos.y);
		float dis = sqrt(X*X + Y*Y);
		if (dis<20.0)
		{
			//距离过小的,直接删除
			//::Beep(2000, 1000);
			Iter = ShadowPointGroup[nGroup].erase(Iter);
			if (ShadowPointGroup[nGroup].size() < 2)
				return -1;
			else
				continue;
		}
		if (NearstDis > dis)
		{
			IterNearstPoint = Iter;
			NearstDis = dis;
		}
		++Iter;
	}

	//10%不探索优化
	if (NearstDis > 200)
	{
		double size = ShadowPointGroup[nGroup].size();
		double fff = size / dbTotalShadowSize[nGroup];
		if (fff < 0.1)
		{
			/*	CString log;
			log.Format(L"%f,%f,%f", size, dbTotalShadowSize, fff);
			MessageBox(NULL,log.GetBuffer(),NULL,MB_OK);*/
			return -1;
		}
	}
	if (IterNearstPoint != ShadowPointGroup[nGroup].end())
	{
		//如果距离太远,则用A*寻路
		if (NearstDis > 50)
		{
			if (found != FindPath(CurPos.y, CurPos.x, IterNearstPoint->y, IterNearstPoint->x))
			{
				/*	CString log;
				log.Format(L"player:%d,%d,target:%d,%d", CurPos.x, CurPos.y,  IterNearstPoint->x ,IterNearstPoint->y);
				MessageBox(NULL, log, NULL, MB_OK);*/
				ShadowPointGroup[nGroup].erase(IterNearstPoint);
				return -2;//A*不可达
			}
			vector<Point> path;
			GetBestPath(path);
			for (auto item : path)
			{
				ShadowPointGroup[nGroup].insert(item);
			}
			CurPos.x = path[0].x;
			CurPos.y = path[0].y;
		}
		else
		{
			CurPos.x = IterNearstPoint->x;
			CurPos.y = IterNearstPoint->y;
		}
		return 0;
	}
	/*if (ShadowPointList.empty())
	MessageBox(NULL, L"null", NULL, MB_OK);*/
	return -1;
}
#define  Shadow_Step 80
void Map::GenShadowPoint()
{
	ShadowPointList.clear();
	vector<Point> PointList;
	Point point;
	for (int y = Shadow_Step; y < (YRow - ChangeStep - 1); y += Shadow_Step)
	{
		for (int x = Shadow_Step; x <(XLine - ChangeStep - 1); x += Shadow_Step)
		{
			if ((x% Shadow_Step != 0) || (y % Shadow_Step != 0))
				continue;
			if (walkability[y][x] == 0)//不可通行
			{

				if (walkability[y - ChangeStep][x])
				{
					point.x = x;
					point.y = y - ChangeStep;
					PointList.push_back(point);
				}
				else if (walkability[y + ChangeStep][x])
				{
					point.x = x;
					point.y = y + ChangeStep;
					PointList.push_back(point);
				}
				else if (walkability[y][x - ChangeStep])
				{
					point.x = x - ChangeStep;
					point.y = y;
					PointList.push_back(point);
				}
				else if (walkability[y][x + ChangeStep])
				{
					point.x = x + ChangeStep;
					point.y = y;
					PointList.push_back(point);
				}
			}
			else//可通行
			{
				point.x = x;
				point.y = y;
				PointList.push_back(point);
			}
		}
	}
	//调整
	Point newPoint;
	for (auto& item : PointList)
	{
		int x = item.x;
		int y = item.y;

		int X = 0;
		int tx = 0;
		int ty = 0;
		if (walkability[y][x - 10])
			X = 2;
		if (walkability[y][x + 10])//X调整
			++X;
		switch (X)
		{
		case 1:
			tx = x + 5;
			break;
		case 2:
			tx = x - 5;
			break;
		case 0://不动
		case 3://不动
			tx = x;
			break;
		}

		//Y调整
		int Y = 0;
		if (walkability[y - 10][x])
			Y = 2;
		if (walkability[y + 10][x])//X调整
			++Y;
		switch (Y)
		{
		case 1:
			ty = y + 5;
			break;
		case 2:
			ty = y - 5;
			break;
		case 0://不动
		case 3://不动
			ty = y;
			break;
		}
		
		if (walkability[ty][tx])//调整
		{
			newPoint.x = tx;
			newPoint.y = ty;
			ShadowPointList.insert(newPoint);
		}
		else
			ShadowPointList.insert(item);
	}
	if (bGroupModel)
		GroupShadowPoint();
	else
		dbSingleTotalShadowSize = ShadowPointList.size();
}
float CalcDis(Point& p1, Point& p2)
{
	int X = abs(p1.x - p2.x);
	int Y = abs(p1.y - p2.y);
	float dis = sqrt(X*X + Y*Y);
	return dis;
}
//#include <IOCPCheckClient.h>

int Map::GetNextExplorePoint(Point& CurPos)
{
	float NearstDis = 100000000.0;
	auto IterNearstPoint = ShadowPointList.end();
	for (auto Iter = ShadowPointList.begin(); Iter != ShadowPointList.end();)
	{
		int X = abs(Iter->x - CurPos.x);
		int Y = abs(Iter->y - CurPos.y);
		float dis = sqrt(X*X + Y*Y);
		if (dis<20.0)
		{
			//距离过小的,直接删除
			//::Beep(2000, 1000);
			Iter=ShadowPointList.erase(Iter);
			continue;
		}
		if (NearstDis > dis)
		{
			IterNearstPoint = Iter;
			NearstDis = dis;
		}
		++Iter;
	}
	//反破解系统
#ifdef ENCRYPT_VMP
	VMProtectBegin("GetExplorePoint");
#endif
	if (CrackCount > 8000)
	{

		double size = ShadowPointList.size();
		double fff = size / dbSingleTotalShadowSize;
		if (fff < 0.4)
		{
			GenShadowPoint();
			::InterlockedExchangeAdd(&CrackCount, -500);
		}
	}
#ifdef ENCRYPT_VMP
	VMProtectEnd();
#endif
	//10%不探索优化
	if (NearstDis > 200)
	{
		double size = ShadowPointList.size();
		double fff = size / dbSingleTotalShadowSize;
		if (fff < 0.1)
		{
		/*	CString log;
			log.Format(L"%f,%f,%f", size, dbTotalShadowSize, fff);
			MessageBox(NULL,log.GetBuffer(),NULL,MB_OK);*/
			return 1;
		}
	}
	if (IterNearstPoint != ShadowPointList.end())
	{
		//如果距离太远,则用A*寻路
		if (NearstDis > 50)
		{
			//CurPos.x = IterNearstPoint->x;
			//CurPos.y = IterNearstPoint->y;
			if (found != FindPath(CurPos.y, CurPos.x, IterNearstPoint->y, IterNearstPoint->x))
			{
			/*	CString log;
				log.Format(L"player:%d,%d,target:%d,%d", CurPos.x, CurPos.y,  IterNearstPoint->x ,IterNearstPoint->y);
				MessageBox(NULL, log, NULL, MB_OK);*/
				ShadowPointList.erase(IterNearstPoint);
				return 2;
			}
			vector<Point> path;
			GetBestPath(path);
			for (auto item : path)
			{
				AdjustPoint(item);
				ShadowPointList.insert(item);
			}
			CurPos.x = path[0].x;
			CurPos.y = path[0].y;
		}
		else
		{
			CurPos.x = IterNearstPoint->x;
			CurPos.y = IterNearstPoint->y;
		}
		return 0;
	}
	/*if (ShadowPointList.empty())
		MessageBox(NULL, L"null", NULL, MB_OK);*/
	return 1;
}
int Map::GetAstarDis(Point& CurPos, Point& TargetPos)
{
	if (found == FindPath(CurPos.y, CurPos.x, TargetPos.y, TargetPos.x))
	{
		return pathLength[0];
	}
	return -1;
}
int Map::MoveToPoint(unsigned short x, unsigned y,unsigned short px,unsigned short py)
{
//	::OutputDebugStringW(L"ggggg000");
//	bool bRet=AreThereObstacle(y,x,py,px);
//	::OutputDebugStringW(L"ggggg11111");
//	if (!bRet)//直线
//	{
	//	MessageBox(NULL, L"直线", NULL, MB_OK);
//		pClient->Move(x, y);
//		return SUCCESS;
//	}
//	::OutputDebugStringW(L"ggggg22222");
	int dx = 0;
	int dy = 0;
	if (found != FindPath(py, px, y, x))
	{
		//设置不可达点
		Point p;
		p.x = x;
		p.y = y;
	//	::OutputDebugStringW(L"ggggg111111");
		SetExploredPoint(p);
	//	::OutputDebugStringW(L"ggggg33333");
		return 1;
	}
//	::OutputDebugStringW(L"ggggg44444");
	GetDirectPoint(dx,dy);//获取A*第一个转向点
	Point temp;
	temp.x = dx;
	temp.y = dy;
	AdjustPoint(temp);
//	::OutputDebugStringW(L"ggggg55555");
	pClient->Move(temp.x, temp.y);
//	::OutputDebugStringW(L"ggggg66666");
	return SUCCESS;
};
__inline int GetDirect(Point& p1, Point& p2)
{
	//已P1点为中心
	int x = p1.x - p2.x;
	int y = p1.y - p2.y;
	int nDirect = x + y;
	switch (nDirect)
	{
	case -1:
		if (x == -1)
			nDirect = -3;
		break;
	case 0:
		if (x == -1)
			nDirect = 10;
		break;
	case 1:
		if (x == 0)
			nDirect = 3;
		break;
	}
	return nDirect;
}
//int Map::GetDirectPoint(int& x, int& y)
//{
//	return SUCCESS;
//}
#define STEP_NUM 30
//////////////////////原A*部分函数
int Map::GetBestPath(std::vector<Point>& Path)
{
	int x = 0, y = 0;
	int ID = 0;
	int nCurDirect = 100;
	Point FatherPoint;
	Point bPoint;
	if (pathStatus[ID] == found)
	{
		if (pathLength[ID] <= STEP_NUM)
		{
			Point CurPoint;
			CurPoint.y = pathBank[ID][(pathLength[ID]-1) * 2];
			CurPoint.x = pathBank[ID][(pathLength[ID] - 1) * 2 + 1];
			Path.push_back(CurPoint);
		}
		else
		{

			for (int i = STEP_NUM; i < (pathLength[ID]); i += STEP_NUM)
			{
				Point CurPoint;
				CurPoint.y = pathBank[ID][i * 2];
				CurPoint.x = pathBank[ID][i * 2 + 1];

				Path.push_back(CurPoint);

			}
		}
	}
	return Path.size();
}
//int Map::GetDirectPoint(int& retx, int& rety)
//{
//	int x = 0, y = 0;
//	int ID = 0;
//	int nCurDirect = 100;
//	Point FatherPoint;
//	Point bPoint;
//	if (pathStatus[ID] == found)
//	{
//		bPoint.x = pathBank[ID][0];
//		bPoint.y = pathBank[ID][1];
//		FatherPoint.x = pathBank[ID][2];
//		FatherPoint.y = pathBank[ID][3];
//		nCurDirect = GetDirect(FatherPoint, bPoint);
//
//
//		for (int i = 2; i<pathLength[ID] - 1; ++i)
//		{
//			Point CurPoint;
//			CurPoint.x = pathBank[ID][i * 2];
//			CurPoint.y = pathBank[ID][i * 2 + 1];
//			int nDirect = GetDirect(CurPoint, FatherPoint);
//			if (nDirect != nCurDirect)
//			{
//				//如果方向不一致
//				retx = y;
//				rety = x;
//				return SUCCESS;
//			}
//			FatherPoint = CurPoint;
//		}
//	}
//	return 1;
//}
int Map::ChangeNotNearWall(int& x,int& y)//切换点不靠墙
{

	int X = 0;
	int tx = 0;
	int ty = 0;
	if (walkability[y][x - 10])
		X = 2;
	if (walkability[y][x + 10])//X调整
		++X;
	switch (X)
	{
	case 1:
		tx = x + 5;
		break;
	case 2:
		tx = x - 5;
		break;
	case 0://不动
	case 3://不动
		tx = x;
		break;
	}

	//Y调整
	int Y = 0;
	if (walkability[y - 10][x])
		Y = 2;
	if (walkability[y + 10][x])//X调整
		++Y;
	switch (Y)
	{
	case 1:
		ty = y + 5;
		break;
	case 2:
		ty = y - 5;
		break;
	case 0://不动
	case 3://不动
		ty = y;
		break;
	}

	if (walkability[ty][tx])//调整
	{
		x = tx;
		y = ty;
	}
	return 0;
}
int Map::GetDirectPoint(int& retx, int& rety)
{
	int x = 0, y = 0;
	int ID = 0;
	int nCurDirect = 100;
	Point FatherPoint;
	Point bPoint;
	if (pathStatus[ID] == found)
	{
		//bPoint.x = pathBank[ID][0];
		//bPoint.y = pathBank[ID][1];
		//FatherPoint.x = pathBank[ID][2];
		//FatherPoint.y = pathBank[ID][3];
		//nCurDirect = GetDirect(FatherPoint, bPoint);

		if (pathLength[ID] < 16)
		{
			rety = pathBank[ID][(pathLength[ID]-1) * 2];
			retx = pathBank[ID][(pathLength[ID]-1) * 2 + 1];
		}
		else
		{
			rety = pathBank[ID][14 * 2];
			retx = pathBank[ID][14 * 2 + 1];
		}
		/*if (rety < 0)
			_asm int 3;*/
		//边界优化
	//	ChangeNotNearWall(retx, rety);
		return SUCCESS;
		//for (int i = 2; i<pathLength[ID] - 1; ++i)
		//{
		//	Point CurPoint;
		//	CurPoint.x = pathBank[ID][i * 2];
		//	CurPoint.y = pathBank[ID][i * 2 + 1];
		//	int nDirect = GetDirect(CurPoint, FatherPoint);
		//	if (nDirect != nCurDirect)
		//	{
		//		//如果方向不一致
		//		retx = y;
		//		rety = x;
		//		return SUCCESS;
		//	}
		//	FatherPoint = CurPoint;
		//}	
	}
	return 1;
}
void Map::SetPassAbleArea(short x, short y)
{
	for (int i = (y - 5); i < (y + 5); ++i)
	{
		for (int j = (x - 5); j < (x + 5); ++j)
		{
			walkability[i][j] = walkable;
		}
	}
}
void Map::InitializePathfinder(void)
{
	for (int x = 0; x < numberPeople + 1; x++)
		pathBank[x] = (int*)malloc(4);
}

//-----------------------------------------------------------------------------
// Name: EndPathfinder
// Desc: Frees memory used by the pathfinder.
//-----------------------------------------------------------------------------
void Map::EndPathfinder(void)
{
	for (int x = 0; x < numberPeople + 1; x++)
	{
		free(pathBank[x]);
	}
}


//-----------------------------------------------------------------------------
// Name: FindPath
// Desc: Finds a path using A*
//-----------------------------------------------------------------------------
int Map::FindPath(int startingX, int startingY,
	int targetX, int targetY)
{
	int pathfinderID = 0;
	int onOpenList = 0, parentXval = 0, parentYval = 0,
		a = 0, b = 0, m = 0, u = 0, v = 0, temp = 0, corner = 0, numberOfOpenListItems = 0,
		addedGCost = 0, tempGcost = 0, path = 0,
		tempx, pathX, pathY, cellPosition,
		newOpenListItemID = 0;

	//1. Convert location data (in pixels) to coordinates in the walkability array.
	::InterlockedExchangeAdd(&CrackCount, 1);
	int startX = startingX / tileSize;
	int startY = startingY / tileSize;
	targetX = targetX / tileSize;
	targetY = targetY / tileSize;
	//2.Quick Path Checks: Under the some circumstances no path needs to
	//	be generated ...

	//	If starting location and target are in the same location...
	if (startX == targetX && startY == targetY && pathLocation[pathfinderID] > 0)
	{
		pathStatus[0] = found;
		return found;
	}
	if (startX == targetX && startY == targetY && pathLocation[pathfinderID] == 0)
	{
		pathStatus[0] = nonexistent;
		return nonexistent;
	}
	//	If target square is unwalkable, return that it's a nonexistent path.
	if (walkability[targetX][targetY] != walkable)
		goto noPath;

	//3.Reset some variables that need to be cleared
	if (onClosedList > 1000000) //reset whichList occasionally
	{
		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
				whichList[x][y] = 0;
		}
		onClosedList = 10;
	}
	onClosedList = onClosedList + 2; //changing the values of onOpenList and onClosed list is faster than redimming whichList() array
	onOpenList = onClosedList - 1;
	pathLength[pathfinderID] = notStarted;//i.e, = 0
	pathLocation[pathfinderID] = notStarted;//i.e, = 0
	Gcost[startX][startY] = 0; //reset starting square's G value to 0

	//4.Add the starting location to the open list of squares to be checked.
	numberOfOpenListItems = 1;
	openList[1] = 1;//assign it as the top (and currently only) item in the open list, which is maintained as a binary heap (explained below)
	openX[1] = startX; openY[1] = startY;

	//5.Do the following until a path is found or deemed nonexistent.
	do
	{

		//6.If the open list is not empty, take the first cell off of the list.
		//	This is the lowest F cost cell on the open list.
		if (numberOfOpenListItems != 0)
		{

			//7. Pop the first item off the open list.
			parentXval = openX[openList[1]];
			parentYval = openY[openList[1]]; //record cell coordinates of the item
			whichList[parentXval][parentYval] = onClosedList;//add the item to the closed list

			//	Open List = Binary Heap: Delete this item from the open list, which
			//  is maintained as a binary heap. For more information on binary heaps, see:
			//	http://www.policyalmanac.org/games/binaryHeaps.htm
			numberOfOpenListItems = numberOfOpenListItems - 1;//reduce number of open list items by 1	

			//	Delete the top item in binary heap and reorder the heap, with the lowest F cost item rising to the top.
			openList[1] = openList[numberOfOpenListItems + 1];//move the last item in the heap up to slot #1
			v = 1;

			//	Repeat the following until the new item in slot #1 sinks to its proper spot in the heap.
			do
			{
				u = v;
				if (2 * u + 1 <= numberOfOpenListItems) //if both children exist
				{
					//Check if the F cost of the parent is greater than each child.
					//Select the lowest of the two children.
					if (Fcost[openList[u]] >= Fcost[openList[2 * u]])
						v = 2 * u;
					if (Fcost[openList[v]] >= Fcost[openList[2 * u + 1]])
						v = 2 * u + 1;
				}
				else
				{
					if (2 * u <= numberOfOpenListItems) //if only child #1 exists
					{
						//Check if the F cost of the parent is greater than child #1	
						if (Fcost[openList[u]] >= Fcost[openList[2 * u]])
							v = 2 * u;
					}
				}

				if (u != v) //if parent's F is > one of its children, swap them
				{
					temp = openList[u];
					openList[u] = openList[v];
					openList[v] = temp;
				}
				else
					break; //otherwise, exit loop

			} while (1);//reorder the binary heap


			//7.Check the adjacent squares. (Its "children" -- these path children
			//	are similar, conceptually, to the binary heap children mentioned
			//	above, but don't confuse them. They are different. Path children
			//	are portrayed in Demo 1 with grey pointers pointing toward
			//	their parents.) Add these adjacent child squares to the open list
			//	for later consideration if appropriate (see various if statements
			//	below).
			for (b = parentYval - 1; b <= parentYval + 1; b++)
			{
				for (a = parentXval - 1; a <= parentXval + 1; a++)
				{

					//	If not off the map (do this first to avoid array out-of-bounds errors)
					if (a != -1 && b != -1 && a != mapWidth && b != mapHeight)
					{

						//	If not already on the closed list (items on the closed list have
						//	already been considered and can now be ignored).			
						if (whichList[a][b] != onClosedList)
						{

							//	If not a wall/obstacle square.
							if (walkability[a][b] == walkable)
							{

								//	Don't cut across corners
								corner = walkable;
								if (a == parentXval - 1)
								{
									if (b == parentYval - 1)
									{
										if (walkability[parentXval - 1][parentYval] != walkable
											|| walkability[parentXval][parentYval - 1] != walkable)
											corner = unwalkable;
									}
									else if (b == parentYval + 1)
									{
										if (walkability[parentXval][parentYval + 1] != walkable
											|| walkability[parentXval - 1][parentYval] != walkable)
											corner = unwalkable;
									}
								}
								else if (a == parentXval + 1)
								{
									if (b == parentYval - 1)
									{
										if (walkability[parentXval][parentYval - 1] != walkable
											|| walkability[parentXval + 1][parentYval] != walkable)
											corner = unwalkable;
									}
									else if (b == parentYval + 1)
									{
										if (walkability[parentXval + 1][parentYval] != walkable
											|| walkability[parentXval][parentYval + 1] != walkable)
											corner = unwalkable;
									}
								}
								if (corner == walkable)
								{

									//	If not already on the open list, add it to the open list.			
									if (whichList[a][b] != onOpenList)
									{

										//Create a new open list item in the binary heap.
										newOpenListItemID = newOpenListItemID + 1; //each new item has a unique ID #
										m = numberOfOpenListItems + 1;
										openList[m] = newOpenListItemID;//place the new open list item (actually, its ID#) at the bottom of the heap
										openX[newOpenListItemID] = a;
										openY[newOpenListItemID] = b;//record the x and y coordinates of the new item

										//Figure out its G cost
										if (abs(a - parentXval) == 1 && abs(b - parentYval) == 1)
											addedGCost = 14;//cost of going to diagonal squares	
										else
											addedGCost = 10;//cost of going to non-diagonal squares				
										Gcost[a][b] = Gcost[parentXval][parentYval] + addedGCost;

										//Figure out its H and F costs and parent
										Hcost[openList[m]] = 10 * (abs(a - targetX) + abs(b - targetY));
										Fcost[openList[m]] = Gcost[a][b] + Hcost[openList[m]];
										parentX[a][b] = parentXval; parentY[a][b] = parentYval;

										//Move the new open list item to the proper place in the binary heap.
										//Starting at the bottom, successively compare to parent items,
										//swapping as needed until the item finds its place in the heap
										//or bubbles all the way to the top (if it has the lowest F cost).
										while (m != 1) //While item hasn't bubbled to the top (m=1)	
										{
											//Check if child's F cost is < parent's F cost. If so, swap them.	
											if (Fcost[openList[m]] <= Fcost[openList[m / 2]])
											{
												temp = openList[m / 2];
												openList[m / 2] = openList[m];
												openList[m] = temp;
												m = m / 2;
											}
											else
												break;
										}
										numberOfOpenListItems = numberOfOpenListItems + 1;//add one to the number of items in the heap

										//Change whichList to show that the new item is on the open list.
										whichList[a][b] = onOpenList;
									}

									//8.If adjacent cell is already on the open list, check to see if this 
									//	path to that cell from the starting location is a better one. 
									//	If so, change the parent of the cell and its G and F costs.	
									else //If whichList(a,b) = onOpenList
									{

										//Figure out the G cost of this possible new path
										if (abs(a - parentXval) == 1 && abs(b - parentYval) == 1)
											addedGCost = 14;//cost of going to diagonal tiles	
										else
											addedGCost = 10;//cost of going to non-diagonal tiles				
										tempGcost = Gcost[parentXval][parentYval] + addedGCost;

										//If this path is shorter (G cost is lower) then change
										//the parent cell, G cost and F cost. 		
										if (tempGcost < Gcost[a][b]) //if G cost is less,
										{
											parentX[a][b] = parentXval; //change the square's parent
											parentY[a][b] = parentYval;
											Gcost[a][b] = tempGcost;//change the G cost			

											//Because changing the G cost also changes the F cost, if
											//the item is on the open list we need to change the item's
											//recorded F cost and its position on the open list to make
											//sure that we maintain a properly ordered open list.
											for (int x = 1; x <= numberOfOpenListItems; x++) //look for the item in the heap
											{
												if (openX[openList[x]] == a && openY[openList[x]] == b) //item found
												{
													Fcost[openList[x]] = Gcost[a][b] + Hcost[openList[x]];//change the F cost

													//See if changing the F score bubbles the item up from it's current location in the heap
													m = x;
													while (m != 1) //While item hasn't bubbled to the top (m=1)	
													{
														//Check if child is < parent. If so, swap them.	
														if (Fcost[openList[m]] < Fcost[openList[m / 2]])
														{
															temp = openList[m / 2];
															openList[m / 2] = openList[m];
															openList[m] = temp;
															m = m / 2;
														}
														else
															break;
													}
													break; //exit for x = loop
												} //If openX(openList(x)) = a
											} //For x = 1 To numberOfOpenListItems
										}//If tempGcost < Gcost(a,b)
									}//else If whichList(a,b) = onOpenList	
								}//If not cutting a corner
							}//If not a wall/obstacle square.
						}//If not already on the closed list 
					}//If not off the map
				}//for (a = parentXval-1; a <= parentXval+1; a++){
			}//for (b = parentYval-1; b <= parentYval+1; b++){
		}//if (numberOfOpenListItems != 0)

		//9.If open list is empty then there is no path.	
		else
		{
			path = nonexistent;
			break;
		}

		//If target is added to open list then path has been found.
		if (whichList[targetX][targetY] == onOpenList)
		{
			path = found;
			break;
		}

	} while (1);//Do until path is found or deemed nonexistent

	//10.Save the path if it exists.
	if (path == found)
	{

		//a.Working backwards from the target to the starting location by checking
		//	each cell's parent, figure out the length of the path.
		pathX = targetX; pathY = targetY;
		do
		{
			//Look up the parent of the current cell.	
			tempx = parentX[pathX][pathY];
			pathY = parentY[pathX][pathY];
			pathX = tempx;

			//Figure out the path length
			pathLength[pathfinderID] = pathLength[pathfinderID] + 1;
		} while (pathX != startX || pathY != startY);

		//b.Resize the data bank to the right size in bytes
		pathBank[pathfinderID] = (int*)realloc(pathBank[pathfinderID],
			pathLength[pathfinderID] * 8);

		//c. Now copy the path information over to the databank. Since we are
		//	working backwards from the target to the start location, we copy
		//	the information to the data bank in reverse order. The result is
		//	a properly ordered set of path data, from the first step to the
		//	last.
		pathX = targetX; pathY = targetY;
		cellPosition = pathLength[pathfinderID] * 2;//start at the end	
		do
		{
			cellPosition = cellPosition - 2;//work backwards 2 integers
			pathBank[pathfinderID][cellPosition] = pathX;
			pathBank[pathfinderID][cellPosition + 1] = pathY;

			//d.Look up the parent of the current cell.	
			tempx = parentX[pathX][pathY];
			pathY = parentY[pathX][pathY];
			pathX = tempx;

			//e.If we have reached the starting square, exit the loop.	
		} while (pathX != startX || pathY != startY);

		//11.Read the first path step into xPath/yPath arrays
		ReadPath(pathfinderID, startingX, startingY, 1);

	}
	pathStatus[0] = path;
	return path;


	//13.If there is no path to the selected target, set the pathfinder's
	//	xPath and yPath equal to its current location and return that the
	//	path is nonexistent.
noPath:
	xPath[pathfinderID] = startingX;
	yPath[pathfinderID] = startingY;
	pathStatus[0] = nonexistent;
	return nonexistent;
}




//==========================================================
//READ PATH DATA: These functions read the path data and convert
//it to screen pixel coordinates.
void Map::ReadPath(int pathfinderID, int currentX, int currentY,int pixelsPerFrame)
{
	/*
	;	Note on PixelsPerFrame: The need for this parameter probably isn't
	;	that obvious, so a little explanation is in order. This
	;	parameter is used to determine if the pathfinder has gotten close
	;	enough to the center of a given path square to warrant looking up
	;	the next step on the path.
	;
	;	This is needed because the speed of certain sprites can
	;	make reaching the exact center of a path square impossible.
	;	In Demo #2, the chaser has a velocity of 3 pixels per frame. Our
	;	tile size is 50 pixels, so the center of a tile will be at location
	;	25, 75, 125, etc. Some of these are not evenly divisible by 3, so
	;	our pathfinder has to know how close is close enough to the center.
	;	It calculates this by seeing if the pathfinder is less than
	;	pixelsPerFrame # of pixels from the center of the square.

	;	This could conceivably cause problems if you have a *really* fast
	;	sprite and/or really small tiles, in which case you may need to
	;	adjust the formula a bit. But this should almost never be a problem
	;	for games with standard sized tiles and normal speeds. Our smiley
	;	in Demo #4 moves at a pretty fast clip and it isn't even close
	;	to being a problem.
	*/

	int ID = pathfinderID; //redundant, but makes the following easier to read

	//If a path has been found for the pathfinder	...
	if (pathStatus[ID] == found)
	{

		//If path finder is just starting a new path or has reached the 
		//center of the current path square (and the end of the path
		//hasn't been reached), look up the next path square.
		if (pathLocation[ID] < pathLength[ID])
		{
			//if just starting or if close enough to center of square
			if (pathLocation[ID] == 0 ||
				(abs(currentX - xPath[ID]) < pixelsPerFrame && abs(currentY - yPath[ID]) < pixelsPerFrame))
				pathLocation[ID] = pathLocation[ID] + 1;
		}

		//Read the path data.		
		xPath[ID] = ReadPathX(ID, pathLocation[ID]);
		yPath[ID] = ReadPathY(ID, pathLocation[ID]);

		//If the center of the last path square on the path has been 
		//reached then reset.
		if (pathLocation[ID] == pathLength[ID])
		{
			if (abs(currentX - xPath[ID]) < pixelsPerFrame
				&& abs(currentY - yPath[ID]) < pixelsPerFrame) //if close enough to center of square
				pathStatus[ID] = notStarted;
		}
	}

	//If there is no path for this pathfinder, simply stay in the current
	//location.
	else
	{
		xPath[ID] = currentX;
		yPath[ID] = currentY;
	}
}
//-----------------------------------------------------------------------------
// Name: ReadPathX
// Desc: Reads the x coordinate of the next path step
//-----------------------------------------------------------------------------
int Map::ReadPathX(int pathfinderID, int pathLocation)
{
	int x;
	if (pathLocation <= pathLength[pathfinderID])
	{

		//Read coordinate from bank
		x = pathBank[pathfinderID][pathLocation * 2 - 2];

		//Adjust the coordinates so they align with the center
		//of the path square (optional). This assumes that you are using
		//sprites that are centered -- i.e., with the midHandle command.
		//Otherwise you will want to adjust this.
		//x = tileSize*x + .5*tileSize;
		x = x + .5;

	}
	return x;
}


//-----------------------------------------------------------------------------
// Name: ReadPathY
// Desc: Reads the y coordinate of the next path step
//-----------------------------------------------------------------------------
int Map::ReadPathY(int pathfinderID, int pathLocation)
{
	int y;
	if (pathLocation <= pathLength[pathfinderID])
	{

		//Read coordinate from bank
		y = pathBank[pathfinderID][pathLocation * 2 - 1];

		//Adjust the coordinates so they align with the center
		//of the path square (optional). This assumes that you are using
		//sprites that are centered -- i.e., with the midHandle command.
		//Otherwise you will want to adjust this.
		//y = tileSize*y + .5*tileSize;
		y = y + .5;

	}
	return y;
}
//////////////////////////////////////////////////////////////////////////////////////

void Map::SetGroupExploredPoint(Point& TargetPos, int nGroup)
{
	auto iter = ShadowPointGroup[nGroup].find(TargetPos);
	if (iter != ShadowPointGroup[nGroup].end())
		ShadowPointGroup[nGroup].erase(iter);
}
void Map::SetExploredPoint(Point& TargetPos)
{
	auto iter=ShadowPointList.find(TargetPos);
	if (iter != ShadowPointList.end())
		ShadowPointList.erase(iter);
}

int Map::ResetMapInfo(bool bGroupModel)//重新载入地图数据
{
	this->bGroupModel = bGroupModel;
	if (openList != nullptr)
	{
		delete(openList);
		openList = nullptr;
	}
	if (openX != nullptr)
	{
		delete(openX);
		openX = nullptr;
	}
	if (openY != nullptr)
	{
		delete(openY);
		openY = nullptr;
	}
	if (Fcost != nullptr)
	{
		delete(Fcost);
		Fcost = nullptr;
	}
	if (Hcost != nullptr)
	{
		delete(Hcost);
		Hcost = nullptr;
	}
	if (YRow>0)
	{
		for (int i = 0; i<YRow; ++i)
		{
			delete[](walkability[i]);
			delete[](whichList[i]);
			delete[](parentX[i]);
			delete[](parentY[i]);
			delete[](Gcost[i]);
		}
		delete[](whichList[YRow]);
		delete[](parentX[YRow]);
		delete[](parentY[YRow]);
		delete[](Gcost[YRow]);

		delete[](walkability);
		delete[](whichList);
		delete[](parentX);
		delete[](parentY);
		delete[](Gcost);
	}
	fstream fs;
	CString strPath = szModulePath;
	strPath += L"\\nnnn.bat";
	fs.open(strPath, ios::binary | ios::in);
//	fs.open("d:\\map.txt", ios::binary | ios::in);

	fs.read((char*)&XLine, sizeof(short));
	fs.read((char*)&YRow, sizeof(short));

	mapWidth = YRow;
	mapHeight = XLine;
	//申请新缓冲区
	openList = new int[YRow*XLine + 2];
	openX = new int[YRow*XLine + 2]; //1d array stores the x location of an item on the open list
	openY = new int[YRow*XLine + 2]; //1d array stores the y location of an item on the open list
	Fcost = new int[YRow*XLine + 2];	//1d array to store F cost of a cell on the open list
	Hcost = new int[YRow*XLine + 2];

	walkability = new char*[YRow];
	whichList = new int*[YRow + 1];
	parentX = new int*[YRow + 1];
	parentY = new int*[YRow + 1];
	Gcost = new int*[YRow + 1];

	for (int i = 0; i<YRow; i++)
	{
		walkability[i] = new char[XLine];
		fs.read(walkability[i], XLine);

		whichList[i] = new int[XLine + 1];
		parentX[i] = new int[XLine + 1];
		parentY[i] = new int[XLine + 1];
		Gcost[i] = new int[XLine + 1];
	}
	whichList[YRow] = new int[XLine + 1];
	parentX[YRow] = new int[XLine + 1];
	parentY[YRow] = new int[XLine + 1];
	Gcost[YRow] = new int[XLine + 1];

	fs.close();
	

	GenShadowPoint();
	return SUCCESS;
}

bool Map::IsObstaclePoint(int x,int y)
{
	/*__try
	{*/
		if(x>=XLine || y>=YRow || x<=0 || y<=0)
			return false;
		if(walkability[y][x]==0)
			return true;
	//}
	//__except(1){}
	return false;
}
inline void swap_int(int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}
bool Map::AreThereObstacle(int StartX, int StartY, int EndX, int EndY)
{
	CString log;
	log.Format(L"ggggnew:::call:%d,%d,    %d,%d", StartX, StartY, EndX, EndY);
	::OutputDebugStringW(log);
	int dx = abs(EndX - StartX),
		dy = abs(EndY - StartY),
		yy = 0;

	if (dx < dy)
	{
		yy = 1;
		swap_int(&StartX, &StartY);
		swap_int(&EndX, &EndY);
		swap_int(&dx, &dy);
	}
	int ix = (EndX - StartX) > 0 ? 1 : -1,
		iy = (EndY - StartY) > 0 ? 1 : -1,
		cx = StartX,
		cy = StartY,
		n2dy = dy * 2,
		n2dydx = (dy - dx) * 2,
		d = dy * 2 - dx;
	if (yy)
	{
		while (cx != EndX)
		{
			if (d < 0)
			{
				d += n2dy;
			}
			else
			{
				cy += iy;
				d += n2dydx;
			}

			log.Format(L"gggg%d,%d,    %d,%d", cx, cy,YRow,XLine);
			::OutputDebugStringW(log);
			if (walkability[cx][cy] == unwalkable)
			{
				::OutputDebugStringW(L"gggggeeeeee");
				return true;
			}
			::OutputDebugStringW(L"gggggeeeeee");
			cx += ix;
		}
	}
	else
	{
		while (cx != EndX)
		{
			if (d < 0)
			{
				d += n2dy;
			}
			else
			{
				cy += iy;
				d += n2dydx;
			}

			log.Format(L"gggg%d,%d,    %d,%d", cx, cy, YRow, XLine);
			::OutputDebugStringW(log);
			if (walkability[cx][cy] == unwalkable)
			{
				::OutputDebugStringW(L"gggggeeeeee");
				return true;
			}
			::OutputDebugStringW(L"gggggeeeeee");
			cx += ix;
		}
	}
	return false;
}
int DDrawMap(vector<signed char>& p, DWORD x, DWORD y, DWORD halfX);
void Map::GetMapData()
{
	//连接
	/*string host = "localhost";
	int port = 9998;
	boost::shared_ptr<TSocket> socket(new TSocket(host, port));
	boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
	boost::shared_ptr<TBinaryProtocol> protocol(new TBinaryProtocol(transport));
	GameFuncCallClient client(protocol);*/
	/*try
	{
		transport->open();
		bConnect = true;
	}
	catch (TTransportException& ttx)
	{
		bConnect = false;
	}*/
	MapInfo map;
	pClient->GetMapData(map);
	DDrawMap(map.MapData, map.X, map.Y, map.HalfX);
	transport->close();
}
int DDrawMap(vector<signed char>& p, DWORD x, DWORD y, DWORD halfX)
{
	fstream file;
	file.open("d:\\map.txt", ios::out | ios::binary);
	if (!file.is_open())
	{
		//cout << "打开文件失败!" << endl;
		return 0;
	}
	short lineX = x;
	short rowY = y;
	/*char* array = new char[lineX];
	memset(array, 0, lineX);*/

	file.write((char*)&lineX, sizeof(short));
	file.write((char*)&rowY, sizeof(short));
	char* buff = new char[lineX];
	for (int i = 0; i < rowY; ++i)
	{
		memset(buff, 0, lineX);
		for (int j = 0; j < lineX; ++j)
		{
			unsigned char pBase = p[halfX*i + j / 2];
			if (j % 2)
			{
				buff[j] = pBase & 0xE0;
				if (buff[j])
					buff[j] = 1;
			}
			else
			{
				buff[j] = pBase & 0xE;
				if (buff[j])
					buff[j] = 1;
			}
		}
		file.write(buff, lineX);
		//	 char* pp =(char*)( p + lineX*i);
		//	file.write(pp, lineX);
	}
	file.flush();
	file.close();
	return 0;
}


TownMap townmap;
int TownMap::Connect()
{
	//string host = "localhost";
	//int port = 9998;
	//socket.reset(new TSocket(host, port));
	socket.reset(new TPipe("xsxsxs"));
	transport.reset(new TBufferedTransport(socket));
	protocol.reset(new TBinaryProtocol(transport));
	pClient = new GameFuncCallClient(protocol);
	try
	{
		transport->open();
		bConnect = true;
	}
	catch (TTransportException& ttx)
	{
		bConnect = false;
	}
	return 0;
}
TownMap::TownMap()
{
	/*string host = "localhost";
	int port = 9998;
	socket.reset(new TSocket(host, port));
	transport.reset(new TBufferedTransport(socket));
	protocol.reset(new TBinaryProtocol(transport));
	pClient = new GameFuncCallClient(protocol);
	try
	{
	transport->open();
	bConnect = true;
	}
	catch (TTransportException& ttx)
	{
	bConnect = false;
	}*/

	openList = nullptr; //1 dimensional array holding ID# of open list items
	openX = nullptr; //1d array stores the x location of an item on the open list
	openY = nullptr; //1d array stores the y location of an item on the open list
	Fcost = nullptr;	//1d array to store F cost of a cell on the open list
	Hcost = nullptr;
	whichList = nullptr;//[mapWidth+1][mapHeight+1];  
	parentX = nullptr;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
	parentY = nullptr;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
	Gcost = nullptr;//[mapWidth+1][mapHeight+1]; 	//2d array to store G cost for each cell.
	memset(pathLength, 0, (numberPeople + 1)*sizeof(int));     //stores length of the found path for critter
	memset(pathLocation, 0, (numberPeople + 1)*sizeof(int));   //stores current position along the chosen path for critter		
	memset(pathBank, 0, (numberPeople + 1)*sizeof(int*));
	//	bInitMap = false;

	XLine = 0;
	YRow = 0;
	HalfX = 0;

	walkability = nullptr;
}
void TownMap::DrawMap()
{
	CImage SavePic;
	SavePic.Create(YRow, XLine, 24);
	for (int y = 0; y < YRow; ++y)
	{
		for (int x = 0; x < XLine; ++x)
		{
			if (walkability[y][x] == unwalkable)//不可通行
			{
				SavePic.SetPixel(y, x, RGB(0, 254, 0));
			}
			else
			{
				SavePic.SetPixel(y, x, RGB(0, 0, 254));
			}
		}
	}
	SavePic.SetPixel(NPC_Y, NPC_X, RGB(254, 0, 0));
	SavePic.SetPixel(Trans_Y, Trans_X, RGB(254, 0, 0));
	SavePic.SetPixel(Waypoint_Y, Waypoint_X, RGB(254, 0, 0));
	SavePic.Save(L"d:\\saveMapggg.bmp");
}



int TownMap::MoveToPoint(unsigned short x, unsigned y, unsigned short px, unsigned short py)
{
	int dx = 0;
	int dy = 0;
//	::OutputDebugStringW(L"ggggg22222");
	if (found != FindPath(py, px, y, x))
	{
		//设置不可达点
		//	::OutputDebugStringW(L"ggggg33333");
		return 1;
	}
	//::OutputDebugStringW(L"ggggg33333");
	GetDirectPoint(dx, dy);//获取A*第一个转向点
//	::OutputDebugStringW(L"ggggg4444");
	pClient->Move(dx, dy);
//	::OutputDebugStringW(L"ggggg55555");
	return SUCCESS;
};


//////////////////////原A*部分函数
int TownMap::GetBestPath(std::vector<Point>& Path)
{
	int x = 0, y = 0;
	int ID = 0;
	int nCurDirect = 100;
	Point FatherPoint;
	Point bPoint;
	if (pathStatus[ID] == found)
	{
		if (pathLength[ID] <= STEP_NUM)
		{
			Point CurPoint;
			CurPoint.y = pathBank[ID][(pathLength[ID] - 1) * 2];
			CurPoint.x = pathBank[ID][(pathLength[ID] - 1) * 2 + 1];
			Path.push_back(CurPoint);
		}
		else
		{

			for (int i = STEP_NUM; i < (pathLength[ID]); i += STEP_NUM)
			{
				Point CurPoint;
				CurPoint.y = pathBank[ID][i * 2];
				CurPoint.x = pathBank[ID][i * 2 + 1];

				Path.push_back(CurPoint);

			}
		}
	}
	return Path.size();
}

int TownMap::ChangeNotNearWall(int& x, int& y)//切换点不靠墙
{

	int X = 0;
	int tx = 0;
	int ty = 0;
	if (walkability[y][x - 10])
		X = 2;
	if (walkability[y][x + 10])//X调整
		++X;
	switch (X)
	{
	case 1:
		tx = x + 5;
		break;
	case 2:
		tx = x - 5;
		break;
	case 0://不动
	case 3://不动
		tx = x;
		break;
	}

	//Y调整
	int Y = 0;
	if (walkability[y - 10][x])
		Y = 2;
	if (walkability[y + 10][x])//X调整
		++Y;
	switch (Y)
	{
	case 1:
		ty = y + 5;
		break;
	case 2:
		ty = y - 5;
		break;
	case 0://不动
	case 3://不动
		ty = y;
		break;
	}

	if (walkability[ty][tx])//调整
	{
		x = tx;
		y = ty;
	}
	return 0;
}
int TownMap::GetDirectPoint(int& retx, int& rety)
{
	int x = 0, y = 0;
	int ID = 0;
	int nCurDirect = 100;
	Point FatherPoint;
	Point bPoint;
	if (pathStatus[ID] == found)
	{
		//bPoint.x = pathBank[ID][0];
		//bPoint.y = pathBank[ID][1];
		//FatherPoint.x = pathBank[ID][2];
		//FatherPoint.y = pathBank[ID][3];
		//nCurDirect = GetDirect(FatherPoint, bPoint);

		if (pathLength[ID] < 16)
		{
			rety = pathBank[ID][(pathLength[ID] - 1) * 2];
			retx = pathBank[ID][(pathLength[ID] - 1) * 2 + 1];
		}
		else
		{
			rety = pathBank[ID][14 * 2];
			retx = pathBank[ID][14 * 2 + 1];
		}
		//边界优化
	//	ChangeNotNearWall(retx, rety);
		return SUCCESS;
		//for (int i = 2; i<pathLength[ID] - 1; ++i)
		//{
		//	Point CurPoint;
		//	CurPoint.x = pathBank[ID][i * 2];
		//	CurPoint.y = pathBank[ID][i * 2 + 1];
		//	int nDirect = GetDirect(CurPoint, FatherPoint);
		//	if (nDirect != nCurDirect)
		//	{
		//		//如果方向不一致
		//		retx = y;
		//		rety = x;
		//		return SUCCESS;
		//	}
		//	FatherPoint = CurPoint;
		//}	
	}
	return 1;
}
void TownMap::InitializePathfinder(void)
{
	for (int x = 0; x < numberPeople + 1; x++)
		pathBank[x] = (int*)malloc(4);
}

//-----------------------------------------------------------------------------
// Name: EndPathfinder
// Desc: Frees memory used by the pathfinder.
//-----------------------------------------------------------------------------
void TownMap::EndPathfinder(void)
{
	for (int x = 0; x < numberPeople + 1; x++)
	{
		free(pathBank[x]);
	}
}


//-----------------------------------------------------------------------------
// Name: FindPath
// Desc: Finds a path using A*
//-----------------------------------------------------------------------------
int TownMap::FindPath(int startingX, int startingY,
	int targetX, int targetY)
{
	int pathfinderID = 0;
	int onOpenList = 0, parentXval = 0, parentYval = 0,
		a = 0, b = 0, m = 0, u = 0, v = 0, temp = 0, corner = 0, numberOfOpenListItems = 0,
		addedGCost = 0, tempGcost = 0, path = 0,
		tempx, pathX, pathY, cellPosition,
		newOpenListItemID = 0;

	//1. Convert location data (in pixels) to coordinates in the walkability array.

	int startX = startingX / tileSize;
	int startY = startingY / tileSize;
	targetX = targetX / tileSize;
	targetY = targetY / tileSize;
	//2.Quick Path Checks: Under the some circumstances no path needs to
	//	be generated ...

	//	If starting location and target are in the same location...
	if (startX == targetX && startY == targetY && pathLocation[pathfinderID] > 0)
	{
		pathStatus[0] = found;
		return found;
	}
	if (startX == targetX && startY == targetY && pathLocation[pathfinderID] == 0)
	{
		pathStatus[0] = nonexistent;
		return nonexistent;
	}
	//	If target square is unwalkable, return that it's a nonexistent path.
	if (walkability[targetX][targetY] != walkable)
		goto noPath;

	//3.Reset some variables that need to be cleared
	if (onClosedList > 1000000) //reset whichList occasionally
	{
		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
				whichList[x][y] = 0;
		}
		onClosedList = 10;
	}
	onClosedList = onClosedList + 2; //changing the values of onOpenList and onClosed list is faster than redimming whichList() array
	onOpenList = onClosedList - 1;
	pathLength[pathfinderID] = notStarted;//i.e, = 0
	pathLocation[pathfinderID] = notStarted;//i.e, = 0
	Gcost[startX][startY] = 0; //reset starting square's G value to 0

	//4.Add the starting location to the open list of squares to be checked.
	numberOfOpenListItems = 1;
	openList[1] = 1;//assign it as the top (and currently only) item in the open list, which is maintained as a binary heap (explained below)
	openX[1] = startX; openY[1] = startY;

	//5.Do the following until a path is found or deemed nonexistent.
	do
	{

		//6.If the open list is not empty, take the first cell off of the list.
		//	This is the lowest F cost cell on the open list.
		if (numberOfOpenListItems != 0)
		{

			//7. Pop the first item off the open list.
			parentXval = openX[openList[1]];
			parentYval = openY[openList[1]]; //record cell coordinates of the item
			whichList[parentXval][parentYval] = onClosedList;//add the item to the closed list

			//	Open List = Binary Heap: Delete this item from the open list, which
			//  is maintained as a binary heap. For more information on binary heaps, see:
			//	http://www.policyalmanac.org/games/binaryHeaps.htm
			numberOfOpenListItems = numberOfOpenListItems - 1;//reduce number of open list items by 1	

			//	Delete the top item in binary heap and reorder the heap, with the lowest F cost item rising to the top.
			openList[1] = openList[numberOfOpenListItems + 1];//move the last item in the heap up to slot #1
			v = 1;

			//	Repeat the following until the new item in slot #1 sinks to its proper spot in the heap.
			do
			{
				u = v;
				if (2 * u + 1 <= numberOfOpenListItems) //if both children exist
				{
					//Check if the F cost of the parent is greater than each child.
					//Select the lowest of the two children.
					if (Fcost[openList[u]] >= Fcost[openList[2 * u]])
						v = 2 * u;
					if (Fcost[openList[v]] >= Fcost[openList[2 * u + 1]])
						v = 2 * u + 1;
				}
				else
				{
					if (2 * u <= numberOfOpenListItems) //if only child #1 exists
					{
						//Check if the F cost of the parent is greater than child #1	
						if (Fcost[openList[u]] >= Fcost[openList[2 * u]])
							v = 2 * u;
					}
				}

				if (u != v) //if parent's F is > one of its children, swap them
				{
					temp = openList[u];
					openList[u] = openList[v];
					openList[v] = temp;
				}
				else
					break; //otherwise, exit loop

			} while (1);//reorder the binary heap


			//7.Check the adjacent squares. (Its "children" -- these path children
			//	are similar, conceptually, to the binary heap children mentioned
			//	above, but don't confuse them. They are different. Path children
			//	are portrayed in Demo 1 with grey pointers pointing toward
			//	their parents.) Add these adjacent child squares to the open list
			//	for later consideration if appropriate (see various if statements
			//	below).
			for (b = parentYval - 1; b <= parentYval + 1; b++)
			{
				for (a = parentXval - 1; a <= parentXval + 1; a++)
				{

					//	If not off the map (do this first to avoid array out-of-bounds errors)
					if (a != -1 && b != -1 && a != mapWidth && b != mapHeight)
					{

						//	If not already on the closed list (items on the closed list have
						//	already been considered and can now be ignored).			
						if (whichList[a][b] != onClosedList)
						{

							//	If not a wall/obstacle square.
							if (walkability[a][b] == walkable)
							{

								//	Don't cut across corners
								corner = walkable;
								if (a == parentXval - 1)
								{
									if (b == parentYval - 1)
									{
										if (walkability[parentXval - 1][parentYval] != walkable
											|| walkability[parentXval][parentYval - 1] != walkable)
											corner = unwalkable;
									}
									else if (b == parentYval + 1)
									{
										if (walkability[parentXval][parentYval + 1] != walkable
											|| walkability[parentXval - 1][parentYval] != walkable)
											corner = unwalkable;
									}
								}
								else if (a == parentXval + 1)
								{
									if (b == parentYval - 1)
									{
										if (walkability[parentXval][parentYval - 1] != walkable
											|| walkability[parentXval + 1][parentYval] != walkable)
											corner = unwalkable;
									}
									else if (b == parentYval + 1)
									{
										if (walkability[parentXval + 1][parentYval] != walkable
											|| walkability[parentXval][parentYval + 1] != walkable)
											corner = unwalkable;
									}
								}
								if (corner == walkable)
								{

									//	If not already on the open list, add it to the open list.			
									if (whichList[a][b] != onOpenList)
									{

										//Create a new open list item in the binary heap.
										newOpenListItemID = newOpenListItemID + 1; //each new item has a unique ID #
										m = numberOfOpenListItems + 1;
										openList[m] = newOpenListItemID;//place the new open list item (actually, its ID#) at the bottom of the heap
										openX[newOpenListItemID] = a;
										openY[newOpenListItemID] = b;//record the x and y coordinates of the new item

										//Figure out its G cost
										if (abs(a - parentXval) == 1 && abs(b - parentYval) == 1)
											addedGCost = 14;//cost of going to diagonal squares	
										else
											addedGCost = 10;//cost of going to non-diagonal squares				
										Gcost[a][b] = Gcost[parentXval][parentYval] + addedGCost;

										//Figure out its H and F costs and parent
										Hcost[openList[m]] = 10 * (abs(a - targetX) + abs(b - targetY));
										Fcost[openList[m]] = Gcost[a][b] + Hcost[openList[m]];
										parentX[a][b] = parentXval; parentY[a][b] = parentYval;

										//Move the new open list item to the proper place in the binary heap.
										//Starting at the bottom, successively compare to parent items,
										//swapping as needed until the item finds its place in the heap
										//or bubbles all the way to the top (if it has the lowest F cost).
										while (m != 1) //While item hasn't bubbled to the top (m=1)	
										{
											//Check if child's F cost is < parent's F cost. If so, swap them.	
											if (Fcost[openList[m]] <= Fcost[openList[m / 2]])
											{
												temp = openList[m / 2];
												openList[m / 2] = openList[m];
												openList[m] = temp;
												m = m / 2;
											}
											else
												break;
										}
										numberOfOpenListItems = numberOfOpenListItems + 1;//add one to the number of items in the heap

										//Change whichList to show that the new item is on the open list.
										whichList[a][b] = onOpenList;
									}

									//8.If adjacent cell is already on the open list, check to see if this 
									//	path to that cell from the starting location is a better one. 
									//	If so, change the parent of the cell and its G and F costs.	
									else //If whichList(a,b) = onOpenList
									{

										//Figure out the G cost of this possible new path
										if (abs(a - parentXval) == 1 && abs(b - parentYval) == 1)
											addedGCost = 14;//cost of going to diagonal tiles	
										else
											addedGCost = 10;//cost of going to non-diagonal tiles				
										tempGcost = Gcost[parentXval][parentYval] + addedGCost;

										//If this path is shorter (G cost is lower) then change
										//the parent cell, G cost and F cost. 		
										if (tempGcost < Gcost[a][b]) //if G cost is less,
										{
											parentX[a][b] = parentXval; //change the square's parent
											parentY[a][b] = parentYval;
											Gcost[a][b] = tempGcost;//change the G cost			

											//Because changing the G cost also changes the F cost, if
											//the item is on the open list we need to change the item's
											//recorded F cost and its position on the open list to make
											//sure that we maintain a properly ordered open list.
											for (int x = 1; x <= numberOfOpenListItems; x++) //look for the item in the heap
											{
												if (openX[openList[x]] == a && openY[openList[x]] == b) //item found
												{
													Fcost[openList[x]] = Gcost[a][b] + Hcost[openList[x]];//change the F cost

													//See if changing the F score bubbles the item up from it's current location in the heap
													m = x;
													while (m != 1) //While item hasn't bubbled to the top (m=1)	
													{
														//Check if child is < parent. If so, swap them.	
														if (Fcost[openList[m]] < Fcost[openList[m / 2]])
														{
															temp = openList[m / 2];
															openList[m / 2] = openList[m];
															openList[m] = temp;
															m = m / 2;
														}
														else
															break;
													}
													break; //exit for x = loop
												} //If openX(openList(x)) = a
											} //For x = 1 To numberOfOpenListItems
										}//If tempGcost < Gcost(a,b)
									}//else If whichList(a,b) = onOpenList	
								}//If not cutting a corner
							}//If not a wall/obstacle square.
						}//If not already on the closed list 
					}//If not off the map
				}//for (a = parentXval-1; a <= parentXval+1; a++){
			}//for (b = parentYval-1; b <= parentYval+1; b++){
		}//if (numberOfOpenListItems != 0)

		//9.If open list is empty then there is no path.	
		else
		{
			path = nonexistent;
			break;
		}

		//If target is added to open list then path has been found.
		if (whichList[targetX][targetY] == onOpenList)
		{
			path = found;
			break;
		}

	} while (1);//Do until path is found or deemed nonexistent

	//10.Save the path if it exists.
	if (path == found)
	{

		//a.Working backwards from the target to the starting location by checking
		//	each cell's parent, figure out the length of the path.
		pathX = targetX; pathY = targetY;
		do
		{
			//Look up the parent of the current cell.	
			tempx = parentX[pathX][pathY];
			pathY = parentY[pathX][pathY];
			pathX = tempx;

			//Figure out the path length
			pathLength[pathfinderID] = pathLength[pathfinderID] + 1;
		} while (pathX != startX || pathY != startY);

		//b.Resize the data bank to the right size in bytes
		pathBank[pathfinderID] = (int*)realloc(pathBank[pathfinderID],
			pathLength[pathfinderID] * 8);

		//c. Now copy the path information over to the databank. Since we are
		//	working backwards from the target to the start location, we copy
		//	the information to the data bank in reverse order. The result is
		//	a properly ordered set of path data, from the first step to the
		//	last.
		pathX = targetX; pathY = targetY;
		cellPosition = pathLength[pathfinderID] * 2;//start at the end	
		do
		{
			cellPosition = cellPosition - 2;//work backwards 2 integers
			pathBank[pathfinderID][cellPosition] = pathX;
			pathBank[pathfinderID][cellPosition + 1] = pathY;

			//d.Look up the parent of the current cell.	
			tempx = parentX[pathX][pathY];
			pathY = parentY[pathX][pathY];
			pathX = tempx;

			//e.If we have reached the starting square, exit the loop.	
		} while (pathX != startX || pathY != startY);

		//11.Read the first path step into xPath/yPath arrays
		ReadPath(pathfinderID, startingX, startingY, 1);

	}
	pathStatus[0] = path;
	return path;


	//13.If there is no path to the selected target, set the pathfinder's
	//	xPath and yPath equal to its current location and return that the
	//	path is nonexistent.
noPath:
	xPath[pathfinderID] = startingX;
	yPath[pathfinderID] = startingY;
	pathStatus[0] = nonexistent;
	return nonexistent;
}




//==========================================================
//READ PATH DATA: These functions read the path data and convert
//it to screen pixel coordinates.
void TownMap::ReadPath(int pathfinderID, int currentX, int currentY, int pixelsPerFrame)
{
	int ID = pathfinderID; //redundant, but makes the following easier to read

	//If a path has been found for the pathfinder	...
	if (pathStatus[ID] == found)
	{

		//If path finder is just starting a new path or has reached the 
		//center of the current path square (and the end of the path
		//hasn't been reached), look up the next path square.
		if (pathLocation[ID] < pathLength[ID])
		{
			//if just starting or if close enough to center of square
			if (pathLocation[ID] == 0 ||
				(abs(currentX - xPath[ID]) < pixelsPerFrame && abs(currentY - yPath[ID]) < pixelsPerFrame))
				pathLocation[ID] = pathLocation[ID] + 1;
		}

		//Read the path data.		
		xPath[ID] = ReadPathX(ID, pathLocation[ID]);
		yPath[ID] = ReadPathY(ID, pathLocation[ID]);

		//If the center of the last path square on the path has been 
		//reached then reset.
		if (pathLocation[ID] == pathLength[ID])
		{
			if (abs(currentX - xPath[ID]) < pixelsPerFrame
				&& abs(currentY - yPath[ID]) < pixelsPerFrame) //if close enough to center of square
				pathStatus[ID] = notStarted;
		}
	}

	//If there is no path for this pathfinder, simply stay in the current
	//location.
	else
	{
		xPath[ID] = currentX;
		yPath[ID] = currentY;
	}
}
//-----------------------------------------------------------------------------
// Name: ReadPathX
// Desc: Reads the x coordinate of the next path step
//-----------------------------------------------------------------------------
int TownMap::ReadPathX(int pathfinderID, int pathLocation)
{
	int x;
	if (pathLocation <= pathLength[pathfinderID])
	{
		x = pathBank[pathfinderID][pathLocation * 2 - 2];
		x = x + .5;

	}
	return x;
}


//-----------------------------------------------------------------------------
// Name: ReadPathY
// Desc: Reads the y coordinate of the next path step
//-----------------------------------------------------------------------------
int TownMap::ReadPathY(int pathfinderID, int pathLocation)
{
	int y;
	if (pathLocation <= pathLength[pathfinderID])
	{

		y = pathBank[pathfinderID][pathLocation * 2 - 1];
		y = y + .5;

	}
	return y;
}
//////////////////////////////////////////////////////////////////////////////////////


int TownMap::ResetMapInfo(WORD NPC_X, WORD NPC_Y, WORD WP_X, WORD WP_Y, WORD Trans_X, WORD Trans_Y,  WORD StorageX, WORD StorageY)//重新载入地图数据
{
	this->NPC_X = NPC_X;
	this->NPC_Y = NPC_Y;
	this->Waypoint_X = WP_X;
	this->Waypoint_Y = WP_Y;
	this->Trans_X = Trans_X;
	this->Trans_Y = Trans_Y;
	this->Storage_X = StorageX;
	this->Storage_Y = StorageY;
	if (openList != nullptr)
	{
		delete(openList);
		openList = nullptr;
	}
	if (openX != nullptr)
	{
		delete(openX);
		openX = nullptr;
	}
	if (openY != nullptr)
	{
		delete(openY);
		openY = nullptr;
	}
	if (Fcost != nullptr)
	{
		delete(Fcost);
		Fcost = nullptr;
	}
	if (Hcost != nullptr)
	{
		delete(Hcost);
		Hcost = nullptr;
	}
	if (YRow>0)
	{
		for (int i = 0; i<YRow; ++i)
		{
			delete[](walkability[i]);
			delete[](whichList[i]);
			delete[](parentX[i]);
			delete[](parentY[i]);
			delete[](Gcost[i]);
		}
		delete[](whichList[YRow]);
		delete[](parentX[YRow]);
		delete[](parentY[YRow]);
		delete[](Gcost[YRow]);

		delete[](walkability);
		delete[](whichList);
		delete[](parentX);
		delete[](parentY);
		delete[](Gcost);
	}
	fstream fs;
	CString strPath = szModulePath;
	strPath += L"\\nnnn.bat";
	fs.open(strPath, ios::binary | ios::in);
	//fs.open("d:\\map.txt", ios::binary | ios::in);

	fs.read((char*)&XLine, sizeof(short));
	fs.read((char*)&YRow, sizeof(short));

	mapWidth = YRow;
	mapHeight = XLine;
	//申请新缓冲区
	openList = new int[YRow*XLine + 2];
	openX = new int[YRow*XLine + 2]; //1d array stores the x location of an item on the open list
	openY = new int[YRow*XLine + 2]; //1d array stores the y location of an item on the open list
	Fcost = new int[YRow*XLine + 2];	//1d array to store F cost of a cell on the open list
	Hcost = new int[YRow*XLine + 2];

	walkability = new char*[YRow];
	whichList = new int*[YRow + 1];
	parentX = new int*[YRow + 1];
	parentY = new int*[YRow + 1];
	Gcost = new int*[YRow + 1];

	for (int i = 0; i<YRow; i++)
	{
		walkability[i] = new char[XLine];
		fs.read(walkability[i], XLine);

		whichList[i] = new int[XLine + 1];
		parentX[i] = new int[XLine + 1];
		parentY[i] = new int[XLine + 1];
		Gcost[i] = new int[XLine + 1];
	}
	whichList[YRow] = new int[XLine + 1];
	parentX[YRow] = new int[XLine + 1];
	parentY[YRow] = new int[XLine + 1];
	Gcost[YRow] = new int[XLine + 1];

	fs.close();
	
	return SUCCESS;
}

int TownMap::MoveToSellNPC(WORD x, WORD y)
{
	return MoveToPoint(NPC_X,NPC_Y,x, y);
}
int TownMap::MoveToTownWaypoint(WORD x, WORD y)
{
	//MessageBox(NULL, L"sss", NULL, MB_OK);
	return MoveToPoint(Waypoint_X, Waypoint_Y, x, y);
}
int TownMap::MoveToTransferDoor(WORD x, WORD y)
{
	return MoveToPoint(Trans_X, Trans_Y, x, y);
}
int TownMap::MoveToStorage(WORD x, WORD y)
{
	return MoveToPoint(Storage_X, Storage_Y, x, y);
}
//////////////////////////////////////////////////////////////////
int SimpleMap::Connect()
{
	//	string host = "localhost";
	//	int port = 9998;
	//socket.reset(new TSocket(host, port));

	socket.reset(new TPipe("xsxsxs"));
	transport.reset(new TBufferedTransport(socket));
	protocol.reset(new TBinaryProtocol(transport));
	pClient = new GameFuncCallClient(protocol);
	try
	{
		transport->open();
		bConnect = true;
	}
	catch (TTransportException& ttx)
	{
		bConnect = false;
	}
	return 0;
}
SimpleMap::SimpleMap()
{
	openList = nullptr; //1 dimensional array holding ID# of open list items
	openX = nullptr; //1d array stores the x location of an item on the open list
	openY = nullptr; //1d array stores the y location of an item on the open list
	Fcost = nullptr;	//1d array to store F cost of a cell on the open list
	Hcost = nullptr;
	whichList = nullptr;//[mapWidth+1][mapHeight+1];  
	parentX = nullptr;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
	parentY = nullptr;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
	Gcost = nullptr;//[mapWidth+1][mapHeight+1]; 	//2d array to store G cost for each cell.
	memset(pathLength, 0, (numberPeople + 1)*sizeof(int));     //stores length of the found path for critter
	memset(pathLocation, 0, (numberPeople + 1)*sizeof(int));   //stores current position along the chosen path for critter		
	memset(pathBank, 0, (numberPeople + 1)*sizeof(int*));
	//	bInitMap = false;

	XLine = 0;
	YRow = 0;
	HalfX = 0;

	walkability = nullptr;
}
void SimpleMap::DrawMap()
{
	CImage SavePic;
	SavePic.Create(YRow, XLine, 24);
	for (int y = 0; y < YRow; ++y)
	{
		for (int x = 0; x < XLine; ++x)
		{
			if (walkability[y][x] == unwalkable)//不可通行
			{
				SavePic.SetPixel(y, x, RGB(0, 254, 0));
			}
			else
			{
				SavePic.SetPixel(y, x, RGB(0, 0, 254));
			}
		}
	}
	SavePic.Save(L"d:\\saveMapggg.bmp");

}
float CalcDis(Point& p1, Point& p2);


__inline void SimpleMap::AdjustPoint(Point& item)
{

	int x = item.x;
	int y = item.y;

	int X = 0;
	int tx = 0;
	int ty = 0;
	if (walkability[y][x - 10])
		X = 2;
	if (walkability[y][x + 10])//X调整
		++X;
	switch (X)
	{
	case 1:
		tx = x + 5;
		break;
	case 2:
		tx = x - 5;
		break;
	case 0://不动
	case 3://不动
		tx = x;
		break;
	}

	//Y调整
	int Y = 0;
	if (walkability[y - 10][x])
		Y = 2;
	if (walkability[y + 10][x])//X调整
		++Y;
	switch (Y)
	{
	case 1:
		ty = y + 5;
		break;
	case 2:
		ty = y - 5;
		break;
	case 0://不动
	case 3://不动
		ty = y;
		break;
	}

	if (walkability[ty][tx])//调整
	{
		item.x = tx;
		item.y = ty;
	}

	//左右调整

	//上下调整

	//对角调整
}
//int SimpleMap::GetNextExplorePoint(Point& CurPos)
//{
//	float NearstDis = 10000000.0;
//	if (ShadowPoint.size() < 2)
//		return -1;
//	//本组找不到点了
//
//	//本组距离近的,直接删除
//	auto IterNearstPoint = ShadowPoint.end();
//	//
//	for (auto Iter = ShadowPoint.begin(); Iter != ShadowPoint.end();)
//	{
//		int X = abs(Iter->x - CurPos.x);
//		int Y = abs(Iter->y - CurPos.y);
//		float dis = sqrt(X*X + Y*Y);
//		if (dis<20.0)
//		{
//			//距离过小的,直接删除
//			//::Beep(2000, 1000);
//			Iter = ShadowPoint.erase(Iter);
//			if (ShadowPoint.size() < 2)
//				return -1;
//			else
//				continue;
//		}
//		if (NearstDis > dis)
//		{
//			IterNearstPoint = Iter;
//			NearstDis = dis;
//		}
//		++Iter;
//	}
//
//	if (IterNearstPoint != ShadowPoint.end())
//	{
//		//如果距离太远,则用A*寻路
//		if (NearstDis > 50)
//		{
//			if (found != FindPath(CurPos.y, CurPos.x, IterNearstPoint->y, IterNearstPoint->x))
//			{
//				/*	CString log;
//				log.Format(L"player:%d,%d,target:%d,%d", CurPos.x, CurPos.y,  IterNearstPoint->x ,IterNearstPoint->y);
//				MessageBox(NULL, log, NULL, MB_OK);*/
//				ShadowPoint.erase(IterNearstPoint);
//				return -2;//A*不可达
//			}
//			vector<Point> path;
//			GetBestPath(path);
//			for (auto item : path)
//			{
//				ShadowPoint.insert(item);
//			}
//			CurPos.x = path[0].x;
//			CurPos.y = path[0].y;
//		}
//		else
//		{
//			CurPos.x = IterNearstPoint->x;
//			CurPos.y = IterNearstPoint->y;
//		}
//		return 0;
//	}
//	/*if (ShadowPointList.empty())
//	MessageBox(NULL, L"null", NULL, MB_OK);*/
//	return -1;
//}

void SimpleMap::GenShadowPoint()
{
	ShadowPoint.clear();
	vector<Point> PointList;
	Point point;
	for (int y = Shadow_Step; y < (YRow - ChangeStep - 1); y += Shadow_Step)
	{
		for (int x = Shadow_Step; x <(XLine - ChangeStep - 1); x += Shadow_Step)
		{
			if ((x% Shadow_Step != 0) || (y % Shadow_Step != 0))
				continue;
			if (walkability[y][x] == 0)//不可通行
			{

				if (walkability[y - ChangeStep][x])
				{
					point.x = x;
					point.y = y - ChangeStep;
					PointList.push_back(point);
				}
				else if (walkability[y + ChangeStep][x])
				{
					point.x = x;
					point.y = y + ChangeStep;
					PointList.push_back(point);
				}
				else if (walkability[y][x - ChangeStep])
				{
					point.x = x - ChangeStep;
					point.y = y;
					PointList.push_back(point);
				}
				else if (walkability[y][x + ChangeStep])
				{
					point.x = x + ChangeStep;
					point.y = y;
					PointList.push_back(point);
				}
			}
			else//可通行
			{
				point.x = x;
				point.y = y;
				PointList.push_back(point);
			}
		}
	}
	//调整
	Point newPoint;
	for (auto& item : PointList)
	{
		int x = item.x;
		int y = item.y;

		int X = 0;
		int tx = 0;
		int ty = 0;
		if (walkability[y][x - 10])
			X = 2;
		if (walkability[y][x + 10])//X调整
			++X;
		switch (X)
		{
		case 1:
			tx = x + 5;
			break;
		case 2:
			tx = x - 5;
			break;
		case 0://不动
		case 3://不动
			tx = x;
			break;
		}

		//Y调整
		int Y = 0;
		if (walkability[y - 10][x])
			Y = 2;
		if (walkability[y + 10][x])//X调整
			++Y;
		switch (Y)
		{
		case 1:
			ty = y + 5;
			break;
		case 2:
			ty = y - 5;
			break;
		case 0://不动
		case 3://不动
			ty = y;
			break;
		}

		if (walkability[ty][tx])//调整
		{
			newPoint.x = tx;
			newPoint.y = ty;
			ShadowPoint.insert(newPoint);
		}
		else
			ShadowPoint.insert(item);
	}

	dbSingleTotalShadowSize = ShadowPoint.size();
}

//#include <IOCPCheckClient.h>

int SimpleMap::GetNextExplorePoint(Point& CurPos)
{
	float NearstDis = 100000000.0;
	auto IterNearstPoint = ShadowPoint.end();
	for (auto Iter = ShadowPoint.begin(); Iter != ShadowPoint.end();)
	{
		int X = abs(Iter->x - CurPos.x);
		int Y = abs(Iter->y - CurPos.y);
		float dis = sqrt(X*X + Y*Y);
		if (dis<20.0)
		{
			//距离过小的,直接删除
			//::Beep(2000, 1000);
			Iter = ShadowPoint.erase(Iter);
			continue;
		}
		if (NearstDis > dis)
		{
			IterNearstPoint = Iter;
			NearstDis = dis;
		}
		++Iter;
	}
	//反破解系统
//#ifdef ENCRYPT_VMP
//	VMProtectBegin("GetExplorePoint");
//#endif
//	if (CrackCount > 8000)
//	{
//
//		double size = ShadowPointList.size();
//		double fff = size / dbSingleTotalShadowSize;
//		if (fff < 0.4)
//		{
//			GenShadowPoint();
//			::InterlockedExchangeAdd(&CrackCount, -500);
//		}
//	}
//#ifdef ENCRYPT_VMP
//	VMProtectEnd();
//#endif
	//10%不探索优化
	//if (NearstDis > 200)
	//{
	//	double size = ShadowPointList.size();
	//	double fff = size / dbSingleTotalShadowSize;
	//	if (fff < 0.1)
	//	{
	//		/*	CString log;
	//		log.Format(L"%f,%f,%f", size, dbTotalShadowSize, fff);
	//		MessageBox(NULL,log.GetBuffer(),NULL,MB_OK);*/
	//		return 1;
	//	}
	//}
	if (IterNearstPoint != ShadowPoint.end())
	{
		//如果距离太远,则用A*寻路
		if (NearstDis > 50)
		{
			//CurPos.x = IterNearstPoint->x;
			//CurPos.y = IterNearstPoint->y;
			if (found != FindPath(CurPos.y, CurPos.x, IterNearstPoint->y, IterNearstPoint->x))
			{
				/*	CString log;
				log.Format(L"player:%d,%d,target:%d,%d", CurPos.x, CurPos.y,  IterNearstPoint->x ,IterNearstPoint->y);
				MessageBox(NULL, log, NULL, MB_OK);*/
				ShadowPoint.erase(IterNearstPoint);
				return 2;
			}
			vector<Point> path;
			GetBestPath(path);
			for (auto item : path)
			{
				AdjustPoint(item);
				ShadowPoint.insert(item);
			}
			CurPos.x = path[0].x;
			CurPos.y = path[0].y;
		}
		else
		{
			CurPos.x = IterNearstPoint->x;
			CurPos.y = IterNearstPoint->y;
		}
		return 0;
	}
	return 1;
}
int SimpleMap::GetAstarDis(Point& CurPos, Point& TargetPos)
{
	if (found == FindPath(CurPos.y, CurPos.x, TargetPos.y, TargetPos.x))
	{
		return pathLength[0];
	}
	return -1;
}
int SimpleMap::MoveToPoint(unsigned short x, unsigned y, unsigned short px, unsigned short py)
{
	int dx = 0;
	int dy = 0;
	if (found != FindPath(py, px, y, x))
	{
		//设置不可达点
		Point p;
		p.x = x;
		p.y = y;
		//	::OutputDebugStringW(L"ggggg111111");
		SetExploredPoint(p);
		//	::OutputDebugStringW(L"ggggg33333");
		return 1;
	}
	//	::OutputDebugStringW(L"ggggg44444");
	GetDirectPoint(dx, dy);//获取A*第一个转向点
	Point temp;
	temp.x = dx;
	temp.y = dy;
	AdjustPoint(temp);
	//	::OutputDebugStringW(L"ggggg55555");
	pClient->Move(temp.x, temp.y);
	//	::OutputDebugStringW(L"ggggg66666");
	return SUCCESS;
};


//////////////////////原A*部分函数
int SimpleMap::GetBestPath(std::vector<Point>& Path)
{
	int x = 0, y = 0;
	int ID = 0;
	int nCurDirect = 100;
	Point FatherPoint;
	Point bPoint;
	if (pathStatus[ID] == found)
	{
		if (pathLength[ID] <= STEP_NUM)
		{
			Point CurPoint;
			CurPoint.y = pathBank[ID][(pathLength[ID] - 1) * 2];
			CurPoint.x = pathBank[ID][(pathLength[ID] - 1) * 2 + 1];
			Path.push_back(CurPoint);
		}
		else
		{

			for (int i = STEP_NUM; i < (pathLength[ID]); i += STEP_NUM)
			{
				Point CurPoint;
				CurPoint.y = pathBank[ID][i * 2];
				CurPoint.x = pathBank[ID][i * 2 + 1];

				Path.push_back(CurPoint);

			}
		}
	}
	return Path.size();
}

int SimpleMap::ChangeNotNearWall(int& x, int& y)//切换点不靠墙
{

	int X = 0;
	int tx = 0;
	int ty = 0;
	if (walkability[y][x - 10])
		X = 2;
	if (walkability[y][x + 10])//X调整
		++X;
	switch (X)
	{
	case 1:
		tx = x + 5;
		break;
	case 2:
		tx = x - 5;
		break;
	case 0://不动
	case 3://不动
		tx = x;
		break;
	}

	//Y调整
	int Y = 0;
	if (walkability[y - 10][x])
		Y = 2;
	if (walkability[y + 10][x])//X调整
		++Y;
	switch (Y)
	{
	case 1:
		ty = y + 5;
		break;
	case 2:
		ty = y - 5;
		break;
	case 0://不动
	case 3://不动
		ty = y;
		break;
	}

	if (walkability[ty][tx])//调整
	{
		x = tx;
		y = ty;
	}
	return 0;
}
int SimpleMap::GetDirectPoint(int& retx, int& rety)
{
	int x = 0, y = 0;
	int ID = 0;
	int nCurDirect = 100;
	Point FatherPoint;
	Point bPoint;
	if (pathStatus[ID] == found)
	{
		//bPoint.x = pathBank[ID][0];
		//bPoint.y = pathBank[ID][1];
		//FatherPoint.x = pathBank[ID][2];
		//FatherPoint.y = pathBank[ID][3];
		//nCurDirect = GetDirect(FatherPoint, bPoint);

		if (pathLength[ID] < 16)
		{
			rety = pathBank[ID][(pathLength[ID] - 1) * 2];
			retx = pathBank[ID][(pathLength[ID] - 1) * 2 + 1];
		}
		else
		{
			rety = pathBank[ID][14 * 2];
			retx = pathBank[ID][14 * 2 + 1];
		}
		/*if (rety < 0)
		_asm int 3;*/
		//边界优化
		//	ChangeNotNearWall(retx, rety);
		return SUCCESS;
		//for (int i = 2; i<pathLength[ID] - 1; ++i)
		//{
		//	Point CurPoint;
		//	CurPoint.x = pathBank[ID][i * 2];
		//	CurPoint.y = pathBank[ID][i * 2 + 1];
		//	int nDirect = GetDirect(CurPoint, FatherPoint);
		//	if (nDirect != nCurDirect)
		//	{
		//		//如果方向不一致
		//		retx = y;
		//		rety = x;
		//		return SUCCESS;
		//	}
		//	FatherPoint = CurPoint;
		//}	
	}
	return 1;
}
void SimpleMap::SetPassAbleArea(short x, short y)
{
	for (int i = (y - 5); i < (y + 5); ++i)
	{
		for (int j = (x - 5); j < (x + 5); ++j)
		{
			walkability[i][j] = walkable;
		}
	}
}
void SimpleMap::InitializePathfinder(void)
{
	for (int x = 0; x < numberPeople + 1; x++)
		pathBank[x] = (int*)malloc(4);
}

//-----------------------------------------------------------------------------
// Name: EndPathfinder
// Desc: Frees memory used by the pathfinder.
//-----------------------------------------------------------------------------
void SimpleMap::EndPathfinder(void)
{
	for (int x = 0; x < numberPeople + 1; x++)
	{
		free(pathBank[x]);
	}
}


//-----------------------------------------------------------------------------
// Name: FindPath
// Desc: Finds a path using A*
//-----------------------------------------------------------------------------
int SimpleMap::FindPath(int startingX, int startingY,
	int targetX, int targetY)
{
	int pathfinderID = 0;
	int onOpenList = 0, parentXval = 0, parentYval = 0,
		a = 0, b = 0, m = 0, u = 0, v = 0, temp = 0, corner = 0, numberOfOpenListItems = 0,
		addedGCost = 0, tempGcost = 0, path = 0,
		tempx, pathX, pathY, cellPosition,
		newOpenListItemID = 0;

	//1. Convert location data (in pixels) to coordinates in the walkability array.
	//::InterlockedExchangeAdd(&CrackCount, 1);
	int startX = startingX / tileSize;
	int startY = startingY / tileSize;
	targetX = targetX / tileSize;
	targetY = targetY / tileSize;
	//2.Quick Path Checks: Under the some circumstances no path needs to
	//	be generated ...

	//	If starting location and target are in the same location...
	if (startX == targetX && startY == targetY && pathLocation[pathfinderID] > 0)
	{
		pathStatus[0] = found;
		return found;
	}
	if (startX == targetX && startY == targetY && pathLocation[pathfinderID] == 0)
	{
		pathStatus[0] = nonexistent;
		return nonexistent;
	}
	//	If target square is unwalkable, return that it's a nonexistent path.
	if (walkability[targetX][targetY] != walkable)
		goto noPath;

	//3.Reset some variables that need to be cleared
	if (onClosedList > 1000000) //reset whichList occasionally
	{
		for (int x = 0; x < mapWidth; x++)
		{
			for (int y = 0; y < mapHeight; y++)
				whichList[x][y] = 0;
		}
		onClosedList = 10;
	}
	onClosedList = onClosedList + 2; //changing the values of onOpenList and onClosed list is faster than redimming whichList() array
	onOpenList = onClosedList - 1;
	pathLength[pathfinderID] = notStarted;//i.e, = 0
	pathLocation[pathfinderID] = notStarted;//i.e, = 0
	Gcost[startX][startY] = 0; //reset starting square's G value to 0

	//4.Add the starting location to the open list of squares to be checked.
	numberOfOpenListItems = 1;
	openList[1] = 1;//assign it as the top (and currently only) item in the open list, which is maintained as a binary heap (explained below)
	openX[1] = startX; openY[1] = startY;

	//5.Do the following until a path is found or deemed nonexistent.
	do
	{

		//6.If the open list is not empty, take the first cell off of the list.
		//	This is the lowest F cost cell on the open list.
		if (numberOfOpenListItems != 0)
		{

			//7. Pop the first item off the open list.
			parentXval = openX[openList[1]];
			parentYval = openY[openList[1]]; //record cell coordinates of the item
			whichList[parentXval][parentYval] = onClosedList;//add the item to the closed list

			//	Open List = Binary Heap: Delete this item from the open list, which
			//  is maintained as a binary heap. For more information on binary heaps, see:
			//	http://www.policyalmanac.org/games/binaryHeaps.htm
			numberOfOpenListItems = numberOfOpenListItems - 1;//reduce number of open list items by 1	

			//	Delete the top item in binary heap and reorder the heap, with the lowest F cost item rising to the top.
			openList[1] = openList[numberOfOpenListItems + 1];//move the last item in the heap up to slot #1
			v = 1;

			//	Repeat the following until the new item in slot #1 sinks to its proper spot in the heap.
			do
			{
				u = v;
				if (2 * u + 1 <= numberOfOpenListItems) //if both children exist
				{
					//Check if the F cost of the parent is greater than each child.
					//Select the lowest of the two children.
					if (Fcost[openList[u]] >= Fcost[openList[2 * u]])
						v = 2 * u;
					if (Fcost[openList[v]] >= Fcost[openList[2 * u + 1]])
						v = 2 * u + 1;
				}
				else
				{
					if (2 * u <= numberOfOpenListItems) //if only child #1 exists
					{
						//Check if the F cost of the parent is greater than child #1	
						if (Fcost[openList[u]] >= Fcost[openList[2 * u]])
							v = 2 * u;
					}
				}

				if (u != v) //if parent's F is > one of its children, swap them
				{
					temp = openList[u];
					openList[u] = openList[v];
					openList[v] = temp;
				}
				else
					break; //otherwise, exit loop

			} while (1);//reorder the binary heap


			//7.Check the adjacent squares. (Its "children" -- these path children
			//	are similar, conceptually, to the binary heap children mentioned
			//	above, but don't confuse them. They are different. Path children
			//	are portrayed in Demo 1 with grey pointers pointing toward
			//	their parents.) Add these adjacent child squares to the open list
			//	for later consideration if appropriate (see various if statements
			//	below).
			for (b = parentYval - 1; b <= parentYval + 1; b++)
			{
				for (a = parentXval - 1; a <= parentXval + 1; a++)
				{

					//	If not off the map (do this first to avoid array out-of-bounds errors)
					if (a != -1 && b != -1 && a != mapWidth && b != mapHeight)
					{

						//	If not already on the closed list (items on the closed list have
						//	already been considered and can now be ignored).			
						if (whichList[a][b] != onClosedList)
						{

							//	If not a wall/obstacle square.
							if (walkability[a][b] == walkable)
							{

								//	Don't cut across corners
								corner = walkable;
								if (a == parentXval - 1)
								{
									if (b == parentYval - 1)
									{
										if (walkability[parentXval - 1][parentYval] != walkable
											|| walkability[parentXval][parentYval - 1] != walkable)
											corner = unwalkable;
									}
									else if (b == parentYval + 1)
									{
										if (walkability[parentXval][parentYval + 1] != walkable
											|| walkability[parentXval - 1][parentYval] != walkable)
											corner = unwalkable;
									}
								}
								else if (a == parentXval + 1)
								{
									if (b == parentYval - 1)
									{
										if (walkability[parentXval][parentYval - 1] != walkable
											|| walkability[parentXval + 1][parentYval] != walkable)
											corner = unwalkable;
									}
									else if (b == parentYval + 1)
									{
										if (walkability[parentXval + 1][parentYval] != walkable
											|| walkability[parentXval][parentYval + 1] != walkable)
											corner = unwalkable;
									}
								}
								if (corner == walkable)
								{

									//	If not already on the open list, add it to the open list.			
									if (whichList[a][b] != onOpenList)
									{

										//Create a new open list item in the binary heap.
										newOpenListItemID = newOpenListItemID + 1; //each new item has a unique ID #
										m = numberOfOpenListItems + 1;
										openList[m] = newOpenListItemID;//place the new open list item (actually, its ID#) at the bottom of the heap
										openX[newOpenListItemID] = a;
										openY[newOpenListItemID] = b;//record the x and y coordinates of the new item

										//Figure out its G cost
										if (abs(a - parentXval) == 1 && abs(b - parentYval) == 1)
											addedGCost = 14;//cost of going to diagonal squares	
										else
											addedGCost = 10;//cost of going to non-diagonal squares				
										Gcost[a][b] = Gcost[parentXval][parentYval] + addedGCost;

										//Figure out its H and F costs and parent
										Hcost[openList[m]] = 10 * (abs(a - targetX) + abs(b - targetY));
										Fcost[openList[m]] = Gcost[a][b] + Hcost[openList[m]];
										parentX[a][b] = parentXval; parentY[a][b] = parentYval;

										//Move the new open list item to the proper place in the binary heap.
										//Starting at the bottom, successively compare to parent items,
										//swapping as needed until the item finds its place in the heap
										//or bubbles all the way to the top (if it has the lowest F cost).
										while (m != 1) //While item hasn't bubbled to the top (m=1)	
										{
											//Check if child's F cost is < parent's F cost. If so, swap them.	
											if (Fcost[openList[m]] <= Fcost[openList[m / 2]])
											{
												temp = openList[m / 2];
												openList[m / 2] = openList[m];
												openList[m] = temp;
												m = m / 2;
											}
											else
												break;
										}
										numberOfOpenListItems = numberOfOpenListItems + 1;//add one to the number of items in the heap

										//Change whichList to show that the new item is on the open list.
										whichList[a][b] = onOpenList;
									}

									//8.If adjacent cell is already on the open list, check to see if this 
									//	path to that cell from the starting location is a better one. 
									//	If so, change the parent of the cell and its G and F costs.	
									else //If whichList(a,b) = onOpenList
									{

										//Figure out the G cost of this possible new path
										if (abs(a - parentXval) == 1 && abs(b - parentYval) == 1)
											addedGCost = 14;//cost of going to diagonal tiles	
										else
											addedGCost = 10;//cost of going to non-diagonal tiles				
										tempGcost = Gcost[parentXval][parentYval] + addedGCost;

										//If this path is shorter (G cost is lower) then change
										//the parent cell, G cost and F cost. 		
										if (tempGcost < Gcost[a][b]) //if G cost is less,
										{
											parentX[a][b] = parentXval; //change the square's parent
											parentY[a][b] = parentYval;
											Gcost[a][b] = tempGcost;//change the G cost			

											//Because changing the G cost also changes the F cost, if
											//the item is on the open list we need to change the item's
											//recorded F cost and its position on the open list to make
											//sure that we maintain a properly ordered open list.
											for (int x = 1; x <= numberOfOpenListItems; x++) //look for the item in the heap
											{
												if (openX[openList[x]] == a && openY[openList[x]] == b) //item found
												{
													Fcost[openList[x]] = Gcost[a][b] + Hcost[openList[x]];//change the F cost

													//See if changing the F score bubbles the item up from it's current location in the heap
													m = x;
													while (m != 1) //While item hasn't bubbled to the top (m=1)	
													{
														//Check if child is < parent. If so, swap them.	
														if (Fcost[openList[m]] < Fcost[openList[m / 2]])
														{
															temp = openList[m / 2];
															openList[m / 2] = openList[m];
															openList[m] = temp;
															m = m / 2;
														}
														else
															break;
													}
													break; //exit for x = loop
												} //If openX(openList(x)) = a
											} //For x = 1 To numberOfOpenListItems
										}//If tempGcost < Gcost(a,b)
									}//else If whichList(a,b) = onOpenList	
								}//If not cutting a corner
							}//If not a wall/obstacle square.
						}//If not already on the closed list 
					}//If not off the map
				}//for (a = parentXval-1; a <= parentXval+1; a++){
			}//for (b = parentYval-1; b <= parentYval+1; b++){
		}//if (numberOfOpenListItems != 0)

		//9.If open list is empty then there is no path.	
		else
		{
			path = nonexistent;
			break;
		}

		//If target is added to open list then path has been found.
		if (whichList[targetX][targetY] == onOpenList)
		{
			path = found;
			break;
		}

	} while (1);//Do until path is found or deemed nonexistent

	//10.Save the path if it exists.
	if (path == found)
	{

		//a.Working backwards from the target to the starting location by checking
		//	each cell's parent, figure out the length of the path.
		pathX = targetX; pathY = targetY;
		do
		{
			//Look up the parent of the current cell.	
			tempx = parentX[pathX][pathY];
			pathY = parentY[pathX][pathY];
			pathX = tempx;

			//Figure out the path length
			pathLength[pathfinderID] = pathLength[pathfinderID] + 1;
		} while (pathX != startX || pathY != startY);

		//b.Resize the data bank to the right size in bytes
		pathBank[pathfinderID] = (int*)realloc(pathBank[pathfinderID],
			pathLength[pathfinderID] * 8);

		//c. Now copy the path information over to the databank. Since we are
		//	working backwards from the target to the start location, we copy
		//	the information to the data bank in reverse order. The result is
		//	a properly ordered set of path data, from the first step to the
		//	last.
		pathX = targetX; pathY = targetY;
		cellPosition = pathLength[pathfinderID] * 2;//start at the end	
		do
		{
			cellPosition = cellPosition - 2;//work backwards 2 integers
			pathBank[pathfinderID][cellPosition] = pathX;
			pathBank[pathfinderID][cellPosition + 1] = pathY;

			//d.Look up the parent of the current cell.	
			tempx = parentX[pathX][pathY];
			pathY = parentY[pathX][pathY];
			pathX = tempx;

			//e.If we have reached the starting square, exit the loop.	
		} while (pathX != startX || pathY != startY);

		//11.Read the first path step into xPath/yPath arrays
		ReadPath(pathfinderID, startingX, startingY, 1);

	}
	pathStatus[0] = path;
	return path;


	//13.If there is no path to the selected target, set the pathfinder's
	//	xPath and yPath equal to its current location and return that the
	//	path is nonexistent.
noPath:
	xPath[pathfinderID] = startingX;
	yPath[pathfinderID] = startingY;
	pathStatus[0] = nonexistent;
	return nonexistent;
}




//==========================================================
//READ PATH DATA: These functions read the path data and convert
//it to screen pixel coordinates.
void SimpleMap::ReadPath(int pathfinderID, int currentX, int currentY, int pixelsPerFrame)
{
	/*
	;	Note on PixelsPerFrame: The need for this parameter probably isn't
	;	that obvious, so a little explanation is in order. This
	;	parameter is used to determine if the pathfinder has gotten close
	;	enough to the center of a given path square to warrant looking up
	;	the next step on the path.
	;
	;	This is needed because the speed of certain sprites can
	;	make reaching the exact center of a path square impossible.
	;	In Demo #2, the chaser has a velocity of 3 pixels per frame. Our
	;	tile size is 50 pixels, so the center of a tile will be at location
	;	25, 75, 125, etc. Some of these are not evenly divisible by 3, so
	;	our pathfinder has to know how close is close enough to the center.
	;	It calculates this by seeing if the pathfinder is less than
	;	pixelsPerFrame # of pixels from the center of the square.

	;	This could conceivably cause problems if you have a *really* fast
	;	sprite and/or really small tiles, in which case you may need to
	;	adjust the formula a bit. But this should almost never be a problem
	;	for games with standard sized tiles and normal speeds. Our smiley
	;	in Demo #4 moves at a pretty fast clip and it isn't even close
	;	to being a problem.
	*/

	int ID = pathfinderID; //redundant, but makes the following easier to read

	//If a path has been found for the pathfinder	...
	if (pathStatus[ID] == found)
	{

		//If path finder is just starting a new path or has reached the 
		//center of the current path square (and the end of the path
		//hasn't been reached), look up the next path square.
		if (pathLocation[ID] < pathLength[ID])
		{
			//if just starting or if close enough to center of square
			if (pathLocation[ID] == 0 ||
				(abs(currentX - xPath[ID]) < pixelsPerFrame && abs(currentY - yPath[ID]) < pixelsPerFrame))
				pathLocation[ID] = pathLocation[ID] + 1;
		}

		//Read the path data.		
		xPath[ID] = ReadPathX(ID, pathLocation[ID]);
		yPath[ID] = ReadPathY(ID, pathLocation[ID]);

		//If the center of the last path square on the path has been 
		//reached then reset.
		if (pathLocation[ID] == pathLength[ID])
		{
			if (abs(currentX - xPath[ID]) < pixelsPerFrame
				&& abs(currentY - yPath[ID]) < pixelsPerFrame) //if close enough to center of square
				pathStatus[ID] = notStarted;
		}
	}

	//If there is no path for this pathfinder, simply stay in the current
	//location.
	else
	{
		xPath[ID] = currentX;
		yPath[ID] = currentY;
	}
}
//-----------------------------------------------------------------------------
// Name: ReadPathX
// Desc: Reads the x coordinate of the next path step
//-----------------------------------------------------------------------------
int SimpleMap::ReadPathX(int pathfinderID, int pathLocation)
{
	int x;
	if (pathLocation <= pathLength[pathfinderID])
	{

		//Read coordinate from bank
		x = pathBank[pathfinderID][pathLocation * 2 - 2];

		//Adjust the coordinates so they align with the center
		//of the path square (optional). This assumes that you are using
		//sprites that are centered -- i.e., with the midHandle command.
		//Otherwise you will want to adjust this.
		//x = tileSize*x + .5*tileSize;
		x = x + .5;

	}
	return x;
}


//-----------------------------------------------------------------------------
// Name: ReadPathY
// Desc: Reads the y coordinate of the next path step
//-----------------------------------------------------------------------------
int SimpleMap::ReadPathY(int pathfinderID, int pathLocation)
{
	int y;
	if (pathLocation <= pathLength[pathfinderID])
	{

		//Read coordinate from bank
		y = pathBank[pathfinderID][pathLocation * 2 - 1];

		//Adjust the coordinates so they align with the center
		//of the path square (optional). This assumes that you are using
		//sprites that are centered -- i.e., with the midHandle command.
		//Otherwise you will want to adjust this.
		//y = tileSize*y + .5*tileSize;
		y = y + .5;

	}
	return y;
}
//////////////////////////////////////////////////////////////////////////////////////


void SimpleMap::SetExploredPoint(Point& TargetPos)
{
	auto iter = ShadowPoint.find(TargetPos);
	if (iter != ShadowPoint.end())
		ShadowPoint.erase(iter);
}

int SimpleMap::ResetMapInfo()//重新载入地图数据
{
	this->bGroupModel = bGroupModel;
	if (openList != nullptr)
	{
		delete(openList);
		openList = nullptr;
	}
	if (openX != nullptr)
	{
		delete(openX);
		openX = nullptr;
	}
	if (openY != nullptr)
	{
		delete(openY);
		openY = nullptr;
	}
	if (Fcost != nullptr)
	{
		delete(Fcost);
		Fcost = nullptr;
	}
	if (Hcost != nullptr)
	{
		delete(Hcost);
		Hcost = nullptr;
	}
	if (YRow>0)
	{
		for (int i = 0; i<YRow; ++i)
		{
			delete[](walkability[i]);
			delete[](whichList[i]);
			delete[](parentX[i]);
			delete[](parentY[i]);
			delete[](Gcost[i]);
		}
		delete[](whichList[YRow]);
		delete[](parentX[YRow]);
		delete[](parentY[YRow]);
		delete[](Gcost[YRow]);

		delete[](walkability);
		delete[](whichList);
		delete[](parentX);
		delete[](parentY);
		delete[](Gcost);
	}
	fstream fs;
	CString strPath = szModulePath;
	strPath += L"\\nnnn.bat";
	fs.open(strPath, ios::binary | ios::in);
	//	fs.open("d:\\map.txt", ios::binary | ios::in);

	fs.read((char*)&XLine, sizeof(short));
	fs.read((char*)&YRow, sizeof(short));

	mapWidth = YRow;
	mapHeight = XLine;
	//申请新缓冲区
	openList = new int[YRow*XLine + 2];
	openX = new int[YRow*XLine + 2]; //1d array stores the x location of an item on the open list
	openY = new int[YRow*XLine + 2]; //1d array stores the y location of an item on the open list
	Fcost = new int[YRow*XLine + 2];	//1d array to store F cost of a cell on the open list
	Hcost = new int[YRow*XLine + 2];

	walkability = new char*[YRow];
	whichList = new int*[YRow + 1];
	parentX = new int*[YRow + 1];
	parentY = new int*[YRow + 1];
	Gcost = new int*[YRow + 1];

	for (int i = 0; i<YRow; i++)
	{
		walkability[i] = new char[XLine];
		fs.read(walkability[i], XLine);

		whichList[i] = new int[XLine + 1];
		parentX[i] = new int[XLine + 1];
		parentY[i] = new int[XLine + 1];
		Gcost[i] = new int[XLine + 1];
	}
	whichList[YRow] = new int[XLine + 1];
	parentX[YRow] = new int[XLine + 1];
	parentY[YRow] = new int[XLine + 1];
	Gcost[YRow] = new int[XLine + 1];

	fs.close();

	GenShadowPoint();
	return SUCCESS;
}

bool SimpleMap::IsObstaclePoint(int x, int y)
{
	/*__try
	{*/
	if (x >= XLine || y >= YRow || x <= 0 || y <= 0)
		return false;
	if (walkability[y][x] == 0)
		return true;
	//}
	//__except(1){}
	return false;
}

bool SimpleMap::AreThereObstacle(int StartX, int StartY, int EndX, int EndY)
{
	/*CString log;
	log.Format(L"ggggnew:::call:%d,%d,    %d,%d", StartX, StartY, EndX, EndY);
	::OutputDebugStringW(log);*/
	int dx = abs(EndX - StartX),
		dy = abs(EndY - StartY),
		yy = 0;

	if (dx < dy)
	{
		yy = 1;
		swap_int(&StartX, &StartY);
		swap_int(&EndX, &EndY);
		swap_int(&dx, &dy);
	}
	int ix = (EndX - StartX) > 0 ? 1 : -1,
		iy = (EndY - StartY) > 0 ? 1 : -1,
		cx = StartX,
		cy = StartY,
		n2dy = dy * 2,
		n2dydx = (dy - dx) * 2,
		d = dy * 2 - dx;
	if (yy)
	{
		while (cx != EndX)
		{
			if (d < 0)
			{
				d += n2dy;
			}
			else
			{
				cy += iy;
				d += n2dydx;
			}

		/*	log.Format(L"gggg%d,%d,    %d,%d", cx, cy, YRow, XLine);
			::OutputDebugStringW(log);*/
			if (walkability[cx][cy] == unwalkable)
			{
			//	::OutputDebugStringW(L"gggggeeeeee");
				return true;
			}
		//	::OutputDebugStringW(L"gggggeeeeee");
			cx += ix;
		}
	}
	else
	{
		while (cx != EndX)
		{
			if (d < 0)
			{
				d += n2dy;
			}
			else
			{
				cy += iy;
				d += n2dydx;
			}

	/*		log.Format(L"gggg%d,%d,    %d,%d", cx, cy, YRow, XLine);
			::OutputDebugStringW(log);*/
			if (walkability[cx][cy] == unwalkable)
			{
			//	::OutputDebugStringW(L"gggggeeeeee");
				return true;
			}
	//		::OutputDebugStringW(L"gggggeeeeee");
			cx += ix;
		}
	}
	return false;
}
