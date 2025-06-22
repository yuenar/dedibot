#include"ModelLayers.h"
using namespace std;
CircleLayer::CircleLayer()
{
}
CircleLayer::CircleLayer(const Point3D &center,const Point3D &normal,const double radius):c(center),n(normal),r(radius)
{
    n.normalize();//进行单位化
}
CircleLayer::~CircleLayer()
{

}
void CircleLayer::GetNearDir(const Point3D &dirIn,Point3D &dirNear)
{ /*先说下【叉乘】就是通过两个向量来确定一个新的向量,该向量与前两个向量都垂直
简单的矩阵乘法运算：对于两个三维向量：u=(u1,u2,u3)， v=(v1,v2,v3)
叉乘公式：u x v = { u2v3-v2u3 ,u3v1-v3u1 ,u1v2-u2v1 }
点乘公式：u * v = u1v1+u2v2+u3v33
其中点乘dot可以针对任意两个长度相等的向量，而叉乘cross只能是两个三维向量，
例如Vector3d v(1, 2, 3);Vector3d w(0, 1, 2);
那么v.dot(w) 得到的结果是8（等价于v.adjoint() * w），
v.corss(w)得到的结果是(1;-2;1)。*/
	Point3D dirX0=dirIn;
	dirX0.normalize();
	Point3D dirY(0,1,0);
    if(n.dot(dirX0)+eps>1.0)//判定点乘或者叉乘
    {//点乘
		if(n.dot(dirY)+eps>1.0)
			dirY=Point3D(1,0,0);
	}
	else
        dirY=n.cross(dirX0);//叉乘
	dirNear=dirY.cross(n);
	dirNear.normalize();
}

CircleLayers::CircleLayers()
{

}
CircleLayers::~CircleLayers()
{

}
void CircleLayers::Clear()
{
	cls.clear();//清空容器
}
void CircleLayers::InsertLayer(CircleLayer ml)
{
	cls.push_back(ml);//压入容器（尾部）
}
void CircleLayers::PreProcess()
{//预处理
	int size=cls.size(),i,j;
	if(size<1)
        return;//判断容器是否为空
	vector<CircleLayer>tmp;
	if(cls[0].r>eps)
		tmp.push_back(CircleLayer(cls[0].c,cls[0].n,0));
	tmp.push_back(cls[0]);
	for(i=1,j=int(tmp.size())-1;i<size;i++)
	{
		if(cls[i]==tmp[j])
			continue;
		else
		{
			tmp.push_back(cls[i]);
			j++;
		}
	}
	if(cls[size-1].r>eps)
		tmp.push_back(CircleLayer(cls[size-1].c,cls[size-1].n,0));
	size=tmp.size();
	cls.resize(size);
	for(int i=0;i<size;i++)
		cls[i]=tmp[i];
}
bool CircleLayers::ConvertToMesh(Mesh &mesh,int resR)
{//转换成网格
	Point3D dirX0(1,0,0);
	Point3D dirX,dirY,dirZ,c;
	PreProcess();
	int size=cls.size(),base=0,i,j;
	double detB=2*pi/resR,beta,r;
	if(size<2)
		return false;
	for(i=0;i<size;i++)
	{
		cls[i].GetNearDir(dirX0,dirX);
		if(cls[i].r<eps)
		{
			mesh.points.push_back(cls[i].c);
			if(i==0)
				continue;
			if(cls[i-1].r<eps)
				break;
			for(j=1;j<=resR;j++)
				mesh.indices.push_back(Index3D(base-resR+j%resR+1,base+1,base-resR+j));
			base+=1;
		}
		else
		{
			r=cls[i].r;
			c=cls[i].c;
			dirZ=cls[i].n;
			dirY=dirZ.cross(dirX);
			for(j=0;j<resR;j++)
			{
				beta=j*detB;
				mesh.points.push_back(c+r*cos(beta)*dirX+r*sin(beta)*dirY);
			}
			if(i==0)
				continue;
			if(cls[i-1].r<eps)
				for(j=1;j<=resR;j++)
					mesh.indices.push_back(Index3D(base,base+j%resR+1,base+j));
			else
				for(j=1;j<=resR;j++)
				{
					mesh.indices.push_back(Index3D(base-resR+j%resR+1,base+j,base-resR+j));
					mesh.indices.push_back(Index3D(base+j,base+j%resR+1-resR,base+j%resR+1));
				}
			base+=resR;
		}
		dirX0=dirX;
	}
	if(i<size)
		return false;
	return true;
}
CircleLayers CircleLayers::operator+=(const CircleLayers &b)
{
	int i,size=b.cls.size();
	for(i=0;i<size;i++)
		cls.push_back(b.cls[i]);
	return *this;
}

