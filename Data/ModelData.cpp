#include"ModelData.h"
#include"MeshList.h"
#include"ProjectData.h"
#include"SupportG/SupportGenerator.h"
#include"MeshG/TriMeshGenerator.h"
#include"MeshP/MeshProcess.h"
#include"FileRW/FileReaderWriter.h"
#include<algorithm>
using namespace std;
ModelData::ModelData():modelId(-1),supportId(-1),modelVisible(true),supportVisible(true)
{//载入预设参数
    defaultScale=Point3D(1.0,1.0,1.0);//默认缩放值
    defaultMove=Point3D(0.0,0.0,0.0);//默认位移值
	scale=Point3D(1.0,1.0,1.0);
	move=Point3D(0.0,0.0,0.0);
	rotate=Point3D(0.0,0.0,0.0);
	modelColor=Color::blue;
	supportColor=Color::blue;
	supportParam[SupportParam::SupportThickness]=0.7;
	supportParam[SupportParam::ContactPointSize]=0.35;
	supportParam[SupportParam::PillarSpacing]=7.5;
	supportParam[SupportParam::ChassisThickness]=2.0;
	supportParam[SupportParam::PillarRadius]=0.7;
	supportParam[SupportParam::PillarSpacingE]=7.5;
	supportParam[SupportParam::HoistingHeight]=10.0;
}
ModelData::~ModelData()
{

}
const Mesh* ModelData::GetModelMesh()const
{
	return MeshList::GetInstance()->GetMesh(modelId);
}
const Mesh* ModelData::GetSupportMesh()const
{
	return MeshList::GetInstance()->GetMesh(supportId);
}
void ModelData::SetModelId(const int mId)
{
	modelId=mId;//设置中心点，边界点，此处应为包围盒的设定
	Point3D center,pmax,pmin;
	const Mesh* mesh=MeshList::GetInstance()->GetMesh(mId);
	int size=mesh->points.size();
	pmax=pmin=mesh->points[0];
	for(int i=0;i<size;i++)
	{
		for(int j=0;j<3;j++)
        {//取最大最小值
			pmax[j]=max(pmax[j],mesh->points[i][j]);
			pmin[j]=min(pmin[j],mesh->points[i][j]);
		}
	}
    Point3D lenXYZ=pmax-pmin;//包围盒的边长
	center=(pmax+pmin)/2;
	defaultMove=-center;
    double lmax=max(lenXYZ[2],max(lenXYZ[0],lenXYZ[1]));//调用最大最小值
	const double* boxSize=ProjectData::GetInstance()->GetBoxSize();
	defaultScale[0]=defaultScale[1]=defaultScale[2]=boxSize[0]/2/lmax;
    scale[0]=scale[1]=scale[2]=1/defaultScale[0];//更新【缩放】对应值
    UpdateBoundingBox();//更新包围盒边界
}
int ModelData::SplitModelName(string &name)const
{//分拆模型名槽函数 +?-
	int len=modelName.length();
    if(modelName[len-1]!=')')//若模型名最后一字符不是‘)'
	{
		name=modelName;
		return 0;
    }/*以下开始前驱条件，模型名最后一字符是‘)'*/
	int i=len-2;
    while(i>=0&&modelName[i]!='(')//若模型名倒数第二字符不是‘（'
	{
        if(modelName[i]<'0'||modelName[i]>'9')//ASCII值小于48（’0‘）或大于57（’9‘）
            break;//若modelName[i]对应值在0-9，则跳出
        i--;//同时自减
	}
    if(i==0||modelName[i]!='('||len-i>5)//注意载入模型重命名xxx(int).stl
	{
		name=modelName;
		return 0;
	}
	int res=0;
	for(int j=i+1;j<len-1;j++)
	{
		res=res*10+modelName[j]-'0';
	}
	char str[128];
	for(int j=0;j<i;j++)
		str[j]=modelName[j];
	str[i]='\0';
	name=string(str);
	return res;
}
void ModelData::UpdateBoundingBox()
{//更新包围盒边界
    auto M=GetTransformMatrix();//自动类型
	boundingBox[0]=boundingBox[1]=boundingBox[2]=1e100;
	boundingBox[3]=boundingBox[4]=boundingBox[5]=-1e100;
	for(int i=0;i<2;i++)
	{
		const Mesh* mesh;
        if(i==0&&modelId>=0)//已有模型
            mesh=MeshList::GetInstance()->GetMesh(modelId);//获取ID为modelId模型
        else if(i==1&&supportId>=0)//前驱条件没有模型，已有支撑
            mesh=MeshList::GetInstance()->GetMesh(supportId);//获取ID为supportId支撑
		else
			continue;
		int psize=mesh->points.size();
		for(int i=0;i<psize;i++)
		{
			Point3D p(mesh->points[i]);
			Eigen::Vector4d point(p[0],p[1],p[2],1);
			point=M*point;
			for(int j=0;j<3;j++)
			{
				boundingBox[j]=min(boundingBox[j],point[j]);
				boundingBox[j+3]=max(boundingBox[j+3],point[j]);
			}
		}
	}
	move[AxisZ]-=boundingBox[2];
	boundingBox[5]-=boundingBox[2];
	boundingBox[2]=0;
	for(int i=0;i<3;i++)
		geometrySize[i]=boundingBox[3+i]-boundingBox[i];
}
bool ModelData::IsGeometryValide()
{//包围盒尺寸在此限定100*100*100，判定取值是否合法
	const double* boxSize=ProjectData::GetInstance()->GetBoxSize();
	if(boundingBox[0]<-boxSize[0]/2||boundingBox[1]<-boxSize[1]/2||boundingBox[3]>boxSize[0]/2||boundingBox[4]>boxSize[1]/2||boundingBox[5]>boxSize[2])
		return false;
	return true;
}
void ModelData::UpdateColor(bool isSelected)
{//更新选中颜色
	if(isSelected)
	{
		if(IsGeometryValide())
			modelColor=Color::blue,supportColor=Color::blue;
		else
			modelColor=Color::red,supportColor=Color::red;
	}
	else
	{
		if(IsGeometryValide())
			modelColor=Color::grey,supportColor=Color::grey;
		else
			modelColor=Color::purple,supportColor=Color::purple;
	}
}
Eigen::Matrix4d ModelData::GetTransformMatrix()const
{//获得变换矩阵
            /*t平移，r旋转，s缩放，m代表矩阵，gmm初始的平移矩阵，gms初始的缩放矩阵
             *   赋值给左边<<  */
	Eigen::Matrix4d ret,mt,mrx,mry,mrz,ms,gms,gmm;
	gmm<<1,0,0,0,0,1,0,0,0,0,1,0,defaultMove[0],defaultMove[1],defaultMove[2],1;
	gms<<defaultScale[0],0,0,0,0,defaultScale[1],0,0,0,0,defaultScale[2],0,0,0,0,1;
	mt<<1,0,0,0,0,1,0,0,0,0,1,0,move[0],move[1],move[2],1;
	double cosA=cos(rotate[0]/180.0*pi),sinA=sin(rotate[0]/180.0*pi);
	double cosB=cos(rotate[1]/180.0*pi),sinB=sin(rotate[1]/180.0*pi);
	double cosC=cos(rotate[2]/180.0*pi),sinC=sin(rotate[2]/180.0*pi);
	mrx<<1,0,0,0,0,cosA,sinA,0,0,-sinA,cosA,0,0,0,0,1;
	mry<<cosB,0,-sinB,0,0,1,0,0,sinB,0,cosB,0,0,0,0,1;
	mrz<<cosC,sinC,0,0,-sinC,cosC,0,0,0,0,1,0,0,0,0,1;
	ms<<scale[0],0,0,0,0,scale[1],0,0,0,0,scale[2],0,0,0,0,1;
	ret=gmm*gms*ms*mrx*mry*mrz*mt;
    return ret.transpose();//矩阵转置
}
void ModelData::RemoveSupport()
{//移除支撑
    supportId=-1;//强置为-1，暴力移除，此处应改为清空 ’移除‘应对单一支撑操作
	supportVisible=true;
    UpdateBoundingBox();//更新包围盒边界
}
void ModelData::SetSupportFromSC(Supporter_Classic* sc)
{//生成支撑
    RemoveSupport();//先移除支撑
	Mesh mesh,pillars,cuboids;
	vector<Point2D,Eigen::aligned_allocator<Point2D>>pillarPos;
	for(int i=0; i < sc->pillars.size(); i++)
	{
		PillarPara pp=sc->pillars[i];
		Pillar pillar(pp.c_cyl,pp.r_cyl,pp.h);
		pillarPos.push_back(Point2D(pp.c_cyl[0],pp.c_cyl[1]));
		pillar.SetSphereCentre(pp.c_ball_bottom,pp.c_ball_top);
		pillar.SetSphereRadius(pp.r_ball_bottom,pp.r_ball_top);
		pillar.SetTruConeNormal(pp.d_tc_bottom,pp.d_tc_top);
		pillar.GetTriMesh(mesh);
		MeshProcess::MeshUnion(&pillars,&mesh);
	}
	for(int i=0; i < sc->walls.size(); i++)
	{
		WallPara wall_para=sc->walls[i];
		Point3D center=(wall_para.p_tl+wall_para.p_tr+wall_para.p_br+wall_para.p_bl)/4;
		Point3D dirAB=wall_para.p_tr-wall_para.p_tl;
		Point3D dirAC=wall_para.p_bl-wall_para.p_tl;
		Wall wall(center,dirAB.norm(),wall_para.th,dirAC.norm());
		wall.SetDirLWH(dirAB,Point3D(0,0,1).cross(dirAB),Point3D(0,0,1));
		wall.GetTriMesh(mesh);
		MeshProcess::MeshUnion(&cuboids,&mesh);
	}
	MeshProcess::MeshUnion(&cuboids,&pillars);
	double thick=supportParam[SupportParam::ChassisThickness];
	if(thick>eps)
	{
        //double cx=(boundingBox[0]+boundingBox[3])/2;
        //double cy=(boundingBox[1]+boundingBox[4])/2;
        //double l=geometrySize[0]+supportParam[SupportParam::PillarSpacing];
        //double w=geometrySize[1]+supportParam[SupportParam::PillarSpacing];
		ConvexPolyChassis chassis(pillarPos);
		chassis.SetThick(supportParam[SupportParam::ChassisThickness]);
		chassis.GetTriMesh(mesh);
		//FileReaderWriter frw;
		//frw.SaveModel("E:/chassis.stl",&mesh);
		MeshProcess::MeshUnion(&cuboids,&mesh);
	}
	Mesh* support=new Mesh;
	move[Axis::AxisZ]+=supportParam[SupportParam::HoistingHeight];
	auto M=GetTransformMatrix();
	MeshProcess::MeshTransform(&cuboids,M.inverse(),support);
	MeshProcess::UpdateNormals(support);

	/*Mesh *test=new Mesh;
	MeshProcess::MeshUnion(support,input);
	MeshProcess::MeshTransform(support,M,test);
	FileReaderWriter::SaveModel("test1.stl",test);*/
	supportId=MeshList::GetInstance()->InsertMesh(support);
	UpdateBoundingBox();
}
