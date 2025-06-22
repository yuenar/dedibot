#include "SupportGenerator.h"
#include "MeshG/TriMeshGenerator.h"
#include <cmath>
#include <iostream>

using namespace std;

SupportGenerator::SupportGenerator()
{
	Reset();
}

SupportGenerator::SupportGenerator(double th_w, double r_b, double h_tc, double h_model, double d_sample, double d_sample_e)
{
	Th_w = th_w;
	R_b = r_b;
    R_c = th_w;
    H_tc = h_tc;
    H_model = h_model;
	D_sample = d_sample;
    D_sample_edge = d_sample_e;
    m_cd = nullptr;
}

void SupportGenerator::Reset()
{
	Th_w=0.7;
	R_b=0.35;
	R_c=0.7;
	H_tc=0.7;
	H_model=7.5;
	D_sample=7.5;
	D_sample_edge=3.75;
	progress=0;
	m_cd=nullptr;
	mesh.clear();
}

void SupportGenerator::SetMesh(Mesh *m)
{
    int pn = m->points.size();
    int fn = m->indices.size();
	
    // 将Mesh数据转换为OpenMesh的数据
    vector<MyMesh::VertexHandle> vhandle;
    for (int i = 0; i < pn; i++)
        vhandle.push_back(mesh.add_vertex(MyMesh::Point(m->points[i][0], m->points[i][1], m->points[i][2])));
    for (int i = 0; i < fn; i++)
        mesh.add_face(vhandle[m->indices[i][0]], vhandle[m->indices[i][1]], vhandle[m->indices[i][2]]);
    mesh.request_face_normals();
    mesh.update_normals();
//    OpenMesh::IO::write_mesh(mesh, "/Users/Johnson/Library/Application Support/Dedibot/mesh.off");

    // 将模型平移至zplane以上H_model距离
    double z_min = INFINITY;
    MyMesh::VertexIter v_it, v_end(mesh.vertices_end());
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it) z_min = min(z_min, mesh.point(*v_it)[2]);
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it) mesh.point(*v_it)[2] += H_model - z_min;
    for (int i = 0; i < pn; i++) m->points[i][2] += H_model - z_min;

    // 计算模型xy边界
    x_min = INFINITY, y_min = INFINITY;
    x_max = -INFINITY, y_max = -INFINITY;
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it) {
        x_min = min(x_min, mesh.point(*v_it)[0]);
        x_max = max(x_max, mesh.point(*v_it)[0]);
        y_min = min(y_min, mesh.point(*v_it)[1]);
        y_max = max(y_max, mesh.point(*v_it)[1]);
    }

    // 建立采样网格
    n_x = ceil((x_max - x_min) / D_sample) + 1;  // x方向采样点数
    n_y = ceil((y_max - y_min) / D_sample) + 1;  // y方向采样点数
    x_start = (x_max + x_min) / 2 - (n_x - 1)*D_sample / 2;    // 起始采样点x值
    y_start = (y_max + y_min) / 2 - (n_y - 1)*D_sample / 2;    // 起始采样点y值

    // 设置碰撞检测器
    if (m_cd != nullptr) delete m_cd;
    m_cd = new CollisionDetector(*m);
}

void SupportGenerator::SetWallThickness(double th_w){ Th_w = th_w;R_c = th_w; }
void SupportGenerator::SetBallRadius(double r_b) { R_b = r_b; }
void SupportGenerator::SetConeRadius(double r_c) { R_c = r_c; }
void SupportGenerator::SetTruConeHeight(double h_tc) { H_tc = h_tc; }
void SupportGenerator::SetModelHeight(double h_model) { H_model = h_model; }
void SupportGenerator::SetSamplingDistance(double d_sample) { D_sample = d_sample; }
void SupportGenerator::SetEdgeSamplingDistance(double d_sample_e) { D_sample_edge = d_sample_e; }
void SupportGenerator::SetChassisThickness(double th_c){ Th_c = th_c; }

