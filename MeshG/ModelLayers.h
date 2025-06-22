#ifndef MODELLAYERS_H
#define MODELLAYERS_H
#include<vector>
#include"common.h"
class  CircleLayer
{//【模型图层】
private:
    Point3D c;//中心坐标
    Point3D n;//法线坐标
    double r;//半径大小

public:
	friend class CircleLayers;

	CircleLayer();
	CircleLayer(const Point3D &center,const Point3D &normal,const double radius);//重载
	~CircleLayer();
	friend bool operator==(const CircleLayer &A,const CircleLayer &B)
	{
		return (A.c-B.c).dot(A.c-B.c)<eps&&(A.n-B.n).dot(A.n-B.n)<eps&&(A.r-B.r)*(A.r-B.r)<eps;
	}
    void GetNearDir(const Point3D &dirIn,Point3D &dirNear);//获取相邻列表 Dir代表是方向
	
};
class CircleLayers
{//【模型图集】
private:
	std::vector<CircleLayer>cls;//创建一个【模型图层】容器来存放数据

    void PreProcess();//预处理槽函数
public:
	CircleLayers();
	~CircleLayers();

    void Clear();//清空
	void InsertLayer(CircleLayer cl);//插入图层
    bool ConvertToMesh(Mesh& mesh,const int resR);//判定是否可转换成网格，resR条经线

	CircleLayers operator+=(const CircleLayers &b);
};

class ParallelogramLayer
{
private:
	Point3D c;
	Point3D dirL,dirW;
	double l,w;

public:
	friend class ParallelogramLayers;

	ParallelogramLayer();
	ParallelogramLayer(const Point3D &center,const Point3D &dir_len,const Point3D &dir_width,const double length,const double width);
	~ParallelogramLayer();

	friend bool operator==(const ParallelogramLayer &A,const ParallelogramLayer &B)
	{
		return (A.c-B.c).dot(A.c-B.c)<eps&&(A.dirL-B.dirL).dot(A.dirL-B.dirL)<eps&&(A.dirW-B.dirW).dot(A.dirW-B.dirW)<eps&&(A.l-B.l)*(A.l-B.l)<eps&&(A.w-B.w)*(A.w-B.w)<eps;
	}
};

class ParallelogramLayers
{
private:
	std::vector<ParallelogramLayer>pls;

	void PreProcess();
public:
	ParallelogramLayers();
	~ParallelogramLayers();

	void Clear();
	void InsertLayer(ParallelogramLayer cl);
	bool ConvertToMesh(Mesh& mesh,const int connect=1);//连接标记

	ParallelogramLayers operator+=(const ParallelogramLayers &b);
};
#endif