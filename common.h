#ifndef COMMON
#define COMMON

#include<vector>
#include"Eigen/Dense"
typedef Eigen::Vector3i Index3D;
typedef Eigen::Vector3d Point3D;
typedef Eigen::Vector2d Point2D;
const static double eps=1e-6;
const static double pi = acos(-1.0);

struct Mesh
{
	std::vector<Point3D> points;
	std::vector<Index3D> indices;
	std::vector<Point3D> normals;
};
	
#endif // COMMON