// 悬挂点检测
void SupportGenerator::OverhangPointDetection()
{
    MyMesh::VertexIter v_it, v_end(mesh.vertices_end());
    MyMesh::VertexVertexIter vv_it;
    MyMesh::VertexFaceIter vf_it;
	// 检测所有点,若有点相邻的点都比它高，则标记该点悬挂
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it) {
        double pz = mesh.point(*v_it)[2]; // 当前Check点的z值
        // 法向都朝上的面片无需支撑
        for (vf_it = mesh.vf_iter(*v_it); vf_it; ++vf_it) {
            if (mesh.normal(*vf_it)[2] < -eps) {
                mesh.data(*v_it).set_overhang(true);
                break;
            }
        }
        // 遍历包含该点的所有面片，若有面片的其他点有比它低的，则标记无需支撑
        for (vv_it = mesh.vv_iter(*v_it); vv_it; ++vv_it) {
            if ((mesh.point(*vv_it)[2] - pz) < eps) {
                mesh.data(*v_it).set_overhang(false);
                break;
            }
        }
	}
}

void SupportGenerator::OverhangEdgeDetection()
{
    MyMesh::EdgeIter e_it, e_end(mesh.edges_end());
    MyMesh::FaceHandle f1, f2;          // 一条边相邻的两个面
    MyMesh::HalfedgeHandle he1, he2;    // 一条边对应的两天半边
    MyMesh::Point v1, v2, v3, v4;       // 一条边上两个顶点，以及相邻面上的另外两个顶点
    MyMesh::Point dir, n;               // v1->v2的方向，以及该边所属面的法向
    double d1, d2, l1, l2;

//    MyMesh edge_mesh;
//    vector<MyMesh::VertexHandle> vhandle;
//    int i = 0;
    // 遍历包含该点的所有边，若有它为领域内的极低边，则需支撑
    for (e_it = mesh.edges_begin(); e_it != e_end; ++e_it) {
        he1 = mesh.halfedge_handle(e_it, 0);
        he2 = mesh.halfedge_handle(e_it, 1);
        f1 = mesh.face_handle(he1);
        f2 = mesh.face_handle(he2);
        // 相邻面片法向都朝上无需支撑
        if (!f1.is_valid() || !f2.is_valid()) continue;
        if (mesh.normal(f1)[2] > -eps && mesh.normal(f2)[2] > -eps) continue;

        v1 = mesh.point(mesh.to_vertex_handle(he1));
        v2 = mesh.point(mesh.to_vertex_handle(he2));
        v3 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(he1)));
        v4 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(he2)));

        // 边不能太倾斜
        dir = (v1 - v2).normalize();
        if (dir[2] < 0) dir = -dir;
        if (dir[2] > 0.5) continue;

        // 把v3,v4投影到dir,(-dir[1], dir[0], 0)为基的平面上，计算点到平面的距离
        n = dir % MyMesh::Point(-dir[1], dir[0], 0);
        d1 = (v3 - v1) | n;
        d2 = (v4 - v1) | n;

        // 计算面片边的平均值作为eps的尺度
        l1 = 0, l2 = 0;
        for (int i = 0; i < 3; i++) {
            l1 += mesh.calc_edge_length(he1);
            l2 += mesh.calc_edge_length(he2);
            he1 = mesh.next_halfedge_handle(he1);
            he2 = mesh.next_halfedge_handle(he2);
        }
        l1 /= 300;
        l2 /= 300;

        // 该边为1-邻域内极低边则需要支撑
        if ((d1 > l1 && d2 > -l2) || (d1 > -l1 && d2 > l2)) {
            mesh.data(*e_it).set_overhang(true);
//            vhandle.push_back(edge_mesh.add_vertex(v1));
//            vhandle.push_back(edge_mesh.add_vertex(v2));
//            vhandle.push_back(edge_mesh.add_vertex(v3));
//            vhandle.push_back(edge_mesh.add_vertex(v4));
//            edge_mesh.add_face(vhandle[i+1], vhandle[i], vhandle[i+2]);
//            edge_mesh.add_face(vhandle[i], vhandle[i+1], vhandle[i+3]);
//            i += 4;
        }
    }
//    OpenMesh::IO::write_mesh(edge_mesh, "/Users/Johnson/Library/Application Support/Dedibot/output.off");
}

