#include<string.h>
#include<fstream>
#include<sstream>
#include<math.h>
#include<algorithm>
#include<iostream>
#include"FileReaderWriter.h"
#include"SimpleTriangulation.h"

#include"Data/ProjectData.h"
#include"Data/ModelData.h"
#include"Data/MeshList.h"
using namespace std;
FileReaderWriter::FileReaderWriter()
{
}
FileReaderWriter::~FileReaderWriter()
{
}
FileReaderWriter::FileType FileReaderWriter::FormatPath()
{//格式路径，同时过滤文件名
	char p[128];
	int len=strlen(path);
	for(int i=0;i<=len;i++)
	{
		if(path[i]>='A'&&path[i]<='Z')
			p[i]=path[i]-'A'+'a';
		else
			p[i]=path[i];
	}
    if(strcmp(p+len-4,".obj")==0)//根据文件名字符串后四位来区分文件格式
		return OBJ;
	else if(strcmp(p+len-4,".off")==0)
		return OFF;
	else if(strcmp(p+len-4,".stl")==0)
		return STL;
	else
		return READFAIL;
}
bool FileReaderWriter::ConvertToMesh()
{//判定是否可转换成网格
	int i,psize=vertices.size(),fsize=psize/3;
	if(psize==0)
		return false;
	for(i=0;i<psize;i++)
		vertices[i].index=i;
	meshR->indices.resize(fsize);
	for(i=0;i<fsize;i++)
	{
		meshR->indices[i][0]=3*i;
		meshR->indices[i][1]=3*i+1;
		meshR->indices[i][2]=3*i+2;
	}
	progress=93;
	sort(vertices.begin(),vertices.end());
	progress=96;
	int k=0;
	meshR->indices[vertices[0].index/3][vertices[0].index%3]=k;
	for(i=1;i<psize;i++)
	{
		if(vertices[i]!=vertices[k])
		{
			vertices[++k]=vertices[i];
		}
		meshR->indices[vertices[i].index/3][vertices[i].index%3]=k;
	}
	psize=k+1;
	meshR->points.resize(psize);
	for(i=0;i<psize;i++)
	{
		meshR->points[i]=Point3D(vertices[i].x,vertices[i].y,vertices[i].z);
	}
	progress=97;
	vector<int>cnt;
	cnt.resize(psize);
	meshR->normals.resize(psize);
	for(i=0;i<psize;i++)
	{
		cnt[i]=0;
		meshR->normals[i]=Point3D(0,0,0);
	}
	int a,b,c;
	Point3D AB,AC,normal;
	for(i=0;i<fsize;i++)
	{
		a=meshR->indices[i][0],b=meshR->indices[i][1],c=meshR->indices[i][2];
		AB=meshR->points[b]-meshR->points[a];
		AC=meshR->points[c]-meshR->points[a];
		normal=AB.cross(AC).normalized();
		meshR->normals[a]+=normal,cnt[a]++;
		meshR->normals[b]+=normal,cnt[b]++;
		meshR->normals[c]+=normal,cnt[c]++;
	}
	progress=98;
	for(i=0;i<psize;i++)
	{
		meshR->normals[i]/=cnt[i];
		meshR->normals[i].normalize();
	}
	return true;
}
bool FileReaderWriter::LoadModel(const char* filePath,Mesh* input)
{//载入模型
	strcpy(path,filePath);
	meshR=input;
	FileType type=FormatPath();
	vertices.clear();
	switch(type)
	{
		case READFAIL:
			return false;
		case OBJ:
			LoadOBJ();break;
		case STL:
			LoadSTL();break;
	}
	return ConvertToMesh();
}
bool FileReaderWriter::SaveModel(const char* filePath,const Mesh* input)
{//保存模型
	strcpy(path,filePath);
	meshW=input;
	FileType type=FormatPath();
	switch(type)
	{
		case READFAIL:
			return false;
		case OBJ:
			return SaveOBJ();
		case STL:
			return SaveSTL();
	}
	return true;
}
void FileReaderWriter::LoadSTL()
{
	if(LoadBinarySTL()==true)
		return;
	LoadAssicSTL();
}
bool FileReaderWriter::LoadAssicSTL()
{
	ifstream ifs(path);
	if(ifs.fail())
		return false;
	int cnt=0;
	ifs.seekg(0,ios_base::end);
	int fileSize=(int)ifs.tellg();
	ifs.seekg(0,ios_base::beg);
	double x,y,z;
	string line,flag;
	while(getline(ifs,line))
	{
		stringstream ss(line);
		ss>>flag;
		if(flag=="")
			continue;
		if(flag=="vertex")
		{
			ss>>x>>y>>z;
			vertices.push_back(Vertex(x,y,z,cnt++));
		}
		else if(flag=="solid"||flag=="facet"||flag=="outer"||flag=="endloop"||flag=="endfacet"||flag=="endsolid")
			continue;
		else
		{
			vertices.clear();
			return false;
		}
		progress=ifs.tellg()*1.0/fileSize*92;
	}
	return true;
}
bool FileReaderWriter::LoadBinarySTL()
{
	ifstream ifs(path,ios_base::in|ios_base::binary);
	if(ifs.fail())
		return false;
	ifs.seekg(0,ios_base::end);
	int fileSize=(int)ifs.tellg();
	ifs.seekg(0,ios_base::beg);
	if(fileSize<84)
		return false;

	char comment[81];
	ifs.read(comment,80);
	comment[80]=0;
	int count;
	ifs.read((char*)&count,sizeof(int));
	if(fileSize!=(84+count*50))
		return false;

	int cnt=0;
	float x,y,z;
	for(int i=0;i<count;i++)
	{
		ifs.seekg(12,ios_base::cur);
		for(int j=0;j<3;j++)
		{
			ifs.read((char*)&x,sizeof(float));
			ifs.read((char*)&y,sizeof(float));
			ifs.read((char*)&z,sizeof(float));
			vertices.push_back(Vertex(x,y,z,cnt++));
			progress=ifs.tellg()*1.0/fileSize*92;
		}
		ifs.seekg(2,ios_base::cur);
	}
	ifs.close();
	return true;
}
bool FileReaderWriter::SaveSTL()
{
	ofstream ofs(path,ios_base::out|ios_base::binary);
	if(ofs.fail())
		return false;
	char comment[80]="";
	ofs.write(comment,80);
	int count=meshW->indices.size();
	ofs.write((char*)&count,4);
	float x,y,z;
	Point3D dirAB,dirAC,normal;
	for(int i=0;i<count;i++)
	{
		dirAB=meshW->points[meshW->indices[i][1]]-meshW->points[meshW->indices[i][0]];
		dirAC=meshW->points[meshW->indices[i][2]]-meshW->points[meshW->indices[i][0]];
		normal=(dirAB.cross(dirAC)).normalized();
		x=(float)normal[0],y=(float)normal[1],z=(float)normal[2];
		ofs.write((char*)&x,4);
		ofs.write((char*)&y,4);
		ofs.write((char*)&z,4);
		for(int j=0;j<3;j++)
		{
			x=(float)meshW->points[meshW->indices[i][j]][0],y=(float)meshW->points[meshW->indices[i][j]][1],z=(float)meshW->points[meshW->indices[i][j]][2];
			ofs.write((char*)&x,4);
			ofs.write((char*)&y,4);
			ofs.write((char*)&z,4);
		}
		ofs.write("",2);
		progress=i*1.0/count*100;
	}
	ofs.close();
	return true;
}
void FileReaderWriter::LoadOBJ()
{
	ifstream ifs(path);
	if(ifs.fail())
		return;

	ifs.seekg(0,ios_base::end);
	int fileSize=(int)ifs.tellg();
	ifs.seekg(0,ios_base::beg);
	int cnt=0,size;
	string s,op,str;
	Point3D p3d;
	vector<Point3D>points;
	while(getline(ifs,s))
	{
		if(s.find('v')==-1&&s.find('f')==-1)
			continue;
		stringstream ss(s);
		ss>>op;
		if(op=="v")
		{
			ss>>p3d[0]>>p3d[1]>>p3d[2];
			points.push_back(p3d);
		}
		else if(op=="f")
		{
			int idx;
			vector<int>v;
			while(ss>>idx)
			{
				if(s.find('/')!=-1)
					ss>>str;
				if(idx>0)
					idx--;
				else
					idx=points.size()+idx;
				v.push_back(idx);
			}
			size=v.size();
			if(size!=3)
			{
				SimpleTriangulation st;
				vector<Point3D>contour;
				vector<Index3D>tris;
				for(int k=0;k<size;k++)
					contour.push_back(points[v[k]]);
				st.SetInput(contour);
				st.GetOutput(tris);
				size=tris.size();
				for(int k=0;k<size;k++)
				{
					vertices.push_back(Vertex(points[v[tris[k][0]]],cnt++));
					vertices.push_back(Vertex(points[v[tris[k][1]]],cnt++));
					vertices.push_back(Vertex(points[v[tris[k][2]]],cnt++));
				}
			}
			else
			{
				for(int k=0;k<3;k++)
					vertices.push_back(Vertex(points[v[k]],cnt++));
			}
		}
		progress=1.0*ifs.tellg()/fileSize*92;
	}
	ifs.close();
}
bool FileReaderWriter::SaveOBJ()
{
	ofstream ofs(path);
	if(ofs.fail())
		return false;
	int np=meshW->points.size(),nf=meshW->indices.size();
	for(int i=0;i<np;i++)
	{
		ofs<<"v "<<meshW->points[i][0]<<" "<<meshW->points[i][1]<<" "<<meshW->points[i][2]<<endl;
		progress=i*1.0/(np+nf)*100;
	}
	for(int i=0;i<nf;i++)
	{
		ofs<<"f "<<meshW->indices[i][0]+1<<" "<<meshW->indices[i][1]+1<<" "<<meshW->indices[i][2]+1<<endl;
		progress=(np+i)*1.0/(np+nf)*100;
	}
	ofs.close();
	return true;
}

