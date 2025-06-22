#include"MeshList.h"
MeshList* MeshList::instance=0;
MeshList::MeshList()
{

}
int MeshList::InsertMesh(Mesh* mesh)
{
	int i,size=meshes.size();
	for(i=0;i<size;i++)
	{
		if(meshes[i]==NULL)
		{
			meshes[i]=mesh;
			return i;
		}	
	}
	meshes.push_back(mesh);
	return i;
}
void MeshList::SetMesh(const int idx,Mesh* mesh)
{
	int size=meshes.size();
	if(idx>=size)
		meshes.resize(idx+1);
	meshes[idx]=mesh;
}

const Mesh* MeshList::GetMesh(const int idx)const
{
	int size=meshes.size();
	if(idx>=0&&idx<size&&meshes[idx])
		return meshes[idx];
	return NULL;
}

void MeshList::Remove(const int idx)
{
	int i,size=meshes.size();
	if(idx>=0&&idx<size)
	{
		if(meshes[idx])
		{
			delete meshes[idx];
		}
		meshes[idx]=NULL;
	}
	i=size-1;
	while(i>=0&&meshes[i]==NULL)
		i--;
	if(i!=size-1)
		meshes.resize(i+1);
}
void MeshList::GetNullPos(std::vector<int>&pos)
{
	pos.clear();
	int size=meshes.size();
	for(int i=0;i<size;i++)
	{
		if(meshes[i]==NULL)
			pos.push_back(i);
	}
}
void MeshList::Clear(int flag)
{
	int size=meshes.size();
	for(int i=0;i<size;i++)
	{
		if(meshes[i])
			delete meshes[i];
	}
	meshes.clear();
	if(flag)
	{
		delete instance;
		instance=NULL;
	}
}
