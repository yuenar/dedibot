#include "CollisionDetector.h"
#include <iostream>
using namespace Eigen;

/* 空间中两三角形求交所需宏 */
// 功能:求取三角形与平面的交线段
// 参数:VV0,VV1,VV2:三个顶点在交线上的投影; D0,D1,D2:三个顶点到平面的有向距离; isect0, isect1:交线段交点

// -子功能:根据顶点到平面的有向距离之比和顶点在交线段上的投影点求取交线段端点
#define ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1)  \
              isect0 = VV0 + (VV1-VV0)*D0/(D0-D1); \
              isect1 = VV0 + (VV2-VV0)*D0/(D0-D2);

// -子功能：在三角形中选择出夹交线段的两条边
#define COMPUTE_INTERVALS(VV0,VV1,VV2,D0,D1,D2,D0D1,D0D2,isect0,isect1) \
  if (D0D1 > 0.0) {                                     \
    /* V0 V1 在同侧，V2在异侧 */                          \
    ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);          \
  } else if (D0D2 > 0.0) {                              \
    /* V0 V2 在同侧，V1在异侧 */                          \
    ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);          \
  } else if (D1*D2 > 0.0f || D0 != 0.0f) {              \
    /* V1 V2 在同侧，V2在异侧 */                          \
    ISECT(VV0,VV1,VV2,D0,D1,D2,isect0,isect1);          \
  } else if (D1 != 0.0f) {                              \
    ISECT(VV1,VV0,VV2,D1,D0,D2,isect0,isect1);          \
  } else if (D2 != 0.0f) {                              \
    ISECT(VV2,VV0,VV1,D2,D0,D1,isect0,isect1);          \
  } else {                                              \
    /* 简化共面情况处理，认为两三角形相交 */                 \
    return true;                                        \
  }
/* 空间中两三角形求交所需宏 */

#define FINDMINMAX(x0,x1,x2,min,max) \
    min = max = x0;         \
    if (x1 < min) min = x1; \
    if (x1 > max) max = x1; \
    if (x2 < min) min = x2; \
    if (x2 > max) max = x2;

struct BVIntersecter
{
    BVIntersecter() : calls(0), isIntersect(false) {}
    typedef double Scalar;

//    void MyDebug(TriFacet t) {
//        qDebug() << "(" << t.vertex[0][0] << "," <<  t.vertex[0][1] << "," <<  t.vertex[0][2] << "),("
//            << t.vertex[1][0] << "," <<  t.vertex[1][1] << "," <<  t.vertex[1][2] << "),("
//            << t.vertex[2][0] << "," <<  t.vertex[2][1] << "," <<  t.vertex[2][2] << ")";
//    }

    bool intersectVolumeVolume(const Box3d &b1, const Box3d &b2) { ++calls; return b1.intersects(b2); }
    bool intersectVolumeObject(const Box3d &b, const TriFacet &t) { ++calls; return t.intersects(b); }
    bool intersectObjectVolume(const TriFacet &t, const Box3d &b) { ++calls; return t.intersects(b); }
    bool intersectObjectObject(const TriFacet &t1, const TriFacet &t2) {
        ++calls;
//        MyDebug(t1);
        curIsect = t1.intersects(t2);
        if (curIsect) isIntersect = true;
        return curIsect;
    }

    int calls;
    bool isIntersect;
    bool curIsect;
};

CollisionDetector::CollisionDetector(Mesh &model)
{
    std::vector<TriFacet> facets;
    std::vector<Box3d> boxs;
    int fn = model.indices.size();

    for (int i = 0; i < fn; i++) {
        TriFacet facet;
        Box3d box;
        facet.vertex[0] = model.points[model.indices[i][0]];
        facet.vertex[1] = model.points[model.indices[i][1]];
        facet.vertex[2] = model.points[model.indices[i][2]];
        facets.push_back(facet);
        box.extend(facet.vertex[0]);
        box.extend(facet.vertex[1]);
        box.extend(facet.vertex[2]);
        boxs.push_back(box);
    }

    mBVH.init(facets.begin(), facets.end(), boxs.begin(), boxs.end());
}

bool CollisionDetector::IsCollidedWithMesh(Mesh &mesh)
{
    std::vector<TriFacet> facets;
    std::vector<Box3d> boxs;
    int fn = mesh.indices.size();

    for (int i = 0; i < fn; i++) {
        TriFacet facet;
        Box3d box;
        facet.vertex[0] = mesh.points[mesh.indices[i][0]];
        facet.vertex[1] = mesh.points[mesh.indices[i][1]];
        facet.vertex[2] = mesh.points[mesh.indices[i][2]];
        facets.push_back(facet);
        box.extend(facet.vertex[0]);
        box.extend(facet.vertex[1]);
        box.extend(facet.vertex[2]);
        boxs.push_back(box);
    }
    BVIntersecter intersecter;
    KdBVH<double, 3, TriFacet> curBVH(facets.begin(), facets.end(), boxs.begin(), boxs.end());
    BVIntersect(mBVH, curBVH, intersecter);
//    std::cout << intersecter.calls << std::endl;
    return intersecter.isIntersect;
}

