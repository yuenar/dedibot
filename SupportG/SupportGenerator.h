#ifndef SUPPORTGENERATOR
#define SUPPORTGENERATOR

#include "common.h"
#include "CollisionDetector.h"

#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
struct MyTraits : OpenMesh::DefaultTraits
{
    typedef OpenMesh::Vec3d Point;
    typedef OpenMesh::Vec3d Normal;
    VertexTraits {
    private:
        bool overhang_;
    public:
        VertexT():overhang_(false) { }
        const bool overhang() const { return overhang_; }
        void set_overhang(const bool oh) { overhang_ = oh; }
    };
    EdgeTraits {
    private:
        bool overhang_;
    public:
        EdgeT():overhang_(false) { }
        const bool overhang() const { return overhang_; }
        void set_overhang(const bool oh) { overhang_ = oh; }
    };
};
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>  MyMesh;

struct WallPara {
	Point3D p_tl, p_tr, p_bl, p_br;    // 上下左右四个顶点位置
	double th;                      // 厚度
};

struct BallPara {
	Point3D p_center;   // 球心位置
	double r;           // 半径
};

struct CylinderPara {
	Point3D p_center;   // 中心位置
	double h;           // 高度
	double r;           // 底面半径
};

struct TruConePara {
	Point3D p_center;   // 中心位置
    double r_top;       // 上底面半径
    double r_bottom;    // 下底面半径
	double h;           // 圆台高度
};

struct PillarPara {
    Point3D c_ball_top;     // 顶部球心
    Point3D c_ball_bottom;  // 底部球心
    double r_ball_top;      // 顶部球半径
    double r_ball_bottom;   // 底部球半径

    Point3D d_tc_top;       // 顶部圆台朝向
    Point3D d_tc_bottom;    // 底部圆台朝向

    Point3D c_cyl;      // 圆柱中心
    double r_cyl;       // 圆柱半径
    double h;           // 圆柱高度

    bool has_wall;      // 有没有墙相连
};

// 最基本的支撑结构
struct Supporter_Classic {
	std::vector<WallPara> walls;
    std::vector<PillarPara> pillars;
};

// 半边结构
struct HalfEdge {
    int to;         // 边的终点
    int facet;      // 半边所属的面片
    bool overhang;  // 边是否悬挂需要支撑
};

class SupportGenerator {
private:
    MyMesh mesh;

    CollisionDetector *m_cd;
    double Th_w, R_b, R_c, H_tc;    // 一些设置的值，墙厚、球半径、圆柱半径、圆台高度（受倾斜角限制）
    double H_model, Th_c;           // 模型距打印底盘的高度、底盘厚度
    double D_sample, D_sample_edge; // 采样距离

    double x_min, x_max, y_min, y_max;  // 模型xy边界
    int n_x, n_y;               // x,y方向采样点数
    double x_start, y_start;    // 起始采样点x,y值

    float progress, progress_div;        // 支撑生成的进度, 进度增量

    void OverhangPointDetection();
    void OverhangEdgeDetection();

    Supporter_Classic GeneratePEClassicSupporter();
    Supporter_Classic GenerateFacetClassicSupporter(std::vector<Supporter_Classic> &support_map);

    void BuildPillarOP(Supporter_Classic *s, Point3D p_overhang, std::vector<double> &dis, std::vector<Point3D> &norm);
    void BuildPillarOF(Supporter_Classic *s, Point3D p_support, std::vector<double> &dis, std::vector<Point3D> &norm);
    bool BuildWall(Supporter_Classic *s, Point3D c1_t, Point3D c1_b, Point3D c2_t, Point3D c2_b);
    void BoolOpertaionAdapter(Supporter_Classic *s);

    bool PillarCollisonDetection(PillarPara pillar);
    bool WallCollisonDetection(WallPara wall);
    int Perpendicular(Point3D origin, Point3D dir, std::vector<double> &dis, std::vector<Point3D> &norm);
    int Intersect(Point3D origin, Point3D dir, std::vector<double> &dis, std::vector<int> &index);
public:
	SupportGenerator();
    SupportGenerator(double th_w, double r_b, double h_tc, double h_model, double d_sample, double d_sample_e);

	void Reset();

    void SetMesh(Mesh *m);

	void SetWallThickness(double th_w);
	void SetBallRadius(double r_b);
    void SetConeRadius(double r_c);
	void SetTruConeHeight(double h_tc);
	void SetModelHeight(double h_model);
	void SetSamplingDistance(double d_sample);
    void SetEdgeSamplingDistance(double d_sample_e);
    void SetChassisThickness(double th_c);

	float GetProgress(){return progress;}
    Supporter_Classic GenerateClassicSupporter();
};

#endif // SUPPORTGENERATOR