Supporter_Classic SupportGenerator::GenerateClassicSupporter()
{
    progress = 0;
    // 分别计算悬挂点和悬挂面的支撑结构
    Supporter_Classic s1 = GeneratePEClassicSupporter();
    vector<Supporter_Classic> support_map;
    Supporter_Classic s2 = GenerateFacetClassicSupporter(support_map);

    // 删除多余支柱
    PillarPara p1, p2;
    Point2D p_sample;
    vector<PillarPara>::iterator it1 = s1.pillars.begin();
    double dis, diff_z;
    while(it1 != s1.pillars.end()) {
        p1 = *it1;
        int i = round((p1.c_cyl[0] - x_min)/D_sample);
        int j = round((p1.c_cyl[1] - y_min)/D_sample);
        p_sample = Point2D(i*D_sample + x_start, j*D_sample + y_start);
        dis = (p_sample - Point2D(p1.c_cyl[0],p1.c_cyl[1])).norm();
        // 删除离格点太近且已经有支撑的的悬挂点支撑柱
        if(support_map[i*n_y+j].pillars.size() > 0 && dis - 2*R_c < -eps)
            it1 = s1.pillars.erase(it1);
        else {
            vector<PillarPara>::iterator it2 = s1.pillars.begin();
            while(it2 != s1.pillars.end()) {
                p2 = *it2;
                dis = (Point2D(p2.c_cyl[0],p2.c_cyl[1]) - Point2D(p1.c_cyl[0],p1.c_cyl[1])).norm();
                diff_z = abs(p2.c_cyl[2] - p1.c_cyl[2]) - (p2.h + p1.h)/2;
                // 删除距离太近的悬挂点支撑柱
                if(it1 != it2 && dis - 2*R_c < -eps && diff_z < 0)
                    it2 = s1.pillars.erase(it2);
                else it2++;
            }
            it1++;
        }
    }

    // 合并支撑结构
    Supporter_Classic s;
    s.pillars.insert(s.pillars.end(), s1.pillars.begin(), s1.pillars.end());
    s.pillars.insert(s.pillars.end(), s2.pillars.begin(), s2.pillars.end());
    s.walls.insert(s.walls.end(), s1.walls.begin(), s1.walls.end());
    s.walls.insert(s.walls.end(), s2.walls.begin(), s2.walls.end());

    // 为没有墙的柱子建墙
    PillarPara *p3, *p4;
    double min_dis, cur_dis;
    int min_index;   // 距离当前柱子最近的柱子的索引
    for (int i = 0; i < s.pillars.size(); i++) {
        min_dis = INFINITY;
        p3 = &s.pillars[i];
        if(!p3->has_wall) {
            // 在面支撑结构中找距离当前柱子最近的一根柱子
            for (int j = 0; j < s.pillars.size(); j++) {
                p4 = &s.pillars[j];
                cur_dis = (Point2D(p3->c_cyl[0],p3->c_cyl[1]) - Point2D(p4->c_cyl[0],p4->c_cyl[1])).norm();
                diff_z = abs(p3->c_cyl[2] - p4->c_cyl[2]) - (p3->h + p4->h)/2;
                if (i != j && (min_dis - cur_dis) > eps && diff_z < 0) {
                    min_index = j;
                    min_dis = cur_dis;
                }
//                if (min_dis < D_sample) break;
            }
            // 最近的这两柱子不太远则建墙
            p4 = &s.pillars[min_index];
            if(min_dis - 4*D_sample < -eps && min_dis - 2*R_c > eps) {
                if(BuildWall(&s, p3->c_cyl + Point3D(0,0,p3->h/2), p3->c_cyl - Point3D(0,0,p3->h/2),
                          p4->c_cyl + Point3D(0,0,p4->h/2), p4->c_cyl - Point3D(0,0,p4->h/2))) {
                    p3->has_wall = true;
                    p4->has_wall = true;
                }
            }
        }
    }

    // 为了做bool运算，对模型做些偏移
//    BoolOpertaionAdapter(&s);

//    // 添加底盘
//    if(Th_c>eps)
//    {
//        WallPara wall;
//        double y_center = (y_max + y_min)/2;
//        wall.th = y_max - y_min + D_sample;
//        wall.p_tl = Point3D(x_min - D_sample/2, y_center, Th_c);
//        wall.p_tr = Point3D(x_max + D_sample/2, y_center, Th_c);
//        wall.p_bl = Point3D(x_min - D_sample/2, y_center, 0);
//        wall.p_br = Point3D(x_max + D_sample/2, y_center, 0);
//        s.walls.push_back(wall);
//    }
    return s;
}

