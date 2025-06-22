#ifndef MESHLIST_H
#define MESHLIST_H
#include<vector>
#include<iostream>
struct Mesh;
class MeshList
{
private:
	static MeshList* instance;
	std::vector<Mesh*>meshes;
	MeshList();
public:
	static MeshList* GetInstance()
	{
		if(instance==NULL)
			instance=new MeshList;
		return instance;
	}
	void SetMesh(const int idx,Mesh* mesh);
	int InsertMesh(Mesh* mesh);
	void Remove(const int idx);
	void Clear(int flag=0);

	const Mesh* GetMesh(const int idx)const;
	int GetSize()const { return meshes.size(); }
	void GetNullPos(std::vector<int>&pos);
};
#endif
