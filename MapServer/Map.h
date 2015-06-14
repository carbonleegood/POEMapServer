#pragma once
#include <set>
#include <vector>
#include <map>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include <thrift/transport/TPipe.h>
#include <thrift/TProcessor.h>
#include "GameFuncCall.h"
using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::thrift::GameCall;
struct Point
{
	int x;
	int y;
	Point(int x,int y)
	{
		this->x=x;
		this->y=y;
	};
	Point()
	{
		x=0;
		y=0;
	};
	bool operator ==(Point p)
	{
		if(this->x==p.x&&this->y==p.y)
		{
			return true;
		}
		return false;
	}
	bool operator !=(Point p)
	{
		if(this->x==p.x&&this->y==p.y)
		{
			return false;
		}
		return true;
	}
	bool operator <(Point p) const
	{
		if(this->x==p.x)
		{
			return this->y<p.y;
		}
		else
			return this->x<p.x;
	};
	Point& operator =(const Point& p)
	{
		this->x=p.x;
		this->y=p.y;
		return *this;
	}
};

#define tileSize 1
#define numberPeople  1
#define  notfinished 0
#define notStarted  0// path-related constants
#define  found  1
#define nonexistent   2 
#define  walkable   1
#define unwalkable   0// walkability array constants
//#define nStep 10
class Map
{
	int onClosedList;
	int mapWidth;
	int mapHeight;

	int xPath[numberPeople + 1];
	int yPath[numberPeople + 1];

	int *openList; //1 dimensional array holding ID# of open list items
	int *openX; //1d array stores the x location of an item on the open list
	int *openY; //1d array stores the y location of an item on the open list
	int *Fcost;	//1d array to store F cost of a cell on the open list
	int *Hcost;
	int **whichList;//[mapWidth+1][mapHeight+1];  
	int **parentX;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
	int **parentY;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
	int **Gcost;//[mapWidth+1][mapHeight+1]; 	//2d array to store G cost for each cell.


	int pathLength[numberPeople + 1];     //stores length of the found path for critter
	int pathLocation[numberPeople + 1];   //stores current position along the chosen path for critter		
	int* pathBank[numberPeople + 1];
	int pathStatus[numberPeople + 1];

	void InitializePathfinder(void);
	void EndPathfinder(void);
	void ReadPath(int pathfinderID, int currentX, int currentY, int pixelsPerFrame);
	int ReadPathX(int pathfinderID, int pathLocation);
	int ReadPathY(int pathfinderID, int pathLocation);
	int FindPath(int startingX, int startingY, int targetX, int targetY);
	int GetBestPath(std::vector<Point>& Path);
	//////////////////////////////
	bool bConnect;
	ULONG XLine;
	ULONG YRow;
	ULONG HalfX;

	char **walkability;
	std::vector<double> dbTotalShadowSize;
	double dbSingleTotalShadowSize;
	std::set<Point> ShadowPointList;

	std::vector<std::set<Point> > ShadowPointGroup;
	bool bGroupModel;
	//////////////
	//boost::shared_ptr<TSocket> socket;
	boost::shared_ptr<TPipe> socket;
	boost::shared_ptr<TTransport> transport;
	boost::shared_ptr<TBinaryProtocol> protocol;
	GameFuncCallClient* pClient;
	
	void GenShadowPoint();
	void GroupShadowPoint();
	
	bool AreThereObstacle(int StartX, int StartY, int EndX, int EndY);
	
	int GetDirectPoint(int& x, int& y);
	int ChangeNotNearWall(int& x, int& y);
	
public:
	volatile LONG CrackCount;
	void SetExploredPoint(Point& TargetPos);
	int GetNextExplorePoint(Point& CurPos);


	int GetGroup(int x, int y);//获取当前分组
	int GetGroupCount();
	void SetGroupExploredPoint(Point& TargetPos, int nGroup);
	int GetNextGroupExplorePoint(Point& CurPos, int nGroup);
	__inline void AdjustPoint(Point& item);
	void SetPassAbleArea(short x, short y);

	int GetAstarDis(Point& CurPos, Point& TargetPos);
	//

	int ResetMapInfo(bool bGroupModel);//重新载入地图数据
	bool IsObstaclePoint(int x,int y);

	int MoveToPoint(unsigned short x, unsigned y, unsigned short px, unsigned short py);

	Map();
	int Connect();
	//~Map(void);
	void DrawMap();
	void GetMapData();
};

class TownMap
{
	int onClosedList;
	int mapWidth;
	int mapHeight;

	int xPath[numberPeople + 1];
	int yPath[numberPeople + 1];

	int *openList; //1 dimensional array holding ID# of open list items
	int *openX; //1d array stores the x location of an item on the open list
	int *openY; //1d array stores the y location of an item on the open list
	int *Fcost;	//1d array to store F cost of a cell on the open list
	int *Hcost;
	int **whichList;//[mapWidth+1][mapHeight+1];  
	int **parentX;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
	int **parentY;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
	int **Gcost;//[mapWidth+1][mapHeight+1]; 	//2d array to store G cost for each cell.