// 悬挂点和悬挂边的支撑结构生成
Supporter_Classic SupportGenerator::GeneratePEClassicSupporter()
{
    Supporter_Classic s;

    // 支撑柱Map,记录每个采样区域要建支撑的悬挂点和悬挂边的支柱
    vector<Point3D> pillar_map;

    // 悬挂点检测
    OverhangPointDetection();
    // 遍历所有悬挂点，建立支撑结构
    MyMesh::VertexIter v_it, v_end(mesh.vertices_end());
    for (v_it = mesh.vertices_begin(); v_it != v_end; ++v_it) {
        if (mesh.data(*v_it).overhang() == true) {
            Point3D p = Point3D(mesh.point(*v_it).data());
            pillar_map.push_back(p);
        }
    }

    // 悬挂边检测
    OverhangEdgeDetection();

    // 遍历所有悬挂边，建立支撑结构
    MyMesh::EdgeIter e_it, e_end(mesh.edges_end());
    MyMesh::HalfedgeHandle he;
    Point3D v0, v1, dir, p;
    double t0, t1, t;
    int axis = 0;
    for (e_it = mesh.edges_begin(); e_it != e_end; ++e_it) {
        if (mesh.data(*e_it).overhang() == true) {
            he = mesh.halfedge_handle(e_it, 0);
            v0 = Point3D(mesh.point(mesh.from_vertex_handle(he)).data());
            v1 = Point3D(mesh.point(mesh.to_vertex_handle(he)).data());
            dir = v1 - v0;
            // 找一个斜率大的方向采样柱子
            if (abs(dir[0]) > abs(dir[1])) axis = 0;
            else axis = 1;
            // 在x或y方向按照采样距离的一般采样柱子
            t0 = (v0[axis] - (axis?y_start:x_start)) / D_sample_edge;
            t1 = (v1[axis] - (axis?y_start:x_start)) / D_sample_edge;
            if(t0 > t1) {
                swap(t0,t1);
                swap(v0,v1);
                dir = -dir;
            }
            if(ceil(t0) > floor(t1)) continue;
            for(int i = ceil(t0); i <= floor(t1); i++) {
                t = (((axis?y_start:x_start) + i*D_sample_edge) - v0[axis]) / dir[axis];
                p = v0 + t*dir;
                pillar_map.push_back(p);
            }
        }
    }

    progress_div = 1.0/(pillar_map.size() + n_x*n_y);
    for (int i = 0; i < pillar_map.size(); i++) {
        vector<double> dis;
        vector<Point3D> norm;
        // 在悬挂点向下射线上寻找一个最近的点，以该点做半径为(R_tc+R_b)的球与模型相切
        // 由该点即可做一个垂直于模型表面的球和台来支撑柱子
        Perpendicular(pillar_map[i], Point3D(0, 0, -1), dis, norm);
        // 建造悬挂点的支撑结构
        BuildPillarOP(&s, pillar_map[i], dis, norm);
        progress += progress_div;
    }

    return s;
}

