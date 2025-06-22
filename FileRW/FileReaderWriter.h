#ifndef FILEREADERWRITER_H
#define FILEREADERWRITER_H

#include"common.h"
class FileReaderWriter
{
private:
    enum FileType//自定义枚举涵括格式
	{
		READFAIL,OBJ,OFF,STL
	};
	struct Vertex
	{
		double x,y,z;
		int index;
		Vertex(double _x,double _y,double _z,int id)
		{
			x=_x,y=_y,z=_z;
			index=id;
		}
		Vertex(const Point3D &p,int id)
		{
			x=p[0],y=p[1],z=p[2];
			index=id;
		}
		friend bool operator<(const Vertex& A,const Vertex& B)
		{
			if(A.x+eps<B.x||fabs(A.x-B.x)<eps&&A.y+eps<B.y||fabs(A.x-B.x)<eps&&abs(A.y-B.y)<eps&&A.z+eps<B.z)
				return true;
			return false;
		}
		friend bool operator==(const Vertex&A,const Vertex& B)
		{
			if(fabs(A.x-B.x)<eps&&fabs(A.y-B.y)<eps&&fabs(A.z-B.z)<eps)
				return true;
			return false;
		}
		friend bool operator!=(const Vertex&A,const Vertex& B)
		{
			return !(A==B);
		}
	};

	char path[128];
	Mesh* meshR;
	const Mesh* meshW;
	float progress;
	std::vector<Vertex>vertices;
private:
	FileType FormatPath();
	bool ConvertToMesh();
	void LoadOBJ();
	bool SaveOBJ();
	void LoadSTL();
	bool LoadBinarySTL();
	bool LoadAssicSTL();
	bool SaveSTL();
public:
	FileReaderWriter();
    ~FileReaderWriter();

	bool LoadModel(const char* filePath,Mesh* input);//定义载入及保存模型槽函数
	bool SaveModel(const char* filePath,const Mesh* input);

	bool Save(const char* filePath);
	bool Load(const char* filePath);

	float GetProgress() { return progress;}
};
#endif
