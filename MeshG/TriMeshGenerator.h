#ifndef TRIMESHGENERATOR_H
#define TRIMESHGENERATOR_H
#include"common.h"
class CircleLayers;
class ParallelogramLayers;
class Pillar;
class CircleVolume
{//圆形多侧面锥体
protected:
	double h,r;
	int resR,resH;//R方向与H方向精度
	Point3D c,dirH;

	virtual void GetCircleLayers(CircleLayers &cls)=0;

public:
	CircleVolume();
	virtual~CircleVolume();
    void SetResolutionR(const int resolutionR);//设置R方向解析度
    void SetResolutionH(const int resolutionH);//设置H方向解析度
    void SetHeight(const double height);//设置高度
    void SetRadius(const double radius);//设置半径
    void SetCenter(const Point3D &center);//设置中心
    void SetDirH(const Point3D &center);//设置

    bool GetTriMesh(Mesh &mesh);//获取三角网格
};

class Ellipsolid:public CircleVolume
{
private:
	double as,ae;

protected:
	friend class Pillar;
	void GetCircleLayers(CircleLayers &cls);

public:
	Ellipsolid();
	~Ellipsolid();
    Ellipsolid(const Point3D &center,const double radius);
    Ellipsolid(const Point3D &center,const double radius,const double height);
    //重载

	void SetSphereRadius(const double radius);
	void SetRange(const double angle_start,const double angle_end);
};

class TruCone:public CircleVolume
{
private:
	double rt;
	Point3D dirB,dirT;

protected:
	friend class Pillar;
	void GetCircleLayers(CircleLayers &cls);

public:
	TruCone();
	~TruCone();
	TruCone(const Point3D &center,const double radius,const double height);
	TruCone(const Point3D &center,const double radius_bottom,const double radius_top,const double height);

	void SetCylinerRadius(const double radius);
	void SetTruConeRadius(const double radius_bottom,const double radius_top);
	void SetDirBTH(const Point3D &dir);
	void SetDirBT(const Point3D &dir_bottom,const Point3D &dir_top);
};

class Pillar:public CircleVolume
{
private:
	double r1,r2;//球半径1，半径2
	Point3D c1,c2;//球心1、2
	Point3D n1,n2;//法向1、2

protected:
	void GetCircleLayers(CircleLayers &cls);

public:
	Pillar();
	~Pillar();
	Pillar(const Point3D &center,const double radius,const double height);

	void SetSphereCentre(const Point3D &center_1,const Point3D &center_2);//设置球心
    void SetSphereRadius(const double radius_1,const double radius_2);//设置球半径
    void SetTruConeNormal(const Point3D &normal_1,const Point3D &normal_2);//设置角锥法线
};


class ParallelogramVolume
{//【长方体】
protected:
	Point3D c;
	double l,w,h;
	Point3D dirL,dirW,dirH;

	int connect;
	virtual void GetParallelogramLayers(ParallelogramLayers &pls)=0;

public:
	ParallelogramVolume();
	virtual ~ParallelogramVolume();

	void SetConnect(int con);
    void SetCenter(const Point3D &center);//设置中心
    void SetLWH(const double length,const double width,const double height);//设置长宽高
	void SetDirLWH(const Point3D &dir_length,const Point3D &dir_width,const Point3D &dir_height);

    bool GetTriMesh(Mesh& mesh);//判定是否获取三角网格数据
};
class Parallelepiped:public ParallelogramVolume
{
protected:
	void GetParallelogramLayers(ParallelogramLayers &pls);

public:
	Parallelepiped(const Point3D &center=Point3D(0,0,0),const double length=50,const double width=50,const double height=50);
	~Parallelepiped();
};

class Chassis:public ParallelogramVolume
{
private:
	double thick,aor;

protected:
	void GetParallelogramLayers(ParallelogramLayers &pls);

public:
	Chassis(const Point3D &center=Point3D(0,0,0),const double length=50,const double width=50);
	~Chassis();

	void SetThick(const double thickness);
	void SetAngleofRoll(const double angle);
};

class Wall:public ParallelogramVolume
{
private:
	double thick,aor;

protected:
	void GetParallelogramLayers(ParallelogramLayers &pls);

public:
	Wall(const Point3D &center=Point3D(0,0,0),const double length=50,const double width=50,const double height=50);
	~Wall();

	void SetThick(const double thickness);
	void SetAngleofRoll(const double angle);
};

class ConvexPolyChassis
{
private:
	double thick;
	std::vector<Point2D,Eigen::aligned_allocator<Point2D>>pos;
	std::vector<Point2D,Eigen::aligned_allocator<Point2D>>convex;
	int GetAConvexPoint();//左下角
	void CalculateConvexPoly();//求凸包
public:
	ConvexPolyChassis(const std::vector<Point2D,Eigen::aligned_allocator<Point2D>>&pillarPos);
	~ConvexPolyChassis();
	void SetThick(const double thickness);
	bool GetTriMesh(Mesh &mesh);
};
#endif//TRIMESHGENERATOR_H