// 悬挂面支撑结构的生成
Supporter_Classic SupportGenerator::GenerateFacetClassicSupporter(vector<Supporter_Classic> &support_map)
{
    Supporter_Classic s;

    vector<vector<double>> dis_map;
    // 计算每个网格格点上的支撑结构,存储在一个表中
	for (int i = 0; i < n_x; i++) {
		for (int j = 0; j < n_y; j++) {
			vector<double> dis;
            vector<Point3D> norm;
            Supporter_Classic sc;

            Point3D p_sample = Point3D(x_start + i*D_sample, y_start + j*D_sample, 0);

            // 在由采样点向上的射线上寻找所有的点，以这些点做半径为(R_tc+R_b)的球与模型相切
            // 这些点即可做一个垂直于模型表面的球和台的支撑结构，中间由柱子连接
            Perpendicular (p_sample, Point3D(0, 0, 1), dis, norm);
            dis_map.push_back(dis);
            // 根据由上面计算的结果建造悬挂面上每个采样点的支撑结构
            BuildPillarOF (&sc, p_sample, dis, norm);
            support_map.push_back(sc);
            progress += progress_div;
		}
	}

    double x_cur, y_cur;
    PillarPara *p1, *p2;
    for (int i = 0; i < n_x; i++) {
        for (int j = 0; j < n_y; j++) {
            // 当前采样网格点位置
            x_cur = x_start + i*D_sample;
            y_cur = y_start + j*D_sample;
            // 遍历采样点上的所有柱子，在两个柱子间建造墙
            for (int k = 0; k < support_map[n_y*i + j].pillars.size(); k++) {
                // 建造墙
                p1 = &support_map[n_y*i + j].pillars[k];
                if (i > 0) {
                    // 若不是x方向第一个采样点，沿x负方向建造墙
                    for (int v = 0; v < support_map[n_y*(i - 1) + j].pillars.size(); v++) {
                        // 求取当前柱子与x负方向最邻近的采样点上所有柱子之间的墙
                        // 其中三目运算符是由于最低的柱子的支撑点在底盘上
                        p2 = &support_map[n_y*(i - 1) + j].pillars[v];
                        if(BuildWall(&s, p1->c_cyl + Point3D(0,0,p1->h/2), p1->c_cyl - Point3D(0,0,p1->h/2),
                            p2->c_cyl + Point3D(0,0,p2->h/2), p2->c_cyl - Point3D(0,0,p2->h/2))) {
                            p1->has_wall = true;
                            p2->has_wall = true;
                        }
                    }
                }
                if (j > 0) {
                    // 若不是y方向第一采样点点，沿y负方向建造墙
                    for (int v = 0; v < support_map[n_y*i + j - 1].pillars.size(); v++) {
                        // 求取当前柱子与y负方向最邻近的采样点上所有柱子之间的墙
                        p2 = &support_map[n_y*i + j - 1].pillars[v];
                        if(BuildWall(&s, p1->c_cyl + Point3D(0,0,p1->h/2), p1->c_cyl - Point3D(0,0,p1->h/2),
                            p2->c_cyl + Point3D(0,0,p2->h/2), p2->c_cyl - Point3D(0,0,p2->h/2))) {
                            p1->has_wall = true;
                            p2->has_wall = true;
                        }
                    }
                }
            }
        }
    }

    // 合并所有支撑结构
    for (int i = 0; i < n_x; i++)
        for (int j = 0; j < n_y; j++)
            s.pillars.insert(s.pillars.end(), support_map[n_y*i + j].pillars.begin(), support_map[n_y*i + j].pillars.end());

	return s;
}


// 为悬挂点建造柱子支撑结构
void SupportGenerator::BuildPillarOP(Supporter_Classic *s, Point3D p_overhang, vector<double> &dis, vector<Point3D> &norm)
{
    // 柱子结构部件以及默认值初始化，从上至下生成球A、圆台B、圆柱C、圆台E、球F
    PillarPara pillar;
    Point3D cyl_top, cyl_bottom;

    pillar.has_wall = false;
    pillar.r_ball_top = R_b;
    pillar.r_ball_bottom = 0;
    pillar.r_cyl = R_c;
    pillar.c_ball_top = p_overhang - Point3D(0, 0, R_b);
    pillar.d_tc_top = Point3D(0,0,1);

    cyl_top = pillar.c_ball_top - Point3D(0, 0, H_tc);
    pillar.c_ball_bottom = Point3D(p_overhang[0], p_overhang[1], 0);    // 底部初始化在zplane上
    cyl_bottom = Point3D(p_overhang[0], p_overhang[1], -R_b);           // 向下偏移R_b,便于区分底部是否直接在打印底盘上

    Point3D normal;   // 当前面片的边和法向

    for (int i = 0; i < dis.size(); i++) {
        normal = norm[i];
        // 由最近的法向朝上的面片支撑,计算各支撑部件参数
        if (normal[2] > eps) {
            pillar.d_tc_bottom = -normal;
            cyl_bottom = p_overhang + dis[i]*Point3D(0,0,-1);
            pillar.c_ball_bottom = cyl_bottom - normal*H_tc;
            pillar.r_ball_bottom = R_b;
            break;
        }
    }

    pillar.c_cyl = (cyl_top + cyl_bottom)/2;
    pillar.h = cyl_top[2] - cyl_bottom[2] - 2*R_b;    // 圆柱高度为了斜的圆台能正常绘制做一个偏移

    if(pillar.h > eps && !PillarCollisonDetection(pillar))
        s->pillars.push_back(pillar);
}

