#include"TriMeshGenerator.h"
#include"ModelLayers.h"
#include<algorithm>
using namespace std;

CircleVolume::CircleVolume():h(100.0),r(10.0),resH(8),resR(16),c(Point3D(0,0,0)),dirH(Point3D(0,0,1))
{

}
CircleVolume::~CircleVolume()
{

}
void CircleVolume::SetResolutionR(const int resolutionR)
{
	resR=resolutionR;
}
void CircleVolume::SetResolutionH(const int resolutionH)
{
	resH=resolutionH;
}
void CircleVolume::SetCenter(const Point3D &center)
{
	c=center;
}
void CircleVolume::SetHeight(const double height)
{
	h=height;
}
void CircleVolume::SetRadius(const double radius)
{
	r=radius;
}
void CircleVolume::SetDirH(const Point3D &dirHeight)
{
	dirH=dirHeight;
	dirH.normalize();
}
bool CircleVolume::GetTriMesh(Mesh &mesh)
{
	mesh.indices.clear();
	mesh.points.clear();
	CircleLayers cls;
	GetCircleLayers(cls);
	if(cls.ConvertToMesh(mesh,resR))
		return true;
	return false;
}

Ellipsolid::Ellipsolid():as(0),ae(pi)
{
	h=2*r;
}
Ellipsolid::~Ellipsolid()
{

}
Ellipsolid::Ellipsolid(const Point3D &center,const double radius):as(0),ae(pi)
{
	c=center;
	SetSphereRadius(radius);
}
Ellipsolid::Ellipsolid(const Point3D &center,const double radius,const double height):as(0),ae(pi)
{
	c=center;
	r=radius;
	h=height;
}
void Ellipsolid::SetSphereRadius(const double radius)
{
	r=radius;
	h=2*r;
}
void Ellipsolid::SetRange(const double angle_start,const double angle_end)
{
	as=angle_start;
	ae=angle_end;
}
void Ellipsolid::GetCircleLayers(CircleLayers &cls)
{
	if(r<eps||ae+eps<as)
		return;

	cls.Clear();
	Point3D cc;
	double cr,l,cosB,sinB;
	double detB=(ae-as)/resH,beta;
	for(int i=0;i<=resH;i++)
	{
		beta=as+i*detB-pi/2;
		cosB=cos(beta),sinB=sin(beta);
		l=sqrt(1/(cosB*cosB/(r*r)+4*sinB*sinB/(h*h)));
		cr=l*cos(beta);
		cc=c+l*sin(beta)*dirH;
		cls.InsertLayer(CircleLayer(cc,dirH,cr));
	}
}

TruCone::TruCone():rt(5),dirB(dirH),dirT(dirH)
{
}
TruCone::~TruCone()
{

}
TruCone::TruCone(const Point3D &center,const double radius,const double height):rt(radius),dirB(dirH),dirT(dirH)
{
	c=center;
	r=radius;
	h=height;
}
TruCone::TruCone(const Point3D &center,const double radius_bottom,const double radius_top,const double height):rt(radius_top),dirB(dirH),dirT(dirH)
{
	c=center;
	r=radius_bottom;
	h=height;
}
void TruCone::SetCylinerRadius(const double radius)
{
	r=rt=radius;
}
void TruCone::SetTruConeRadius(const double radius_bottom,const double radius_top)
{
	r=radius_bottom;
	rt=radius_top;
}
void TruCone::SetDirBTH(const Point3D &dir)
{
	dirB=dirT=dirH=dir;
}
void TruCone::SetDirBT(const Point3D &dir_bottom,const Point3D &dir_top)
{
	dirB=dir_bottom;
	dirT=dir_top;
}
void TruCone::GetCircleLayers(CircleLayers &cls)
{
	/*判断数据合法性*/
	if(h<eps)
		return;

	cls.Clear();
	Point3D posB=c-h/2*dirH;
	Point3D posT=c+h/2*dirH;

	/*侧面*/
	Point3D cc,up;
	double a[3][3],t,cr;
	for(int i=0;i<3;i++)
	{
		a[i][0]=dirB[i]*h+dirT[i]*h+2*posB[i]-2*posT[i];
		a[i][1]=-2*dirB[i]*h-dirT[i]*h-3*posB[i]+3*posT[i];
		a[i][2]=dirB[i]*h;
	}
	for(int i=0;i<=resH;i++)
	{
		t=1.0*i/resH;
		for(int k=0;k<3;k++)
		{
			cc[k]=posB[k]+a[k][0]*t*t*t+a[k][1]*t*t+a[k][2]*t;
			up[k]=3*a[k][0]*t*t+2*a[k][1]*t+a[k][2];
		}
		cr=(rt-r)*t+r;
		cls.InsertLayer(CircleLayer(cc,up,cr));
	}
}

