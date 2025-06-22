#include"ProjectData.h"
#include"ModelData.h"
#include"MeshList.h"
#include<string>
#include<map>
#include<fstream>
#include"MeshP/MeshProcess.h"
using namespace std;
ProjectData* ProjectData::instance=NULL;//初始为空指针
ProjectData::ProjectData():curIdx(0),langIdx(0),dataPath(""),workPath("")
{
	boxSize[0]=boxSize[1]=boxSize[2]=100;
	//sliceParam[SliceParam::Thickness]=0.045;
}
void ProjectData::Clear(int flag)
{//清空作业数据中所有实例
	int size=mdl.size();
	for(int i=0;i<size;i++)
	{
		ModelData* md=mdl[i];
		delete md;
	}
	mdl.clear();
	if(flag)
	{
		delete instance;
    instance=NULL;//置为空指针
	}
}
ModelData* ProjectData::GetModelData(int idx)
{//获取模型数据
	int size=mdl.size();
    if(idx>=size)//条件取反则遍历
		return NULL;
	return mdl[idx];
}
void ProjectData::InsertModelData(ModelData* md)
{
    mdl.push_back(md);//插入模型数据
}
void ProjectData::RemoveModelData(int idx)
{//移除模型数据
    int size=mdl.size();//获取容器大小
    if(idx>=size)//条件取反则遍历
		return;
    delete mdl[idx];//删除
	mdl.erase(mdl.begin()+idx);
	if(curIdx>=idx&&curIdx)
		curIdx--;
}
void ProjectData::ResetModelColor()
{//重置模型颜色
	int size=mdl.size();
	for(int i=0;i<size;i++)
		mdl[i]->UpdateColor(false);
	if(curIdx>=0&&curIdx<size)
		mdl[curIdx]->UpdateColor(true);
}
void ProjectData::RenameModel()
{//重命名模型
	int size=mdl.size();
	string name,newName;
    map<string,int>M;//新建一个图，存放键值对
	vector<int>conflict;
	for(int i=0;i<size;i++)
	{
		name=mdl[i]->GetModelName();
        if(M.find(name)==M.end())//查找是否到尾部
			M[name]=1;
		else
            conflict.push_back(i);//若没有继续填充数据
	}
	size=conflict.size();
	char str[128];
	for(int i=0;i<size;i++)
	{
		int id=mdl[conflict[i]]->SplitModelName(name);
		do
		{
			id++;
			sprintf(str,"%s(%d)",name.c_str(),id);
			newName=string(str);
        } while(M.find(newName)!=M.end());//查找是否到尾部，并作为循环条件
		M[newName]=1;
		mdl[conflict[i]]->SetModelName(newName);
	}
}
void ProjectData::RemoveUnuseMesh()
{//移除无用的多边形网格
	int size=mdl.size(),id;
	int n=MeshList::GetInstance()->GetSize();
	vector<int>flag(n,0);
	for(int i=0;i<size;i++)
	{
		id=mdl[i]->GetModelId();
		if(id>=0&&id<n)flag[id]++;
		id=mdl[i]->GetSupportId();
		if(id>=0&&id<n)flag[id]++;
	}
	for(int i=0;i<n;i++)
	{
		if(MeshList::GetInstance()->GetMesh(i)!=NULL&&flag[i]==0)
			MeshList::GetInstance()->Remove(i);
	}
}
bool ProjectData::IsAllVisibleModelHasSupport()
{//判定可见所有模型是否已有支撑
	int size=mdl.size();
	for(int i=0;i<size;i++)
	{
		if(mdl[i]->GetModelId()>=0&&mdl[i]->IsModelVisible())
		{
			if(mdl[i]->GetSupportId()<0||mdl[i]->IsSupportVisible()==false)
				return false;
		}
	}
    return true;//默认返回真
}
bool ProjectData::IsAllVisibleModelPrintable()
{//判定是否所有模型是否可被打印
	int size=mdl.size();
	for(int i=0;i<size;i++)
	{
		if(mdl[i]->GetModelId()>=0&&mdl[i]->IsModelVisible()||mdl[i]->GetSupportId()>=0&&mdl[i]->IsSupportVisible())
		{
			if(mdl[i]->IsGeometryValide()==false)
				return false;
		}
	}
    return true;//默认为真
}
int ProjectData::GetNumofVisibleModel()
{//获取模型数值
	int size=mdl.size(),ret=0;
	for(int i=0;i<size;i++)
	{
		if(mdl[i]->GetModelId()>=0&&mdl[i]->IsModelVisible()||mdl[i]->GetSupportId()>=0&&mdl[i]->IsSupportVisible())
			ret++;
	}
	return ret;
}
void ProjectData::GetPrintedMesh(Mesh* print)
{//获取已打印的多边形网格
	Mesh tmp;
	int size=mdl.size();
	print->points.clear();
	print->normals.clear();
    print->indices.clear();//先清空缓存
	for(int i=0;i<size;i++)
	{
		if(mdl[i]->GetModelId()>=0&&mdl[i]->IsModelVisible())
		{
			MeshProcess::MeshTransform(mdl[i]->GetModelMesh(),mdl[i]->GetTransformMatrix(),&tmp);
			MeshProcess::MeshUnion(print,&tmp);
		}	
		if(mdl[i]->GetSupportId()>=0&&mdl[i]->IsSupportVisible())
		{
			MeshProcess::MeshTransform(mdl[i]->GetSupportMesh(),mdl[i]->GetTransformMatrix(),&tmp);
			MeshProcess::MeshUnion(print,&tmp);
		}
	}
}

Point3D ProjectData::GetFocusPoint()
{//获取光标焦点
	Point3D ret(0,0,boxSize[2]/2);
	ModelData* md=GetSelectModel();
	if(md)
	{
        const double* bb=md->GetBoundingBox();//获取包围盒边界坐标以及几何长度
		for(int i=0;i<3;i++)
			ret[i]=(bb[i]+bb[i+3])/2;
	}
	return ret;
}
double ProjectData::GetZmax()
{
	int size=mdl.size();
	double zmax=0;
	const double* bb;
	ModelData* md;
	for(int i=0;i<size;i++)
	{
		md=GetModelData(i);
		bb=md->GetBoundingBox();
		zmax=max(bb[5],zmax);
	}
	return zmax;
}
void ProjectData::Update()
{//刷新数据
	ResetModelColor();
	RenameModel();
	RemoveUnuseMesh();
}