bool FileReaderWriter::Save(const char* path)
{
    ofstream ofs(path,ios_base::out|ios_base::binary);
    if(ofs.fail())
        return false;
    char comment[128]="Create By Dedibot V1.0.0";
    ofs.write(comment,32);
	int size=MeshList::GetInstance()->GetSize();
	int allp=0,allf=0,all=0;
	for(int i=0;i<size;i++)
	{
		const Mesh *mesh=MeshList::GetInstance()->GetMesh(i);
		if(mesh)
		{
			allp+=mesh->points.size();
			allf+=mesh->indices.size();
		}
	}
	ofs.write((char*)(&size),4);
	ofs.write((char*)(&allp),4);
	ofs.write((char*)(&allf),4);
    for(int i=0;i<size;i++)
    {
        int np=0,nf=0;
        const Mesh* mesh=MeshList::GetInstance()->GetMesh(i);
        if(mesh)
        {
            np=mesh->points.size();
            nf=mesh->indices.size();
        }
        ofs.write((char*)&np,4);
        ofs.write((char*)&nf,4);
        for(int j=0;j<np;j++,all++)
        {
            ofs.write((char*)mesh->points[j].data(),3*8);
			progress=all*100.0/(allp+allf);
        }
        for(int j=0;j<nf;j++,all++)
        {
            ofs.write((char*)mesh->indices[j].data(),3*4);
			progress=all*100.0/(allp+allf);
        }
    }
    size=ProjectData::GetInstance()->GetSize();
    int nsupportp=SupportParam::SupportParamEnd;
    //int nslicep=SliceParam::SliceParamEnd;
    ofs.write((char*)(&size),4);
    ofs.write((char*)(&nsupportp),4);
    //ofs.write((char*)(&nslicep),4);
    for(int i=0;i<size;i++)
    {
        ModelData* md=ProjectData::GetInstance()->GetModelData(i);
        int modelId=md->GetModelId(),supportId=md->GetSupportId();
        bool modelVisible=md->IsModelVisible(),supportVisible=md->IsSupportVisible();
        double defaultScale=md->GetDefaultScale(),scale=md->GetScaleValue();
        Point3D move=md->GetMoveValue(),rotate=md->GetRotateValue();
        const double *supportParam=md->GetSupportParam();
        ofs.write(md->GetModelName().data(),32);
        ofs.write((char*)&modelId,4),ofs.write((char*)&supportId,4);
        ofs.write((char*)&modelVisible,1),ofs.write((char*)&supportVisible,1);
        ofs.write((char*)&defaultScale,8),ofs.write((char*)&scale,8);
        ofs.write((char*)&move[0],3*8);
        ofs.write((char*)&rotate[0],3*8);
        ofs.write((char*)supportParam,nsupportp*8);
    }
    //const double* sliceParam=ProjectData::GetInstance()->GetSliceParam();
    //ofs.write((char*)sliceParam,nslicep*8);
    int curIdx=ProjectData::GetInstance()->GetSelectModelIndex();
    ofs.write((char*)&curIdx,4);
	return true;
}
bool FileReaderWriter::Load(const char* path)
{
        ifstream ifs(path,ios_base::in|ios_base::binary);
        if(ifs.fail())
            return false;
        char comment[128];
        ifs.read(comment,32);
        if(strcmp(comment,"Create By Dedibot V1.0.0")!=0)
            return false;
		int size,nsupportp,allp,allf,all=0;
		//int nslicep;
		ifs.read((char*)&size,4);
		ifs.read((char*)&allp,4);
		ifs.read((char*)&allf,4);
        for(int i=0;i<size;i++)
        {
            int np,nf;
            Mesh* mesh=NULL;
            ifs.read((char*)&np,4);
            ifs.read((char*)&nf,4);
            if(np)
            {
                mesh=new Mesh;
                mesh->points.resize(np);
                mesh->indices.resize(nf);
            }
            for(int j=0;j<np;j++,all++)
            {
                ifs.read((char*)mesh->points[j].data(),3*8);
				progress=all*100.0/(allp+allf);
            }
            for(int j=0;j<nf;j++,all++)
            {
                ifs.read((char*)mesh->indices[j].data(),3*4);
				progress=all*100.0/(allp+allf);
            }
			MeshList::GetInstance()->SetMesh(i,mesh);
        }
        ifs.read((char*)&size,4);
        ifs.read((char*)&nsupportp,4);
        //ifs.read((char*)&nslicep,4);
        int modelId,supportId;
        bool modelVisible,supportVisible;
        double defaultScale,scale;
        double supportParam[100];
        //double sliceParam[100];
        Point3D move,rotate;
        char modelName[32];
        for(int i=0;i<size;i++)
        {
            ifs.read(modelName,32);
            ifs.read((char*)&modelId,4),ifs.read((char*)&supportId,4);
            ifs.read((char*)&modelVisible,1),ifs.read((char*)&supportVisible,1);
            ifs.read((char*)&defaultScale,8),ifs.read((char*)&scale,8);
            ifs.read((char*)&move[0],3*8);
            ifs.read((char*)&rotate[0],3*8);
            ifs.read((char*)supportParam,nsupportp*8);
            ModelData* md=new ModelData;
            md->SetModelName(modelName);
            md->SetModelId(modelId),md->SetSupportId(supportId);
            md->SetModelVisible(modelVisible),md->SetSupportVisible(supportVisible);
            md->SetDefaultScale(defaultScale),md->SetScaleValue(scale);
            md->SetMoveValue(move),md->SetRotateValue(rotate);	
            md->SetSupportParam(supportParam);
            ProjectData::GetInstance()->InsertModelData(md);
        }
        //ifs.read((char*)sliceParam,nslicep*8);
        //ProjectData::GetInstance()->SetSliceParam(sliceParam);
        int curIdx;
        ifs.read((char*)&curIdx,4);
        ProjectData::GetInstance()->SetSelectModelIndex(curIdx);
	return true;
}
