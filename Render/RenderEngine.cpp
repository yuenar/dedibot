/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "renderengine.h"
#include "Data/ModelData.h"
#include "Data/ProjectData.h"
#include "Data/MeshList.h"
using namespace std;

RenderEngine::RenderEngine()
    : boxIndexBuf(QOpenGLBuffer::IndexBuffer)
    , axisIndexBuf(QOpenGLBuffer::IndexBuffer)
    , boxHalfL(50), boxHalfW(50), boxH(100)
    , Language(0)
{
    // 设置平行光的参数
    mDirLight.Ambient  = QVector4D(0.2f, 0.2f, 0.2f, 1.0f);
    mDirLight.Diffuse  = QVector4D(0.8f, 0.8f, 0.8f, 1.0f);
    mDirLight.Specular = QVector4D(1.0f, 1.0f, 1.0f, 1.0f);
    mDirLight.Direction = QVector3D(0.0f, 0.0f, -1.0f);
}

RenderEngine::~RenderEngine()
{
    // 释放模型和支撑结构的Buffer
    for (int i = 0; i < vaos.size(); i++) ReleaseOneMeshBuffer(i);
    // 释放线框盒的Buffer
    boxArrayBuf.destroy();
    boxIndexBuf.destroy();
    boxVAO.destroy();
    // 释放方向盒的Buffer
    axisArrayBuf.destroy();
    axisIndexBuf.destroy();
    axisVAO.destroy();
}

void RenderEngine::SetProjection(QMatrix4x4 proj) {projection = proj;}
void RenderEngine::SetViewRTS(QQuaternion rot, QVector3D trans, qreal s) {
    rotation = rot;
    translation = trans;
    scale = s;
}
void RenderEngine::SetViewHW(qreal h, qreal w) {
    viewHeight = h;
    viewWidth = w;
}
void RenderEngine::SetClipPlane(QVector4D plane) {clipPlane = plane;}
void RenderEngine::ClearBuffers() {glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);}
void RenderEngine::SetLanguage(int lang) {
    int tmp = lang;
    Language = lang;
}
void RenderEngine::SetPrinterBox(qreal l, qreal w, qreal h) {
    boxHalfL = l;
    boxHalfW = w;
    boxH = h;
}

bool RenderEngine::InitRenderEngine()
{
    initializeOpenGLFunctions();

    if (!InitShaders()) return false;

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);

    // 初始化线框盒和方向盒的数据和Buffer
    InitWireFrameBox();
    InitAxisCube();

    // 传递光照参数到shader中
    lightingProgram.bind();
    lightingProgram.setUniformValue("light.Ambient", mDirLight.Ambient);
    lightingProgram.setUniformValue("light.Diffuse", mDirLight.Diffuse);
    lightingProgram.setUniformValue("light.Specular", mDirLight.Specular);
    lightingProgram.setUniformValue("light.Direction", mDirLight.Direction);
    lightingProgram.setUniformValue("eyePos", 0.0f, 0.0f, 0.0f);

    return true;
}

