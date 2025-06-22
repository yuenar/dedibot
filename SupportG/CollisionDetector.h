#ifndef COLLISIONDETECT_H
#define COLLISIONDETECT_H

#include "common.h"
#include <unsupported/Eigen/BVH>

typedef Eigen::AlignedBox<double, 3> Box3d;

struct TriFacet {
    Point3D vertex[3];
    bool intersects(const Box3d &b) const;
    bool intersects(const TriFacet &t) const;
};

class CollisionDetector
{
public:
    CollisionDetector(Mesh &model);
    bool IsCollidedWithMesh(Mesh &mesh);
private:
    Eigen::KdBVH<double, 3, TriFacet> mBVH;
};

#endif // COLLISIONDETECT_H