Pillar::Pillar():r1(5),r2(5),c1(Point3D(10,10,80)),c2(Point3D(-10,-10,-80)),n1(Point3D(1,1,1)),n2(Point3D(-1,-1,-1))
{
	resH=16;
	n1.normalize();
	n2.normalize();
}
Pillar::~Pillar()
{

}
Pillar::Pillar(const Point3D &center,const double radius,const double height)
{
	resH=16;
	c=center;
	r=radius;
	h=height;
	r1=r2=r;
	c1=c+h/2*dirH;
	c2=c-h/2*dirH;
	n1=dirH;
	n2=-dirH;
}
void Pillar::SetSphereCentre(const Point3D &center_1,const Point3D &center_2)
{
	c1=center_1;
	c2=center_2;
}
void Pillar::SetSphereRadius(const double radius_1,const double radius_2)
{
	r1=radius_1;
	r2=radius_2;
}
void Pillar::SetTruConeNormal(const Point3D &normal_1,const Point3D &normal_2)
{
	n1=normal_1;
	n2=normal_2;
	n1.normalize();
	n2.normalize();
}
void Pillar::GetCircleLayers(CircleLayers &cls)
{
	cls.Clear();
	CircleLayers tmp;
	Point3D posT=c+h/2*dirH;
	Point3D posB=c-h/2*dirH;
	Point3D dirH1=c1-posB;
	Point3D dirH2=c2-posT;
	double h1=sqrt(dirH1.dot(dirH1));
	double h2=sqrt(dirH2.dot(dirH2));
	double len=2*r1+2*r2+h1+h2;

	Ellipsolid sphere1(c1,r1);
	sphere1.SetRange(0,pi/2);
	sphere1.SetDirH(-n1);
	sphere1.SetResolutionH(5);
	sphere1.GetCircleLayers(tmp);
	cls+=tmp;

	TruCone trucone1((posB+c1)/2,r1,r,h1);
	trucone1.SetDirBT(-n1,Point3D(0,0,1));
	trucone1.SetDirH(-dirH1);
	trucone1.SetResolutionH(5);
	trucone1.GetCircleLayers(tmp);
	cls+=tmp;

	TruCone cylinder(c,r,h);
	cylinder.SetResolutionH(1);
	cylinder.GetCircleLayers(tmp);
	cls+=tmp;

	TruCone trucone2((posT+c2)/2,r,r2,h2);
	trucone2.SetDirBT(Point3D(0,0,1),n2);
	trucone2.SetDirH(dirH2);
	trucone2.SetResolutionH(5);
	trucone2.GetCircleLayers(tmp);
	cls+=tmp;

	Ellipsolid sphere2(c2,r2);
	sphere2.SetRange(pi/2,pi);
	sphere2.SetDirH(n2);
	sphere2.SetResolutionH(5);
	sphere2.GetCircleLayers(tmp);
	cls+=tmp;
}


