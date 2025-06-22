#include"MeshProcess.h"
#include<vector>
using namespace std;
//void MeshBoolean::MeshToCTM(Mesh &mesh,CorkTriMesh &ctm)
//{
//	int i;
//	ctm.n_triangles=mesh->indices.size();
//	ctm.n_vertices=mesh->points.size();
//	ctm.triangles=new uint[3*ctm.n_triangles];
//	ctm.vertices=new float[3*ctm.n_vertices];
//	for(i=0;i<ctm.n_vertices;i++)
//	{
//		ctm.vertices[3*i+0]=(float)mesh->points[i][0];
//		ctm.vertices[3*i+1]=(float)mesh->points[i][1];
//		ctm.vertices[3*i+2]=(float)mesh->points[i][2];
//	}
//	for(i=0;i<ctm.n_triangles;i++)
//	{
//		ctm.triangles[3*i+0]=(uint)mesh->indices[i][0];
//		ctm.triangles[3*i+1]=(uint)mesh->indices[i][1];
//		ctm.triangles[3*i+2]=(uint)mesh->indices[i][2];
//	}
//}
//void MeshBoolean::CTMToMesh(CorkTriMesh& ctm,Mesh& mesh)
//{
//	int i;
//	mesh->points.resize(ctm.n_vertices);
//	mesh->indices.resize(ctm.n_triangles);
//	for(i=0;i<ctm.n_vertices;i++)
//	{
//		mesh->points[i][0]=ctm.vertices[3*i+0];
//		mesh->points[i][1]=ctm.vertices[3*i+1];
//		mesh->points[i][2]=ctm.vertices[3*i+2];
//	}
//	for(i=0;i<ctm.n_triangles;i++)
//	{
//		mesh->indices[i][0]=ctm.triangles[3*i+0];
//		mesh->indices[i][1]=ctm.triangles[3*i+1];
//		mesh->indices[i][2]=ctm.triangles[3*i+2];
//	}
//}
void MeshProcess::MeshUnion(Mesh *m1,const Mesh *m2)
{
	int i,j;
	int base=m1->points.size();
	for(i=0; i < m2->points.size(); i++)
	{
		m1->points.push_back(m2->points[i]);
	}
	for(j=0; j < m2->indices.size(); j++)
	{
		m1->indices.push_back(m2->indices[j]+Index3D(base,base,base));
	}
}
//void MeshBoolean::MeshAdd(Mesh& add1,Mesh& add2,Mesh& res)
//{
//	CorkTriMesh in1,in2,out;
//	MeshToCTM(add1,in1);
//	MeshToCTM(add2,in2);
//	computeUnion(in1,in2,&out);
//	CTMToMesh(out,res);
//	freeCorkTriMesh(&in1);
//	freeCorkTriMesh(&in2);
//	freeCorkTriMesh(&out);
//}
//void MeshBoolean::MeshSub(Mesh& sub1,Mesh& sub2,Mesh& res)//res=sub1-sub2
//{
//	CorkTriMesh in1,in2,out;
//	MeshToCTM(sub1,in1);
//	MeshToCTM(sub2,in2);
//	computeDifference(in1,in2,&out);
//	CTMToMesh(out,res);
//	freeCorkTriMesh(&in1);
//	freeCorkTriMesh(&in2);
//	freeCorkTriMesh(&out);
//}
//void MeshBoolean::MeshInt(Mesh& int1,Mesh& int2,Mesh& res)//res=int1*int2
//{
//	CorkTriMesh in1,in2,out;
//	MeshToCTM(int1,in1);
//	MeshToCTM(int2,in2);
//	computeIntersection(in1,in2,&out);
//	CTMToMesh(out,res);
//	freeCorkTriMesh(&in1);
//	freeCorkTriMesh(&in2);
//	freeCorkTriMesh(&out);
//}
//void MeshBoolean::MeshXor(Mesh& xor1,Mesh& xor2,Mesh& res)
//{
//	CorkTriMesh in1,in2,out;
//	MeshToCTM(xor1,in1);
//	MeshToCTM(xor2,in2);
//	computeSymmetricDifference(in1,in2,&out);
//	CTMToMesh(out,res);
//	freeCorkTriMesh(&in1);
//	freeCorkTriMesh(&in2);
//	freeCorkTriMesh(&out);
//}
void MeshProcess::UpdateNormals(Mesh* mesh)
{
	vector<int>cnt;
	int i;
	int psize=mesh->points.size();
	int fsize=mesh->indices.size();
	cnt.resize(psize);
	mesh->normals.resize(psize);
	for(i=0;i<psize;i++)
	{
		cnt[i]=0;
		mesh->normals[i]=Point3D(0,0,0);
	}
	int a,b,c;
	Point3D AB,AC,normal;
	for(i=0;i<fsize;i++)
	{
		a=mesh->indices[i][0],b=mesh->indices[i][1],c=mesh->indices[i][2];
		AB=mesh->points[b]-mesh->points[a];
		AC=mesh->points[c]-mesh->points[a];
		normal=AB.cross(AC).normalized();
		mesh->normals[a]+=normal,cnt[a]++;
		mesh->normals[b]+=normal,cnt[b]++;
		mesh->normals[c]+=normal,cnt[c]++;
	}
	for(i=0;i<psize;i++)
	{
		mesh->normals[i]/=cnt[i];
		mesh->normals[i].normalize();
	}
}
void MeshProcess::MeshTransform(const Mesh* input,const Eigen::Matrix4d &M,Mesh* output)
{
	int psize=input->points.size();
	output->points.resize(psize);
	output->indices=input->indices;
	for(int i=0;i<psize;i++)
	{
		Point3D p(input->points[i]);
		Eigen::Vector4d point(p[0],p[1],p[2],1);
		point=M*point;
		output->points[i]=Point3D(point[0],point[1],point[2]);
	}
}