// 空间中三角面片和Box求交(Moller 2001)
bool TriFacet::intersects(const Box3d &b) const
{
    Point3D v0, v1, v2;
    double min, max;
    Point3D normal, vmin, vmax;

    Point3D box_center = b.center();
    Point3D box_half = b.sizes()/2;

    // 将包围盒移到(0,0,0)，同时相应移动其他
    v0 = this->vertex[0] - box_center;
    v1 = this->vertex[1] - box_center;
    v2 = this->vertex[2] - box_center;

    // 测试1：测试三角形在X,Y,Z方向上的重叠
    // 方法：寻找三角形每个方向的最大最小值，判断和AAABB包围盒有没有交
    for (int i = 0; i < 3; i++) {
        min = max = v0[i];
        if (v1[i] < min) min = v1[i];
        if (v1[i] > max) max = v1[i];
        if (v2[i] < min) min = v2[i];
        if (v2[i] > max) max = v2[i];
        if (min > box_half[i] || max < -box_half[i]) return false;
    }

    // 测试2：判断三角形所处平面:normal*x+d=0 是否与AABB包围盒相交
    normal = (v1 - v0).cross(v2 - v1);
    // 找最接近法向normal方向的两个对角点进行测试判断
    for(int i = 0; i < 3; i++) {
        if (normal[i] > 0.0) {
            vmin[i] = -box_half[i] - v0[i];
            vmax[i] =  box_half[i] - v0[i];
        } else {
            vmin[i] =  box_half[i] - v0[i];
            vmax[i] = -box_half[i] - v0[i];
        }
    }
    // 如果最小点在平面正方向或者最大点在平面负方向则不相交
    if (normal.dot(vmin) > 0.0 || normal.dot(vmax) < 0.0) return false;

    return true;
}

// 空间中两三角面片求交(Moller 1997)
inline bool TriFacet::intersects(const TriFacet &t) const
{
    TriFacet TriV = *this, TriU = t;
    Point3D E1, E2;
    Point3D N1, N2;
    double d1,d2;
    double du0,du1,du2,dv0,dv1,dv2;
    Point3D D;
    double isect1[2], isect2[2];
    double du0du1,du0du2,dv0dv1,dv0dv2;
    double vp0,vp1,vp2;
    double up0,up1,up2;
    double b,c,max;
    int index;

    // 计算TriV所在平面1：N1*X + d1=0
    E1 = TriV.vertex[1] - TriV.vertex[0];
    E2 = TriV.vertex[2] - TriV.vertex[0];
    N1 = E1.cross(E2);
    d1 = -N1.dot(TriV.vertex[0]);

    // 将TriU三个顶点带入TriV所在的平面1，来计算点到平面的有向距离
    du0 = N1.dot(TriU.vertex[0]) + d1;
    du1 = N1.dot(TriU.vertex[1]) + d1;
    du2 = N1.dot(TriU.vertex[2]) + d1;

    if (fabs(du0) < eps) du0=0.0;
    if (fabs(du1) < eps) du1=0.0;
    if (fabs(du2) < eps) du2=0.0;
    du0du1 = du0*du1;
    du0du2 = du0*du2;

    // TriU的三个顶点在TriV所在平面的一侧，两三角形不相交
    if(du0du1 > 0.0 && du0du2 > 0.0) return false;

    // 计算TriU所在平面2：N2*X + d2=0
    E1 = TriU.vertex[1] - TriU.vertex[0];
    E2 = TriU.vertex[2] - TriU.vertex[0];
    N2 = E1.cross(E2);
    d2 = -N2.dot(TriU.vertex[0]);

    // 将TriV三个顶点带入TriU所在的平面2
    dv0 = N2.dot(TriV.vertex[0]) + d2;
    dv1 = N2.dot(TriV.vertex[1]) + d2;
    dv2 = N2.dot(TriV.vertex[2]) + d2;

    if(fabs(dv0) < eps) dv0 = 0.0;
    if(fabs(dv1) < eps) dv1 = 0.0;
    if(fabs(dv2) < eps) dv2 = 0.0;
    dv0dv1=dv0*dv1;
    dv0dv2=dv0*dv2;

    // TriV的三个顶点在TriU所在平面的一侧,两三角形不相交
    if(dv0dv1 > 0.0 && dv0dv2 > 0.0) return false;

    // 计算两平面交线
    D = N1.cross(N2);

    // 计算D的最大分量，简化将点投影到交线上的计算，简化交线段的计算
    max = fabs(D[0]);
    index = 0;
    b = fabs(D[1]);
    c = fabs(D[2]);
    if (b > max) max = b, index = 1;
    if (c > max) max = c, index = 2;

    // 将三角形的顶点其投影到简化的交线上
    vp0 = TriV.vertex[0][index];
    vp1 = TriV.vertex[1][index];
    vp2 = TriV.vertex[2][index];

    up0 = TriU.vertex[0][index];
    up1 = TriU.vertex[1][index];
    up2 = TriU.vertex[2][index];

    // 计算TriV的的交线段
    COMPUTE_INTERVALS(vp0,vp1,vp2,dv0,dv1,dv2,dv0dv1,dv0dv2,isect1[0],isect1[1]);

    // 计算TriU的的交线段
    COMPUTE_INTERVALS(up0,up1,up2,du0,du1,du2,du0du1,du0du2,isect2[0],isect2[1]);

    // 检测叫线段是否重叠
    if (isect1[0] > isect1[1]) std::swap(isect1[0], isect1[1]);
    if (isect2[0] > isect2[1]) std::swap(isect2[0], isect2[1]);
    if(isect1[1] < isect2[0] || isect2[1] < isect1[0]) return false;
    return true;
}

