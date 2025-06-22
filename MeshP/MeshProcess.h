#ifndef MESHPROCESS_H
#define MESHPROCESS_H
#include"common.h"
class MeshProcess
{
private:
	//static void MeshToCTM(Mesh &mesh,CorkTriMesh &ctm);
	//static void CTMToMesh(CorkTriMesh& ctm,Mesh &mesh);
public:
	static void MeshUnion(Mesh* m1,const Mesh* m2);//m1=m1+m2;
	static void MeshTransform(const Mesh* input,const Eigen::Matrix4d &M,Mesh* output);//M*input=output
	static void UpdateNormals(Mesh* mesh);
	//static void MeshAdd(Mesh& add1,Mesh& add2,Mesh& res);//res=add1+add2
	//static void MeshSub(Mesh& sub1,Mesh& sub2,Mesh& res);//res=sub1-sub2
	//static void MeshInt(Mesh& int1,Mesh& int2,Mesh& res);//res=int1*int2
	//static void MeshXor(Mesh& xor1,Mesh& xor2,Mesh& res);//res=int1^int2
};
#endif//MESHPROCESS_H