ParallelogramLayer::ParallelogramLayer():c(Point3D(0,0,0)),dirL(Point3D(1,0,0)),dirW(Point3D(0,1,0)),l(10),w(10)
{

}
ParallelogramLayer::ParallelogramLayer(const Point3D &center,const Point3D &dir_len,const Point3D &dir_width,const double length,const double width)
	:c(center),dirL(dir_len),dirW(dir_width),l(length),w(width)
{

}
ParallelogramLayer::~ParallelogramLayer()
{

}
ParallelogramLayers::ParallelogramLayers()
{

}
ParallelogramLayers::~ParallelogramLayers()
{

}
void ParallelogramLayers::Clear()
{
	pls.clear();
}
void ParallelogramLayers::InsertLayer(ParallelogramLayer pl)
{
	pls.push_back(pl);
}
void ParallelogramLayers::PreProcess()
{
	int size=pls.size(),i,j;
	if(size<1)
		return;
	vector<ParallelogramLayer>tmp;
	tmp.push_back(pls[0]);
	for(i=1,j=0;i<size;i++)
	{
		if(pls[i]==tmp[j])
			continue;
		else
		{
			tmp.push_back(pls[i]);
			j++;
		}
	}
	pls=tmp;
}
bool ParallelogramLayers::ConvertToMesh(Mesh& mesh,const int connect)
{
	int i,size=pls.size();
	if(size==0)
		return false;
	for(i=0;i<size;i++)
	{
		mesh.points.push_back(Point3D(pls[i].c-pls[i].dirL*pls[i].l/2-pls[i].dirW*pls[i].w/2));
		mesh.points.push_back(Point3D(pls[i].c+pls[i].dirL*pls[i].l/2-pls[i].dirW*pls[i].w/2));
		mesh.points.push_back(Point3D(pls[i].c+pls[i].dirL*pls[i].l/2+pls[i].dirW*pls[i].w/2));
		mesh.points.push_back(Point3D(pls[i].c-pls[i].dirL*pls[i].l/2+pls[i].dirW*pls[i].w/2));
	}
	for(i=0;i<size;i++)
	{
		if(connect==1&&i==0||connect==0&&i%2==0)
		{
			mesh.indices.push_back(Index3D(4*i,4*i+2,4*i+1));
			mesh.indices.push_back(Index3D(4*i,4*i+3,4*i+2));
		}
		if(connect==1&&i>0||connect==0&&i%2==1)
		{
			for(int j=0;j<4;j++)
			{
				mesh.indices.push_back(Index3D(i*4-4+j,i*4-4+(j+1)%4,i*4+j));
				mesh.indices.push_back(Index3D(i*4-4+(j+1)%4,i*4+(j+1)%4,i*4+j));
			}
		}
		if(connect==1&&i==size-1||connect==0&&i%2==1)
		{
			mesh.indices.push_back(Index3D(4*i,4*i+1,4*i+2));
			mesh.indices.push_back(Index3D(4*i,4*i+2,4*i+3));
		}
	}
	return true;
}