ParallelogramVolume::ParallelogramVolume():c(Point3D(0,0,0)),l(10),w(10),h(10),dirL(Point3D(1,0,0)),dirW(Point3D(0,1,0)),dirH(Point3D(0,0,1)),connect(1)
{

}
ParallelogramVolume::~ParallelogramVolume()
{

}
void ParallelogramVolume::SetConnect(int con)
{
	connect=con;
}
void ParallelogramVolume::SetCenter(const Point3D &center)
{
	c=center;
}
void ParallelogramVolume::SetLWH(const double length,const double width,const double height)
{
	l=length;
	w=width;
	h=height;
}
void ParallelogramVolume::SetDirLWH(const Point3D &dir_length,const Point3D &dir_width,const Point3D &dir_height)
{
	dirL=dir_length.normalized();
	dirW=dir_width.normalized();
	dirH=dir_height.normalized();
}
bool ParallelogramVolume::GetTriMesh(Mesh &mesh)
{
	mesh.indices.clear();
	mesh.points.clear();
	ParallelogramLayers pls;
	GetParallelogramLayers(pls);
	if(pls.ConvertToMesh(mesh,connect))
		return true;
	return false;
}

Parallelepiped::Parallelepiped(const Point3D &center,const double length,const double width,const double height)
{
	c=center;
	l=length;
	w=width;
	h=height;
}
Parallelepiped::~Parallelepiped()
{

}
void Parallelepiped::GetParallelogramLayers(ParallelogramLayers &pls)
{
	pls.InsertLayer(ParallelogramLayer(c-h*dirH/2,dirL,dirW,l,w));
	pls.InsertLayer(ParallelogramLayer(c+h*dirH/2,dirL,dirW,l,w));
}

Chassis::Chassis(const Point3D &center,const double length,const double width):thick(1.0),aor(pi/4)
{
	c=center;
	l=length;
	w=width;
	h=2;
}
Chassis::~Chassis()
{

}
void Chassis::SetThick(const double thickness)
{
	thick=thickness;
}
void Chassis::SetAngleofRoll(const double angle)
{
	aor=angle;
}
void Chassis::GetParallelogramLayers(ParallelogramLayers &pls)
{
	double height[4]={0,h,h,thick};
	double add[4]={0,2*h/tan(aor),2*(h-thick)/tan(aor),0};
	for(int i=0;i<4;i++)
	{
		pls.InsertLayer(ParallelogramLayer(c+height[i]*dirH,dirL,dirW,l+add[i],w+add[i]));
	}
	return;
}

Wall::Wall(const Point3D &center,const double length,const double width,const double height):thick(1.0),aor(pi/4)
{
	c=center;
	l=length;
	w=width;
	h=height;
	connect=0;
}
Wall::~Wall()
{

}
void Wall::SetThick(const double thickness)
{
	thick=thickness;
}
void Wall::SetAngleofRoll(const double angle)
{
	aor=angle;
}
void Wall::GetParallelogramLayers(ParallelogramLayers &pls)
{
	int i,n;
	double el[2],ew[2],eh[2],th,hh,alpha;
	Point3D cc,dirEL[2],dirEW[2],dirEH[2];
	th=thick/(cos(aor));
	cc=c-l/2*dirL-h/2*dirH+th/2*dirH;
	hh=l*tan(aor);
	n=ceil(h/(hh+th));
	hh=h/n-th;
	if(hh>eps)
		alpha=atan(hh/l);
	else
	{
		pls.InsertLayer(ParallelogramLayer(c-h*dirH/2,dirL,dirW,l,w));
		pls.InsertLayer(ParallelogramLayer(c+h*dirH/2,dirL,dirW,l,w));
		return;
	}
	eh[0]=eh[1]=l/cos(alpha);
	ew[0]=el[1]=min(h,thick/cos(alpha));
	el[0]=ew[1]=w;
	dirEL[0]=dirW,dirEW[0]=dirH;
	dirEL[1]=dirH,dirEW[1]=dirW;
	dirEH[0]=(dirL*l+dirH*hh).normalized();
	dirEH[1]=(dirH*hh-dirL*l).normalized();
	for(i=0;i<n;i++)
	{
		pls.InsertLayer(ParallelogramLayer(cc,dirEL[i%2],dirEW[i%2],el[i%2],ew[i%2]));
		pls.InsertLayer(ParallelogramLayer(cc+dirEH[i%2]*eh[i%2],dirEL[i%2],dirEW[i%2],el[i%2],ew[i%2]));
		cc=cc+dirEH[i%2]*eh[i%2]+th*dirH;
	}
}

