#ifndef SIMPLETRIANGULATION_H
#define SIMPLETRIANGULATION_H
#include"common.h"
class SimpleTriangulation
{
private:
    std::vector<Point2D,Eigen::aligned_allocator<Point2D> >contour;//轮廓 
    bool InTriangle(const int a,const int b,const int c,const int p);//判定是否转换成三角数据 a、b、c是三角形三个顶点，p是确定是否在三角形内部的参照点
public:
	SimpleTriangulation();
	~SimpleTriangulation();

    void SetInput(std::vector<Point3D>&contourXYZ);//设置输入
    void SetInput(std::vector<Point2D,Eigen::aligned_allocator<Point2D>>&contourXY);//重载

    void GetOutput(std::vector<Index3D>&tris);//获取输出
};

#endif