// 为悬挂面建造柱子支撑结构
void SupportGenerator::BuildPillarOF(Supporter_Classic *s, Point3D p_sample, vector<double> &dis, vector<Point3D> &norm)
{
    // 柱子结构部件以及默认值初始化，从上至下生成球A、圆台B、圆柱C、圆台E、球F
    PillarPara pillar;
    Point3D cyl_top, cyl_bottom;

    pillar.has_wall = false;
    pillar.r_ball_top = R_b;
    pillar.r_ball_bottom = 0;
    pillar.r_cyl = R_c;

    pillar.c_ball_bottom = p_sample;            // 底部初始化在zplane上
    cyl_bottom = p_sample - Point3D(0,0,R_b);   // 向下偏移R_b,便于区分底部是否直接在打印底盘上

    if (dis.size() < 0) return;     // 如果该采样点上没有需要支撑的，则无需设计支撑结构

    Point3D normal;                 // 每个面片的法向
    bool has_supported = true;      // 是否已经有柱子来支撑需要支撑的球和圆台

    for (int i = 0; i < dis.size(); i++) {
        // 求取面法向
        normal = norm[i];
        if (normal[2] > eps && !has_supported) {
            // 在建造下一段支撑结构之前，将前一段支撑结构数据加入链表
            pillar.c_cyl = (cyl_top + cyl_bottom)/2;
            pillar.h = cyl_top[2] - cyl_bottom[2] - 2*R_b;    // 圆柱高度为了斜的圆台能正常绘制做一个偏移
            if(pillar.h > eps && !PillarCollisonDetection(pillar))
                s->pillars.push_back(pillar);
            has_supported = true;

            // 法向朝上，确定底部的参数
            pillar.d_tc_bottom = -normal;
            cyl_bottom = p_sample + dis[i]*Point3D(0,0,1);
            pillar.c_ball_bottom = cyl_bottom - normal*H_tc;
            pillar.r_ball_bottom = R_b;
        } else if (normal[2] < -eps && has_supported){
            // 法向朝下，确定顶部的参数
            pillar.d_tc_top = -normal;
            cyl_top = p_sample + dis[i]*Point3D(0,0,1);
            if(cyl_top[2] - cyl_bottom[2] < D_sample/2)  continue;  // 圆柱太短丢弃
            pillar.c_ball_top = cyl_top - normal*H_tc;

            has_supported = false;
        }
    }

    // 若仍有圆台和球需要支撑，添加支撑结构
    if(!has_supported) {
        pillar.c_cyl = (cyl_top + cyl_bottom)/2;
        pillar.h = cyl_top[2] - cyl_bottom[2] - 2*R_b;    // 圆柱高度为了斜的圆台能正常绘制做一个偏移
        if(pillar.h > eps && !PillarCollisonDetection(pillar))
            s->pillars.push_back(pillar);
    }
}

/* 在两根柱子间建造墙
 * 参数:支撑结构（返回），模型网格，柱1上下底面中心，柱2上下底面中心 */
bool SupportGenerator::BuildWall(Supporter_Classic *s, Point3D c1_t, Point3D c1_b, Point3D c2_t, Point3D c2_b)
{
	WallPara wall;
	wall.th = Th_w;

	// 取较低的悬挂点的z值的最为墙上表面z值,取较高的支撑点的z值的最为墙下表面z值
    double z_top = min(c1_t[2], c2_t[2]);
    double z_buttom = max(c1_b[2], c2_b[2]);
	if (abs(z_buttom - R_b - H_tc) < eps) z_buttom = 0;    // 若支撑点在打印地盘上，墙直接从打印底盘上做起

	// 如果墙上沿比下沿还低无需建墙
    if ((z_top - z_buttom) < -eps) return false;

    wall.p_tl = Point3D(c1_t[0], c1_t[1], z_top);
    wall.p_tr = Point3D(c2_t[0], c2_t[1], z_top);
    wall.p_bl = Point3D(c1_t[0], c1_t[1], z_buttom);
    wall.p_br = Point3D(c2_t[0], c2_t[1], z_buttom);

	// 判断墙和模型有没有碰撞
    if(WallCollisonDetection(wall)) return false;

	s->walls.push_back(wall);
    return true;
}

// 为保证bool运算正常进行，保证各模型间无共面共线的情况出现
void SupportGenerator::BoolOpertaionAdapter(Supporter_Classic *s)
{
    // 保证墙与墙之间不相切，略微缩减墙的长度
    double offset_w = 0.875 * sqrt(3.0 / 4.0) * Th_w;

    for (int i = 0; i < s->walls.size(); i++) {
        Point3D dir = (s->walls[i].p_tr - s->walls[i].p_tl).normalized();
        s->walls[i].p_tl += dir*offset_w;
        s->walls[i].p_tr -= dir*offset_w;
        s->walls[i].p_bl += dir*offset_w;
        s->walls[i].p_br -= dir*offset_w;

        s->walls[i].p_tl -= Point3D(0,0,0.1);
        s->walls[i].p_tr -= Point3D(0,0,0.1);
        s->walls[i].p_bl += Point3D(0,0,0.1);
        s->walls[i].p_br += Point3D(0,0,0.1);
    }
}

