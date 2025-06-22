#ifndef MODELDATA_H
#define MODELDATA_H
#include"common.h"
#include<string>
#include<vector>
struct Supporter_Classic;
namespace Color//创建一个命名空间Color
{
	const static Point3D red=Point3D(203,34,32)/255;
	const static Point3D purple=Point3D(158,73,151)/256;
	const static Point3D blue=Point3D(72,133,174)/255;
	const static Point3D grey=Point3D(160,159,159)/256;
}
enum Axis//自定义两个枚举Axis、SupportParam，后面会用到x\y\z轴
{
	AxisX,AxisY,AxisZ
};
enum SupportParam
{
	SupportThickness,ContactPointSize,PillarSpacing,ChassisThickness,PillarRadius,PillarSpacingE,HoistingHeight,SupportParamEnd
};
class ModelData
{
private:
    std::string modelName;//私有字符串成员模型名
    Point3D defaultScale,defaultMove;//默认缩放值，位移值
    Point3D scale,move,rotate;//缩放、移动、旋转
	
    int modelId,supportId;//分别给模型和支撑一个ID
	bool modelVisible,supportVisible;
	Point3D modelColor,supportColor;
    double boundingBox[6];//数组存放xmin,ymin,zmin,xmax,ymax,zmax包围盒边界坐标
    double geometrySize[3];//数组存放lenX,lenY,lenZ几何长度

    double supportParam[SupportParam::SupportParamEnd];//数组存放支撑相关参数
    void UpdateBoundingBox();//更新包围盒边界槽函数
public:

	ModelData();
	~ModelData();

    void SetModelId(const int mId);//设置模型、支撑ID的两个槽函数
	void SetSupportId(const int sid){ supportId=sid; }
	
    const Mesh* GetModelMesh()const;//因为两ID是const指针类型，表示无权修改
	const Mesh* GetSupportMesh()const;

	void SetModelName(const std::string s){ if(s!="")modelName=s; }
    std::string GetModelName()const{ return modelName; }//获取模型名槽函数
    int SplitModelName(std::string &name)const;//分拆模型名槽函数

    int GetModelId()const	{ return modelId; }//获取模型、支撑ID槽函数
	int GetSupportId()const	{ return supportId; }
    bool IsModelVisible()const	{ return modelVisible; }//判定模型、支撑可见性
	bool IsSupportVisible()const	{ return supportVisible; }
    void SetModelVisible(bool visible)	{ modelVisible=visible; }//设置模型、支撑可见性
	void SetSupportVisible(bool visible)	{ supportVisible=visible; }
    Point3D GetModelColor()const	{ return modelColor; }//获取模型、支撑颜色槽函数
	Point3D GetSupportColor()const	{ return supportColor; }
    const double* GetBoundingBox()const { return boundingBox; }//获取包围盒边界坐标以及几何长度
	const double* GetGeometrySize()const	{ return geometrySize; }
	double GetGeometrySize(const Axis a)const { return geometrySize[a]; }

    bool IsGeometryValide();//判定是否几何越界
    void UpdateColor(bool isSelected=false);//更新颜色
	Eigen::Matrix4d GetTransformMatrix()const;

    //预设缩放值
	/*Default Scale*/
	void SetDefaultScale(const double s){defaultScale=Point3D(s,s,s);UpdateBoundingBox();}
	double GetDefaultScale()const{ return defaultScale[0];}

    //预设位移值
	/*Move*/
	Point3D GetMoveValue()const{return move;}
	double GetMoveValue(const Axis a)const{return move[a];}
	void SetMoveValue(const Point3D& moveXYZ){move=moveXYZ;UpdateBoundingBox();}
	void SetMoveValue(const Axis a,const double value){move[a]=value;UpdateBoundingBox();}

    //预设旋转角度
	/*Rotate*/
	Point3D GetRotateValue()const{return rotate;}
	double GetRotateValue(const Axis a)const{return rotate[a];}
	void SetRotateValue(const Point3D& rotateXYZ){rotate=rotateXYZ;UpdateBoundingBox();}
	void SetRotateValue(const Axis a,const double value){rotate[a]=value;UpdateBoundingBox();}

	/*Scale*/
	double GetScaleValue()const{return scale[0];}
	void SetScaleValue(const double s)
	{
		if(s>eps)
		{
            scale[0]=scale[1]=scale[2]=s;//设置初值
			UpdateBoundingBox();
		}
	}

	/*Support*/
	const double* GetSupportParam()const{return supportParam;}
	double GetSupportParam(const SupportParam a)const{return supportParam[a];}
	void SetSupportParam(const SupportParam a,const double value){supportParam[a]=value;}
	void SetSupportParam(const double* sp){for(int i=0;i<SupportParam::SupportParamEnd;i++)supportParam[i]=sp[i];}
	void SetSupportFromSC(Supporter_Classic* sc);
    void RemoveSupport();//移除支撑的槽函数
};
#endif
