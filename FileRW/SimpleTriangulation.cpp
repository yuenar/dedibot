#include"SimpleTriangulation.h"
#include<fstream>
using namespace std;
using namespace Eigen;

SimpleTriangulation::SimpleTriangulation()
{

}
SimpleTriangulation::~SimpleTriangulation()
{

}
bool SimpleTriangulation::InTriangle(const int a,const int b,const int c,const int p)
{//判定是否转换成三角数据
	Point2D AC=contour[c]-contour[a];
	Point2D AB=contour[b]-contour[a];
	Point2D AP=contour[p]-contour[a];

	double bb=AC.dot(AC);
	double bc=AC.dot(AB);
	double bp=AC.dot(AP);
	double cc=AB.dot(AB);
	double cp=AB.dot(AP);

	double inverDeno=1/(bb*cc-bc*bc);
	double u=(cc*bp-bc*cp)*inverDeno;
	double v=(bb*cp-bc*bp)*inverDeno;

	if(u<-1e-10||u>1+1e-10||v<-1e-10||v >1+1e-10||(fabs(u+v-1)<1e-10||fabs(u+v)<1e-10)&&fabs(u*v)<1e-10)
		return false;
	return u+v<=1+1e10;
}
void SimpleTriangulation::SetInput(vector<Point2D,Eigen::aligned_allocator<Point2D>>&contourXY)
{//设置输入该位置二维轮廓xy坐标值
	contour.clear();
	int size=contourXY.size();
	for(int i=0;i<size;i++)
		contour.push_back(contourXY[i]);
}
void SimpleTriangulation::SetInput(std::vector<Point3D>&contourXYZ)
{//设置输入三维轮廓位置
	int i,k=0,size=contourXYZ.size();
	for(i=1;i<size;i++)
	{
		if(contourXYZ[i][0]-contourXYZ[k][0]>eps||fabs(contourXYZ[i][0]-contourXYZ[k][0])<eps&&contourXYZ[i][1]-contourXYZ[k][1]>eps
			||fabs(contourXYZ[i][0]-contourXYZ[k][0])<eps&&fabs(contourXYZ[i][1]-contourXYZ[k][1])<eps&&contourXYZ[i][2]-contourXYZ[k][2]>eps)
			k=i;
	}
	Point3D AO=contourXYZ[k]-contourXYZ[(k-1+size)%size];
	Point3D OB=contourXYZ[(k+1)%size]-contourXYZ[k];
	Point3D n=(AO.cross(OB)).normalized(),point;

	contour.resize(size);
	Matrix3d Rx=MatrixXd::Identity(3,3);//绕X轴旋转n与ox所在平面与xoz平面的夹角
	Matrix3d Ry=MatrixXd::Identity(3,3);
	double sinA,cosA,sinB,cosB;
	if(fabs(n[1])<eps)
		sinA=0,cosA=1;
	else
	{
		sinA=n[1]/sqrt(n[1]*n[1]+n[2]*n[2]);
		cosA=n[2]/sqrt(n[1]*n[1]+n[2]*n[2]);
	}
	Rx(1,1)=Rx(2,2)=cosA;
	Rx(1,2)=-sinA;Rx(2,1)=sinA;
	n=Rx*n;
	if(fabs(n[0])<eps)
		sinB=0,cosB=n[2];
	else
	{
		sinB=-n[0]/sqrt(n[0]*n[0]+n[2]*n[2]);
		cosB=n[2]/sqrt(n[0]*n[0]+n[2]*n[2]);
	}
	Ry(0,0)=Ry(2,2)=cosB;
	Ry(0,2)=sinB,Ry(2,0)=-sinB;
	n=Ry*n;
	Matrix3d R=Ry*Rx;
	for(int i=0;i<size;i++)
	{
		point=R*contourXYZ[i];
		contour[i][0]=point[0];
		contour[i][1]=point[1];
	}
}
void SimpleTriangulation::GetOutput(vector<Index3D>&tris)
{//获取输出
	tris.clear();
	vector<int>L;
	vector<int>R;
	vector<int>A;
	vector<int>concave;
	vector<int>convex;
	int k,j,i,size=contour.size();
	L.resize(size);
	R.resize(size);
	A.resize(size);
	for(i=0;i<size;i++)
	{
		L[i]=(i-1+size)%size;
		R[i]=(i+1)%size;
		A[i]=i;
	}
	Point2D AO,OB;
	double a,b;
	while(size>3)
	{
		k=0;
		concave.clear();
		convex.clear();
		/*寻找特征点，一定为凸点*/
		for(i=1;i<size;i++)
		{
			if(contour[A[i]][0]-contour[A[k]][0]>eps||fabs(contour[A[i]][0]-contour[A[k]][0])<eps&&contour[A[i]][1]-contour[A[k]][1]>eps)
				k=i;
		}
		/*凹凸点分类*/
		AO=contour[A[k]]-contour[A[(k-1+size)%size]];
		OB=contour[A[(k+1)%size]]-contour[A[k]];
		a=AO[0]*OB[1]-AO[1]*OB[0];
		for(i=0;i<size;i++)
		{
			AO=contour[A[i]]-contour[A[(i-1+size)%size]];
			OB=contour[A[(i+1)%size]]-contour[A[i]];
			b=AO[0]*OB[1]-AO[1]*OB[0];
			if(a*b<=0)
				concave.push_back(A[i]);
			else
				convex.push_back(A[i]);
		}
		/*切除凸点*/
		int size1=convex.size();
		int size2=concave.size();
		for(i=0;i<size1;i++)
		{
			for(j=0;j<size2;j++)
			{
				if(InTriangle(L[convex[i]],convex[i],R[convex[i]],concave[j]))
					break;
			}
			if(j==size2)
			{
				for(vector<int>::iterator iter=A.begin();iter!=A.end();iter++)
				{
					if(*iter==convex[i])
					{
						A.erase(iter);
						R[L[convex[i]]]=R[convex[i]];
						L[R[convex[i]]]=L[convex[i]];
						tris.push_back(Index3D(L[convex[i]],convex[i],R[convex[i]]));
						size--;
						break;
					}		
				}
				break;
			}
		}
		if(i==size1)
			return;
	}
	tris.push_back(Index3D(A[0],A[1],A[2]));
}

