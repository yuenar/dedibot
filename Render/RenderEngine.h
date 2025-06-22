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

#ifndef GEOMETRYENGINE_H
#define GEOMETRYENGINE_H

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector4D>
#include <QVector3D>
#include <vector>
#include "common.h"

struct DirectionalLight {
    QVector4D Ambient;
    QVector4D Diffuse;
    QVector4D Specular;
    QVector3D Direction;
};

class RenderEngine : protected QOpenGLFunctions
{
public:
    RenderEngine();
    virtual ~RenderEngine();

    bool InitRenderEngine();

    void SetProjection(QMatrix4x4 proj);
    void SetViewRTS(QQuaternion rot, QVector3D trans, qreal s);
    void SetViewHW(qreal h, qreal w);
    void SetClipPlane(QVector4D plane);
    void ClearBuffers();

    void SetLanguage(int lang);

    void SetPrinterBox(qreal l, qreal w, qreal h);

    void DrawWireFrameBox();
    void DrawAxisCube();
    void DrawModels();

private:

    bool InitShaders();

    void InitWireFrameBox();
    void InitAxisCube();
    void InitOneMesh(int id, const Mesh *mesh);
    void DrawOneMesh(int id, int size, Point3D color, QMatrix4x4 transformMatrix);
    void ReleaseOneMeshBuffer(int id);

    // Shaders
    QOpenGLShaderProgram lightingProgram;
    QOpenGLShaderProgram wireFrameProgram;
    QOpenGLShaderProgram textureProgram;

    DirectionalLight mDirLight; // 光照参数

    // Meshs的VBOs和VAO
    std::vector<QOpenGLBuffer*> vertexBuffers;
    std::vector<QOpenGLBuffer*> indexBuffers;
    std::vector<QOpenGLVertexArrayObject*> vaos;

    // 线框盒（打印机箱体）的VBOs和VAO
    QOpenGLBuffer boxArrayBuf;
    QOpenGLBuffer boxIndexBuf;
    QOpenGLVertexArrayObject boxVAO;
    // 方向盒的VBOs、VAO和纹理
    QOpenGLBuffer axisArrayBuf;
    QOpenGLBuffer axisIndexBuf;
    QOpenGLVertexArrayObject axisVAO;
    QOpenGLTexture *axisTextureCH;
    QOpenGLTexture *axisTextureEN;

    QMatrix4x4 projection;  // 投影矩阵
    QQuaternion rotation;   // 旋转矩阵
    QVector3D translation;  // 平移矩阵
    qreal scale;            // 缩放

    qreal viewHeight, viewWidth;        // 视口高度、宽度
    qreal boxHalfL, boxHalfW, boxH;     // 打印机箱体长宽高

    QVector4D clipPlane;    // 切面

    int Language;         // 语言（0-英文，1-中文）
};

#endif // GEOMETRYENGINE_H
