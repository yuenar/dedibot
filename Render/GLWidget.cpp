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

#include "GLWidget.h"

#include <QMouseEvent>

#include <QtMath>
#include <cmath>

#include "Data/ProjectData.h"

GLWidget::GLWidget(QWidget *parent) :
    QOpenGLWidget(parent),
    renderer(0),
    scrollDelta(0),
    viewSize(50),
    clipdis(1000)
{
    renderer = new RenderEngine;
    setViewDirection(DefaultView);
}

GLWidget::~GLWidget()
{
    // Make sure the context is current when deleting the texture and the buffers.
    makeCurrent();
    delete renderer;
    doneCurrent();
}

// 设置截面层数
void GLWidget::setClipLayerNumber(int layer) {
    cliplayer = layer;
}

// 设置视图方向
void GLWidget::setViewDirection(ViewDir vd) {
    Point3D center=ProjectData::GetInstance()->GetFocusPoint();
    switch(vd) {
    case FrontView:
        rotate2D = QVector2D(0, -90);
        translation = QVector2D(-center[0], center[2]);break;
    case BackView:
        rotate2D = QVector2D(180, -90);
        translation = QVector2D(center[0], center[2]);break;
    case LeftView:
        rotate2D = QVector2D(-90, -90);
        translation = QVector2D(-center[1], center[2]);break;
    case RightView:
        rotate2D = QVector2D(90, -90);
        translation = QVector2D(center[1], center[2]);break;
    case UpView:
        rotate2D = QVector2D(0, 0);
        translation = QVector2D(-center[0], center[1]);break;
    case DownView:
        rotate2D = QVector2D(0, 180);
        translation = QVector2D(-center[0], -center[1]);break;
    case DefaultView:
        rotate2D = QVector2D(-20, -70);
        translation = QVector2D(0.0, viewSize/2);
    default:break;
    }
    updateRotation();
    scrollDelta = 0.4;
}

// 设置语言（0-英文，1-中文）
void GLWidget::setLanguage(int lang) {
    renderer->SetLanguage(lang);
}

void GLWidget::wheelEvent(QWheelEvent *e)
{
    // 计算缩放量
    scrollDelta +=  e->delta() / 1200.0;
    if(scrollDelta > 0.9) scrollDelta = 0.9;
    update();
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    oldMousePos = QVector2D(e->localPos());
    newMousePos = QVector2D(e->localPos());
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
     newMousePos = QVector2D(e->localPos());
     // 左键控制旋转，计算新的rotation
     if(e->buttons() & Qt::LeftButton) updateRotation();
     // 中间控制拖动，计算新的translation
     if(e->buttons() & Qt::MidButton) translation += (newMousePos - oldMousePos) / radioScreenToView;
     oldMousePos = newMousePos;
     update();
}

void GLWidget::updateRotation()
{
    qreal d = std::min(GLWidget::width(),GLWidget::height()) / 3.0;
    rotate2D = ((newMousePos - oldMousePos) / d) * 180.0 / pi + rotate2D;
    rotation = QQuaternion::fromAxisAndAngle(1, 0, 0, rotate2D[1])
            * QQuaternion::fromAxisAndAngle(0, 0, 1, rotate2D[0]);
}

void GLWidget::initializeGL()
{
    if(!renderer->InitRenderEngine()) close();
}

void GLWidget::resizeGL(int w, int h)
{
     // 计算屏幕宽高比
    aspect = qreal(w) / qreal(h ? h : 1);
    // 屏幕坐标和视口坐标的比例
    radioScreenToView = h / (2*viewSize);

    const qreal zNear = -1000.0, zFar = 1000.0;
    // 计算projection
    projection.setToIdentity();  
    projection.ortho(-viewSize*aspect, viewSize*aspect, -viewSize, viewSize, zNear, zFar);
}

void GLWidget::paintGL()
{
    renderer->SetProjection(projection);
    renderer->SetViewRTS(rotation, QVector3D(translation.x(), -translation.y(), -50.0), 1 - scrollDelta);
    renderer->SetViewHW(2*viewSize, 2*viewSize*aspect);

	double zmax=ProjectData::GetInstance()->GetZmax();
    clipdis = (cliplayer < 100.0? cliplayer/100.0*(zmax) : 10000.0);
    renderer->SetClipPlane(QVector4D(0, 0, -1.0, clipdis));

    renderer->ClearBuffers();  
    renderer->DrawWireFrameBox();
    renderer->DrawAxisCube();
    renderer->DrawModels();
}