// 判断柱子是否与模型网格碰撞(true-碰撞，false-不碰撞）
bool SupportGenerator::PillarCollisonDetection(PillarPara pillar) {
    Mesh mesh;
    TruCone cylinder(pillar.c_cyl, 1.6 * pillar.r_cyl, pillar.h);   // 柱子半径扩大1.6倍进行碰撞检测，1.6为经验参数
    cylinder.SetResolutionH(1);
    cylinder.GetTriMesh(mesh);   
    if (m_cd->IsCollidedWithMesh(mesh)) return true;
    else {
        // 再检测更高更细的碰撞
        Point3D center = (pillar.c_ball_top + pillar.c_ball_bottom)/2;
        Point3D dir = pillar.c_ball_top - pillar.c_ball_bottom;
        double h = dir.norm();
        cylinder = TruCone(center, pillar.r_ball_top, h);
        cylinder.SetDirH(dir.normalized());
        cylinder.SetResolutionH(1);
        cylinder.GetTriMesh(mesh);
        if (m_cd->IsCollidedWithMesh(mesh)) return true;
        else return false;
    }
}

// 判断墙是否与模型网格碰撞(true-碰撞，false-不碰撞）
bool SupportGenerator::WallCollisonDetection(WallPara wall) {
    Mesh mesh;
    Point3D center = (wall.p_tl + wall.p_tr + wall.p_br + wall.p_bl) / 4;
    Point3D dirAB = wall.p_tr - wall.p_tl;
    Point3D dirAC = wall.p_bl - wall.p_tl;
	Parallelepiped cube(center,dirAB.norm(),2*wall.th,dirAC.norm());    // 墙厚度扩大1.6倍进行碰撞检测，1.6为经验参数
    cube.SetDirLWH(dirAB,Point3D(0,0,1).cross(dirAB),Point3D(0,0,1));
    cube.GetTriMesh(mesh);
    if (m_cd->IsCollidedWithMesh(mesh)) return true;
    else return false;
}

