#ifndef PROJECTDATA_H
#define PROJECTDATA_H
#include<vector>
#include<string>
#include"common.h"

class ModelData;//引用
class ProjectData
{
private:
	ProjectData();
    int curIdx;//当前位置
    std::vector<ModelData*>mdl;//定义存放ModelData指针的容器
    static ProjectData* instance;//创建ProjectData静态实例指针

    void ResetModelColor();//重置模型颜色、重命名模型名、移除未使用的Mesh槽函数
	void RenameModel();
	void RemoveUnuseMesh();

	int langIdx;
	double boxSize[3];
	std::string workPath,dataPath;
public:
    static ProjectData* GetInstance()//获取实例对象
	{
        if(instance==NULL)
            instance=new ProjectData;//若空取当前值
		return instance;
	}
	
	int GetSize(){return mdl.size();}
	void SetSelectModelIndex(int idx){curIdx=idx;}
	int GetSelectModelIndex(){return curIdx;}
	ModelData*  GetSelectModel(){return GetModelData(curIdx);}
	void Clear(int flag=0);
	void Update();

    ModelData* GetModelData(int idx);//获取模型数据
    void InsertModelData(ModelData* md);//插入、移除槽函数
    void RemoveModelData(int idx);//移除模型数据

    bool IsAllVisibleModelHasSupport();//判定可见模型是否有支撑
    bool IsAllVisibleModelPrintable();//判定可见模型是否可打印
    int GetNumofVisibleModel();//获取多个可见模型

	/*View*/
    Point3D GetFocusPoint();//获取光标焦点
	double GetZmax();

	/*Slice*/
	void GetPrintedMesh(Mesh* print);

	/*Setting*/
	void SetBoxSize(double &l,double &w,double &h){boxSize[0]=l,boxSize[1]=w,boxSize[2]=h;}
	const double* GetBoxSize(){return boxSize;}
	void SetLangIndex(const int index){langIdx=index;}
	int GetLangIndex(){return langIdx;}
	void SetWorkPath(const std::string& work){ workPath=work; }
	std::string GetWorkPath(){ return workPath; }
	void SetDataPath(const std::string& data){ dataPath=data; }
	std::string GetDataPath(){ return dataPath; }
};
#endif