	int pathLength[numberPeople + 1];     //stores length of the found path for critter
	int pathLocation[numberPeople + 1];   //stores current position along the chosen path for critter		
	int* pathBank[numberPeople + 1];
	int pathStatus[numberPeople + 1];

	void InitializePathfinder(void);
	void EndPathfinder(void);
	void ReadPath(int pathfinderID, int currentX, int currentY, int pixelsPerFrame);
	int ReadPathX(int pathfinderID, int pathLocation);
	int ReadPathY(int pathfinderID, int pathLocation);
	int FindPath(int startingX, int startingY, int targetX, int targetY);
	int GetBestPath(std::vector<Point>& Path);
	//////////////////////////////
	bool bConnect;
	ULONG XLine;
	ULONG YRow;
	ULONG HalfX;

	char **walkability;

	//////////////
	//boost::shared_ptr<TSocket> socket;
	boost::shared_ptr<TPipe> socket;
	boost::shared_ptr<TTransport> transport;
	boost::shared_ptr<TBinaryProtocol> protocol;
	GameFuncCallClient* pClient;

	int NPC_X;
	int NPC_Y;
	int Waypoint_X;
	int Waypoint_Y;
	int Trans_X;
	int Trans_Y;
	int Storage_X;
	int Storage_Y;

	int GetDirectPoint(int& x, int& y);
	int ChangeNotNearWall(int& x, int& y);

public:
	char szUID[42] ;
	char szPWD[22] ;
	//
	int ResetMapInfo(WORD NPC_X, WORD NPC_Y, WORD WP_X, WORD WP_Y, WORD Trans_X, WORD Trancs_Y,WORD StorageX,WORD StorageY);//重新载入地图数据
	int MoveToPoint(unsigned short x, unsigned y, unsigned short px, unsigned short py);
	int MoveToSellNPC(WORD x, WORD y);
	int MoveToTownWaypoint(WORD x, WORD y);
	int MoveToTransferDoor(WORD x, WORD y);
	int MoveToStorage(WORD x, WORD y);
	TownMap();
	int Connect();
	//~Map(void);
	void DrawMap();
};

class SimpleMap
{
	int onClosedList;
	int mapWidth;
	int mapHeight;

	int xPath[numberPeople + 1];
	int yPath[numberPeople + 1];

	int *openList; //1 dimensional array holding ID# of open list items
	int *openX; //1d array stores the x location of an item on the open list
	int *openY; //1d array stores the y location of an item on the open list
	int *Fcost;	//1d array to store F cost of a cell on the open list
	int *Hcost;
	int **whichList;//[mapWidth+1][mapHeight+1];  
	int **parentX;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (x)
	int **parentY;//[mapWidth+1][mapHeight+1]; //2d array to store parent of each cell (y)
	int **Gcost;//[mapWidth+1][mapHeight+1]; 	//2d array to store G cost for each cell.


	int pathLength[numberPeople + 1];     //stores length of the found path for critter
	int pathLocation[numberPeople + 1];   //stores current position along the chosen path for critter		
	int* pathBank[numberPeople + 1];
	int pathStatus[numberPeople + 1];

	void InitializePathfinder(void);
	void EndPathfinder(void);
	void ReadPath(int pathfinderID, int currentX, int currentY, int pixelsPerFrame);
	int ReadPathX(int pathfinderID, int pathLocation);
	int ReadPathY(int pathfinderID, int pathLocation);
	int FindPath(int startingX, int startingY, int targetX, int targetY);
	int GetBestPath(std::vector<Point>& Path);
	//////////////////////////////
	bool bConnect;
	ULONG XLine;
	ULONG YRow;
	ULONG HalfX;

	char **walkability;
	std::vector<double> dbTotalShadowSize;
	double dbSingleTotalShadowSize;
//	std::set<Point> ShadowPointList;

	std::set<Point> ShadowPoint;
	bool bGroupModel;
	//////////////
	//boost::shared_ptr<TSocket> socket;
	boost::shared_ptr<TPipe> socket;
	boost::shared_ptr<TTransport> transport;
	boost::shared_ptr<TBinaryProtocol> protocol;
	GameFuncCallClient* pClient;

	void GenShadowPoint();
	void GroupShadowPoint();

	bool AreThereObstacle(int StartX, int StartY, int EndX, int EndY);

	int GetDirectPoint(int& x, int& y);
	int ChangeNotNearWall(int& x, int& y);

public:

	void SetExploredPoint(Point& TargetPos);
	int GetNextExplorePoint(Point& CurPos);
	__inline void AdjustPoint(Point& item);
	void SetPassAbleArea(short x, short y);

	int GetAstarDis(Point& CurPos, Point& TargetPos);

	int ResetMapInfo();//重新载入地图数据
	bool IsObstaclePoint(int x, int y);

	int MoveToPoint(unsigned short x, unsigned y, unsigned short px, unsigned short py);

	SimpleMap();
	int Connect();
	void DrawMap();

};