bool RenderEngine::InitShaders()
{
    if (!lightingProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Render/lighting.vert")) return false;
    if (!lightingProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Render/lighting.frag")) return false;
    if (!lightingProgram.link()) return false;

    if (!wireFrameProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Render/wireframe.vert")) return false;
    if (!wireFrameProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Render/wireframe.frag")) return false;
    if (!wireFrameProgram.link()) return false;

    if (!textureProgram.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/Render/texture.vert")) return false;
    if (!textureProgram.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/Render/texture.frag")) return false;
    if (!textureProgram.link()) return false;

    return true;
}

void RenderEngine::DrawModels()
{
	// 删除废弃不用的Buffer数据
	vector<int> nullpos;
	MeshList::GetInstance()->GetNullPos(nullpos);
	for(int i=0; i < nullpos.size(); i++) 
		ReleaseOneMeshBuffer(nullpos[i]);
	for(int i=MeshList::GetInstance()->GetSize();i<vaos.size();i++)
		ReleaseOneMeshBuffer(i);

    lightingProgram.bind(); // 使用带光照的shader

    int size = ProjectData::GetInstance()->GetSize();
    ModelData* md;
    int id_m, id_s, id_max, id_max_cur;

    for (int i = 0; i < size; i++)
    {
        md = ProjectData::GetInstance()->GetModelData(i);
        id_m = md->GetModelId();
        id_s = md->GetSupportId();

        // 获取模型的变换矩阵，将其转换为QMatrix4x4
        QMatrix4x4 transformMatrix;
        double *m_ptr = md->GetTransformMatrix().data();
        float *tm_ptr = transformMatrix.data();
        for(int j = 0;j < 16; j++) tm_ptr[j] = m_ptr[j];

        // 根据Mesh ID重新设置Buffer数组的大小
        id_max = max(id_m, id_s);
        id_max_cur = vaos.size() - 1;
        if(id_max > id_max_cur) {
            vertexBuffers.resize(id_max+1);
            indexBuffers.resize(id_max+1);
            vaos.resize(id_max+1);
        }

        // 绘制模型和支撑结构数据，若第一次绘制则需要初始化咯Buffer数据
        if (id_m >= 0) {
            if (vaos[id_m] == nullptr) InitOneMesh(id_m, md->GetModelMesh());
            if (md->IsModelVisible())
                DrawOneMesh(id_m, md->GetModelMesh()->indices.size(),  md->GetModelColor(), transformMatrix);
        }
        if (id_s >= 0) {
            if (vaos[id_s] == nullptr) InitOneMesh(id_s, md->GetSupportMesh());
            if (md->IsSupportVisible())
				DrawOneMesh(id_s,md->GetSupportMesh()->indices.size(),md->GetSupportColor(),transformMatrix);
        }
    }

}

// 初始化一个Mesh的数据(VAO,VBOs)
// 参数：id-Mesh的ID，mesh-Mesh
void RenderEngine::InitOneMesh(int id, const Mesh *mesh)
{
    int pn = mesh->points.size();
    int fn = mesh->indices.size();

    vertexBuffers[id] = new QOpenGLBuffer();
    indexBuffers[id] = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    vaos[id] = new QOpenGLVertexArrayObject();

    vertexBuffers[id]->create();
    vertexBuffers[id]->setUsagePattern(QOpenGLBuffer::StaticDraw);
    indexBuffers[id]->create();
    indexBuffers[id]->setUsagePattern(QOpenGLBuffer::StaticDraw);
    vaos[id]->create();

    vaos[id]->bind();

    // 将double转换为float，int转换为uint增加渲染效率
    std::vector<QVector3D> vertexs;
    for (int i = 0; i < pn;i++)
        vertexs.push_back(QVector3D(mesh->points[i][0], mesh->points[i][1], mesh->points[i][2]));

    vertexBuffers[id]->bind();
    vertexBuffers[id]->allocate(vertexs.data(), pn*sizeof(QVector3D));
    indexBuffers[id]->bind();
    indexBuffers[id]->allocate(mesh->indices.data(), fn*sizeof(Index3D));

    vertexBuffers[id]->bind();
    lightingProgram.enableAttributeArray(0);
    lightingProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));

    vaos[id]->release();
}

// 用光照模型绘制一个Mesh
// 参数：id-Mesh的ID，size-顶点数，color-材质颜色, transformMatrix-模型矩阵
void RenderEngine::DrawOneMesh(int id, int size, Point3D color, QMatrix4x4 transformMatrix)
{
    vaos[id]->bind();

    // 计算模型的模型视图矩阵
    QMatrix4x4 modelview;
    modelview.translate(translation);
    modelview.rotate(rotation);
    modelview.scale(scale);
    modelview *= transformMatrix;

    // 传递各矩阵参数给Shader
    lightingProgram.setUniformValue("mvp_matrix", projection * modelview);
    lightingProgram.setUniformValue("mv_matrix", modelview);
    lightingProgram.setUniformValue("trans_matrix", transformMatrix);
    // 传递各Mesh材质参数给Shader
    lightingProgram.setUniformValue("mat.Ambient", color[0], color[1], color[2], 1.0);
    lightingProgram.setUniformValue("mat.Diffuse", color[0], color[1], color[2], 1.0);
    lightingProgram.setUniformValue("mat.Specular", 0.5f, 0.5f, 0.5f, 16.0);
    // 传递各截面参数给Shader
    lightingProgram.setUniformValue("clipPlane", clipPlane);

    glDrawElements(GL_TRIANGLES, 3*size, GL_UNSIGNED_INT, 0);
    vaos[id]->release();
}

// 释放一个Mesh的Buffer
// 参数：id-Mesh的ID
void RenderEngine::ReleaseOneMeshBuffer(int id) {
	if(id>=vertexBuffers.size())return;
    if (vertexBuffers[id] != nullptr) vertexBuffers[id]->destroy();
    if (indexBuffers[id] != nullptr) indexBuffers[id]->destroy();
    if (vaos[id] != nullptr) vaos[id]->destroy();
    vertexBuffers[id] = nullptr;
    indexBuffers[id] = nullptr;
    vaos[id] = nullptr;
}

// 初始化线框盒（打印机箱体）的数据和Buffer
void RenderEngine::InitWireFrameBox()
{
	const double* boxSize=ProjectData::GetInstance()->GetBoxSize();
	boxHalfL=boxSize[0]/2;
	boxHalfW=boxSize[1]/2;
	boxH=boxSize[2];
    QVector3D vertices[] = {
        QVector3D(-boxHalfL, -boxHalfW, 0.0f),    // v0
        QVector3D(-boxHalfL,  boxHalfW, 0.0f),    // v1
        QVector3D( boxHalfL,  boxHalfW, 0.0f),    // v2
        QVector3D( boxHalfL, -boxHalfW, 0.0f),    // v3
        QVector3D(-boxHalfL, -boxHalfW, boxH),   // v4
        QVector3D(-boxHalfL,  boxHalfW, boxH),   // v5
        QVector3D( boxHalfL,  boxHalfW, boxH),   // v6
        QVector3D( boxHalfL, -boxHalfW, boxH),   // v7

        QVector3D(-boxHalfL*0.75f, -boxHalfW, 0.0f),    // v8
        QVector3D(-boxHalfL*0.75f,  boxHalfW, 0.0f),    // v9
        QVector3D(-boxHalfL*0.50f, -boxHalfW, 0.0f),    // v10
        QVector3D(-boxHalfL*0.50f,  boxHalfW, 0.0f),    // v11
        QVector3D(-boxHalfL*0.25f, -boxHalfW, 0.0f),    // v12
        QVector3D(-boxHalfL*0.25f,  boxHalfW, 0.0f),    // v13
        QVector3D(           0.00f, -boxHalfW, 0.0f),    // v14
        QVector3D(           0.00f,  boxHalfW, 0.0f),    // v15
        QVector3D( boxHalfL*0.25f, -boxHalfW, 0.0f),    // v16
        QVector3D( boxHalfL*0.25f,  boxHalfW, 0.0f),    // v17
        QVector3D( boxHalfL*0.50f, -boxHalfW, 0.0f),    // v18
        QVector3D( boxHalfL*0.50f,  boxHalfW, 0.0f),    // v19
        QVector3D( boxHalfL*0.75f, -boxHalfW, 0.0f),    // v20
        QVector3D( boxHalfL*0.75f,  boxHalfW, 0.0f),    // v21
        QVector3D( boxHalfL, -boxHalfW*0.75f, 0.0f),    // v22
        QVector3D(-boxHalfL, -boxHalfW*0.75f, 0.0f),    // v23
        QVector3D( boxHalfL, -boxHalfW*0.50f, 0.0f),    // v24
        QVector3D(-boxHalfL, -boxHalfW*0.50f, 0.0f),    // v25
        QVector3D( boxHalfL, -boxHalfW*0.25f, 0.0f),    // v26
        QVector3D(-boxHalfL, -boxHalfW*0.25f, 0.0f),    // v27
        QVector3D( boxHalfL,           0.00f, 0.0f),    // v28
        QVector3D(-boxHalfL,           0.00f, 0.0f),    // v29
        QVector3D( boxHalfL,  boxHalfW*0.25f, 0.0f),    // v30
        QVector3D(-boxHalfL,  boxHalfW*0.25f, 0.0f),    // v31
        QVector3D( boxHalfL,  boxHalfW*0.50f, 0.0f),    // v32
        QVector3D(-boxHalfL,  boxHalfW*0.50f, 0.0f),    // v33
        QVector3D( boxHalfL,  boxHalfW*0.75f, 0.0f),    // v34
        QVector3D(-boxHalfL,  boxHalfW*0.75f, 0.0f),    // v35
    };

    GLushort indices[] = {
        // 线框盒
        0, 1, 1, 2, 2, 3, 3, 0,
        4, 5, 5, 6, 6, 7, 7, 4,
        0, 4, 1, 5, 2, 6, 3, 7,
        // 底面网格
         8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21,
        22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35
    };

    boxArrayBuf.create();
    boxArrayBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
    boxIndexBuf.create();
    boxIndexBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
    boxVAO.create();

    wireFrameProgram.bind();
    boxVAO.bind();

    boxArrayBuf.bind();
    boxArrayBuf.allocate(vertices, 36 * sizeof(QVector3D));
    boxIndexBuf.bind();
    boxIndexBuf.allocate(indices, 52 * sizeof(GLushort));
    wireFrameProgram.enableAttributeArray(0);
    wireFrameProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));

    boxVAO.release();
}

// 用线框Shader绘制线框盒（打印机箱体）
void RenderEngine::DrawWireFrameBox()
{
    wireFrameProgram.bind();
    boxVAO.bind();

    QMatrix4x4 modelview;
    modelview.translate(translation);
    modelview.rotate(rotation);
    modelview.scale(scale);

    wireFrameProgram.setUniformValue("mvp_matrix", projection * modelview);
    wireFrameProgram.setUniformValue("color", 0.25, 0.25, 0.25, 1.0);

    glDrawElements(GL_LINES, 52, GL_UNSIGNED_SHORT, 0);
    boxVAO.release();
}

struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};

// 初始化方向盒的数据和Buffer
void RenderEngine::InitAxisCube()
{
    VertexData vertices[] = {
        // Vertex data for face 0
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.0f, 0.0f)},  // v0
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.33f, 0.0f)}, // v1
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.0f, 0.5f)},  // v2
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.33f, 0.5f)}, // v3

        // Vertex data for face 1
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D( 0.0f, 0.5f)}, // v4
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.33f, 0.5f)}, // v5
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.0f, 1.0f)},  // v6
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.33f, 1.0f)}, // v7

        // Vertex data for face 2
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.5f)}, // v8
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(1.0f, 0.5f)},  // v9
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.66f, 1.0f)}, // v10
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(1.0f, 1.0f)},  // v11

        // Vertex data for face 3
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.0f)}, // v12
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(1.0f, 0.0f)},  // v13
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.66f, 0.5f)}, // v14
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(1.0f, 0.5f)},  // v15

        // Vertex data for face 4
        {QVector3D(-1.0f, -1.0f, -1.0f), QVector2D(0.33f, 0.0f)}, // v16
        {QVector3D( 1.0f, -1.0f, -1.0f), QVector2D(0.66f, 0.0f)}, // v17
        {QVector3D(-1.0f, -1.0f,  1.0f), QVector2D(0.33f, 0.5f)}, // v18
        {QVector3D( 1.0f, -1.0f,  1.0f), QVector2D(0.66f, 0.5f)}, // v19

        // Vertex data for face 5
        {QVector3D(-1.0f,  1.0f,  1.0f), QVector2D(0.33f, 0.5f)}, // v20
        {QVector3D( 1.0f,  1.0f,  1.0f), QVector2D(0.66f, 0.5f)}, // v21
        {QVector3D(-1.0f,  1.0f, -1.0f), QVector2D(0.33f, 1.0f)}, // v22
        {QVector3D( 1.0f,  1.0f, -1.0f), QVector2D(0.66f, 1.0f)}  // v23
    };

    GLushort indices[] = {
         0,  1,  2,  3,  3,     // Face 0 - triangle strip ( v0,  v1,  v2,  v3)
         4,  4,  5,  6,  7,  7, // Face 1 - triangle strip ( v4,  v5,  v6,  v7)
         8,  8,  9, 10, 11, 11, // Face 2 - triangle strip ( v8,  v9, v10, v11)
        12, 12, 13, 14, 15, 15, // Face 3 - triangle strip (v12, v13, v14, v15)
        16, 16, 17, 18, 19, 19, // Face 4 - triangle strip (v16, v17, v18, v19)
        20, 20, 21, 22, 23      // Face 5 - triangle strip (v20, v21, v22, v23)
    };
    // 获取和设置方向盒的纹理
    axisTextureCH = new QOpenGLTexture(QImage(":/Resources/axis-ch.png").mirrored());
    axisTextureCH->setMinificationFilter(QOpenGLTexture::Nearest);
    axisTextureCH->setMagnificationFilter(QOpenGLTexture::Linear);
    axisTextureCH->setWrapMode(QOpenGLTexture::Repeat);

    axisTextureEN = new QOpenGLTexture(QImage(":/Resources/axis-en.png").mirrored());
    axisTextureEN->setMinificationFilter(QOpenGLTexture::Nearest);
    axisTextureEN->setMagnificationFilter(QOpenGLTexture::Linear);
    axisTextureEN->setWrapMode(QOpenGLTexture::Repeat);

    axisArrayBuf.create();
    axisArrayBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
    axisIndexBuf.create();
    axisIndexBuf.setUsagePattern(QOpenGLBuffer::StaticDraw);
    axisVAO.create();

    textureProgram.bind();
    axisVAO.bind();

    axisArrayBuf.bind();
    axisArrayBuf.allocate(vertices, 24 * sizeof(VertexData));
    axisIndexBuf.bind();
    axisIndexBuf.allocate(indices, 34 * sizeof(GLushort));
    textureProgram.enableAttributeArray(0);
    textureProgram.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(VertexData));
    textureProgram.enableAttributeArray(1);
    textureProgram.setAttributeBuffer(1, GL_FLOAT, sizeof(QVector3D), 2, sizeof(VertexData));

    axisVAO.release();
}

// 绘制方向盒
void RenderEngine::DrawAxisCube()
{
    textureProgram.bind();
    axisVAO.bind();
    if (Language == 0) axisTextureEN->bind();
    else axisTextureCH->bind();

    QMatrix4x4 modelview;
    modelview.translate(QVector3D(viewWidth*0.4, viewHeight*0.4, 800));
    modelview.rotate(rotation);
    modelview.scale(3);

    textureProgram.setUniformValue("mvp_matrix", projection * modelview);
    textureProgram.setUniformValue("u_texture", 0);

    glDrawElements(GL_TRIANGLE_STRIP, 34, GL_UNSIGNED_SHORT, 0);
    axisVAO.release();
}