// 求取由orgin沿dir方向发出的射线的点集（该点集中的点与模型网格的面片或边的距离为R）
int SupportGenerator::Perpendicular(Point3D origin, Point3D dir, vector<double> &dis, vector<Point3D> &norm)
{
    double R = H_tc + R_b;

    // 求取由orgin沿dir方向发出的射线上的点Pt,该点到当前面距离为R
    Point3D A, B, C;
    Point3D dirAB, dirAC, normal;   // 每个面片的边和法向
    Point3D dirOA;  // 射线origin到三角形一点的向量
    Point3D dirAP;  // A到垂点的向量
    Point3D P_per;  // 垂点
    double t;       // 直线上一点 Pt = Po + t*dir
    double dotdn;   // 直线与面法向的内积
    double dot00, dot01, dot02, dot11, dot12, invDenom, u, v;   // 判断点在三角形内所用变量
    bool facet_supported;

    MyMesh::FaceIter f_it, f_end(mesh.faces_end());
    MyMesh::FaceVertexIter fv_it;
    MyMesh::FaceEdgeIter fe_it;
    for (f_it = mesh.faces_begin(); f_it != f_end; ++f_it) {
        facet_supported = false;
        fv_it = mesh.fv_iter(*f_it);
        A = Point3D(mesh.point(*(fv_it++)).data());
        B = Point3D(mesh.point(*(fv_it++)).data());
        C = Point3D(mesh.point(*(fv_it)).data());
        dirOA = A - origin;
        dirAB = B - A;
        dirAC = C - A;
        normal = Point3D(mesh.normal(*f_it).data());
        // 求取该直线上到当前面距离为R的一点(Pt = Po + t*dir)     
        dotdn = dir.dot(normal);
        // 根据点到直线面距离公式计算点的位置
        if (abs(dotdn) > eps) t = (R*normal.norm() + dirOA.dot(normal)) / dotdn;
        else continue;

        if (t > eps) {
            P_per = origin + t*dir - R*normal;
            // 判断垂点是否在三角形内(重心法）
            dirAP = P_per - A;

            dot00 = dirAC.dot(dirAC);
            dot01 = dirAC.dot(dirAB);
            dot02 = dirAC.dot(dirAP);
            dot11 = dirAB.dot(dirAB);
            dot12 = dirAB.dot(dirAP);

            invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
            u = (dot11 * dot02 - dot01 * dot12) * invDenom;
            v = (dot00 * dot12 - dot01 * dot02) * invDenom;

            // 垂点在三角形内则添加到链表中
            if((u >= 0) && (v >= 0) && (u + v <= 1)) {
                facet_supported = true;
                dis.push_back(t);
                norm.push_back(normal);
            }
        }

        // 当需要被支撑的面没有被支撑的时候需要支撑边
        for (fe_it = mesh.fe_iter(*f_it); fe_it; ++fe_it)
            mesh.data(*fe_it).set_overhang(!facet_supported);
    }

    // 求取由orgin沿dir方向发出的射线上的点Pt,该点到当前边距离为R
    MyMesh::HalfedgeHandle he;  // 当前半边
    Point3D v0, v1;             // 当前半边起止点
    Point3D dirL, dirLn;        // 边的方向, 和归一化的方向
    Point3D dirAPt1, dirAPt2;   // 边起点到直线上求解得点Pt的方向
    Point3D norm1, norm2;       // 边上垂点到直线上求解得点Pt的方向(圆台法向)
    double t1, t2, l1, l2;
    MyMesh::FaceHandle f1, f2;  // 边相邻的两个面
    Point3D normal_avg;         // 两边法向的均值

    // 解方程中间变量
    Point3D a,b;
    double dotab, dotaa, dotbb, delta_4;

    MyMesh::EdgeIter e_it, e_end(mesh.edges_end());
    for (e_it = mesh.edges_begin(); e_it != e_end; ++e_it) {
        // 为需要支撑的边提供支撑，只需考虑一半的半边（起点索引<终点索引）
        if (mesh.data(*e_it).overhang() == true) {
            he = mesh.halfedge_handle(e_it, 0);
            v0 = Point3D(mesh.point(mesh.from_vertex_handle(he)).data());
            v1 = Point3D(mesh.point(mesh.to_vertex_handle(he)).data());
            dirL = v1 - v0;
            dirLn = dirL.normalized();
            dirOA = v0 - origin;
            // 计算Pt点的位置，R = |APt × L|/|L| => |(AO + t*dir) × L| = R => |a + t*b| = R
            a = (-dirOA).cross(dirLn);
            b = dir.cross(dirLn);
            dotab = a.dot(b);
            dotaa = a.dot(a);
            dotbb = b.dot(b);
            delta_4 = dotab*dotab - dotbb*(dotaa - R*R);
            if (delta_4 >= 0 && dotbb > eps) {
                // 计算t,并判断垂点是否在线段内,且判断Pt是否在平面正方向上
                t1 = (-dotab + sqrt(delta_4)) / dotbb;
                t2 = (-dotab - sqrt(delta_4)) / dotbb;
                dirAPt1 = t1*dir - dirOA;
                dirAPt2 = t2*dir - dirOA;
                l1 = dirAPt1.dot(dirLn);
                l2 = dirAPt2.dot(dirLn);
                norm1 = (dirAPt1 - l1*dirLn).normalized();
                norm2 = (dirAPt2 - l2*dirLn).normalized();

                f1 = mesh.face_handle(he);
                f2 = mesh.face_handle(mesh.opposite_halfedge_handle(he));
                if (!f1.is_valid() || !f2.is_valid()) continue;
                normal_avg = Point3D((mesh.normal(f1) + mesh.normal(f2)).data())/2;

                // 圆台法向需要与边相邻的两个面有较大夹角，也就是与这两个的面的平均法向相接近
                if (l1 > 0 && l1 < dirL.norm() && t1 > eps && norm1.dot(normal_avg) > 0.8) {
                    dis.push_back(t1);
                    norm.push_back(norm1);
                }
                if (l2 > 0 && l2 < dirL.norm() && t2 > eps && norm2.dot(normal_avg) > 0.8) {
                    dis.push_back(t2);
                    norm.push_back(norm2);
                }
            }
        }
    }



    if (dis.size() < 2) return dis.size();
    // 排序
    for (int i = 0; i < dis.size() - 1; i++) {
        for (int j = 0; j < dis.size() - i - 1; j++) {
            if (dis[j + 1] < dis[j]) {
                swap(dis[j], dis[j + 1]);
                swap(norm[j], norm[j + 1]);
            }
        }
    }

    return dis.size();
}