ConvexPolyChassis::ConvexPolyChassis(const std::vector<Point2D,Eigen::aligned_allocator<Point2D>>&pillarPos):thick(2.0),pos(pillarPos)
{

}
ConvexPolyChassis::~ConvexPolyChassis()
{

}
void ConvexPolyChassis::SetThick(const double thickness)
{
	thick=thickness;
}
int ConvexPolyChassis::GetAConvexPoint()
{
	int size=pos.size(),res;
	if(size==0)
		return -1;
	Point2D convex=pos[res=0];
	for(int i=1;i<size;i++)
	{
		if(pos[i][0]<convex[0]||fabs(pos[i][0]-convex[0])<eps&&pos[i][1]<convex[1])
			convex=pos[res=i];
	}
	return res;
} 
void ConvexPolyChassis::CalculateConvexPoly()
{
	convex.clear();
	int idx=GetAConvexPoint();
	if(idx<0)
		return;
	double xld=pos[idx][0]-0,yld=pos[idx][1]-0;
	convex.push_back(Point2D(xld,yld));//中间点
	convex.push_back(Point2D(xld-10,yld));
	convex.push_back(Point2D(xld-5/sin(pi/4),yld-5/sin(pi/4)));
	convex.push_back(Point2D(xld,yld-10));
	int size=pos.size();
	vector<pair<pair<double,double>,int>>ali;//角度、长度、索引
	for(int i=0;i<size;i++)
	{
		Point2D dis=pos[i]-convex[1];
		double len=sqrt(dis[0]*dis[0]+dis[1]*dis[1]);
		double alpha=acos(-dis[1]/len);
		ali.push_back(make_pair(make_pair(alpha,len),i));
	}
	sort(ali.begin(),ali.end());
	for(int i=0;i<size-1;)
	{
		if(ali[i].first.first==ali[i+1].first.first)
			ali.erase(ali.begin()+i),size--;
		else
			i++;
	}
	size=ali.size();
	int i=0,k=4;
	while(i<size)
	{
		Point2D AB=convex[k-1]-convex[k-2];
		Point2D AC=pos[ali[i].second]-convex[k-2];
		double flag=AB[0]*AC[1]-AB[1]*AC[0];
		if(flag>0||k==4)
		{
			convex.push_back(pos[ali[i].second]);
			k++,i++;
		}
		else
		{
			convex.pop_back();
			k--;
		}
	}
}
bool ConvexPolyChassis::GetTriMesh(Mesh& mesh)
{
	mesh.indices.clear();
	mesh.points.clear();
	CalculateConvexPoly();
	int size=convex.size();
	if(size==0)
		return false;
	for(int i=0;i<size;i++)
		mesh.points.push_back(Point3D(convex[i][0],convex[i][1],0));
	for(int i=0;i<size;i++)
		mesh.points.push_back(Point3D(convex[i][0],convex[i][1],thick));
	mesh.points[2][2]+=3;
	mesh.points[2+size][2]+=3;
	for(int i=1;i<size;i++)
	{
		mesh.indices.push_back(Index3D(i,0,i%(size-1)+1));
		mesh.indices.push_back(Index3D(i+size,i%(size-1)+1+size,size));
	}
	for(int i=1;i<size;i++)
	{
		mesh.indices.push_back(Index3D(i,i%(size-1)+1,i%(size-1)+1+size));
		mesh.indices.push_back(Index3D(i,i%(size-1)+1+size,i+size));
	}
	return true;
}
