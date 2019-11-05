/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#ifdef QUICK3D_MODULE

#include "mousearea3d.h"

#include <QtGui/qguiapplication.h>
#include <QtQml/qqmlinfo.h>

namespace QmlDesigner {
namespace Internal {

MouseArea3D *MouseArea3D::s_mouseGrab = nullptr;

MouseArea3D::MouseArea3D(QQuick3DNode *parent)
    : QQuick3DNode(parent)
{
}

QQuick3DViewport *MouseArea3D::view3D() const
{
    return m_view3D;
}

bool MouseArea3D::hovering() const
{
    return m_hovering;
}

bool MouseArea3D::dragging() const
{
    return m_dragging;
}

bool MouseArea3D::grabsMouse() const
{
    return m_grabsMouse;
}

qreal MouseArea3D::x() const
{
    return m_x;
}

qreal MouseArea3D::y() const
{
    return m_y;
}

qreal MouseArea3D::width() const
{
    return m_width;
}

qreal MouseArea3D::height() const
{
    return m_height;
}

void MouseArea3D::setView3D(QQuick3DViewport *view3D)
{
    if (m_view3D == view3D)
        return;

    m_view3D = view3D;
    emit view3DChanged();
}

void MouseArea3D::setGrabsMouse(bool grabsMouse)
{
    if (m_grabsMouse == grabsMouse)
        return;

    m_grabsMouse = grabsMouse;
    emit grabsMouseChanged(grabsMouse);
}

void MouseArea3D::setX(qreal x)
{
    if (qFuzzyCompare(m_x, x))
        return;

    m_x = x;
    emit xChanged(x);
}

void MouseArea3D::setY(qreal y)
{
    if (qFuzzyCompare(m_y, y))
        return;

    m_y = y;
    emit yChanged(y);
}

void MouseArea3D::setWidth(qreal width)
{
    if (qFuzzyCompare(m_width, width))
        return;

    m_width = width;
    emit widthChanged(width);
}

void MouseArea3D::setHeight(qreal height)
{
    if (qFuzzyCompare(m_height, height))
        return;

    m_height = height;
    emit heightChanged(height);
}

void MouseArea3D::componentComplete()
{
    if (!m_view3D) {
        qmlDebug(this) << "property 'view3D' is not set!";
        return;
    }
    m_view3D->setAcceptedMouseButtons(Qt::LeftButton);
    m_view3D->setAcceptHoverEvents(true);
    m_view3D->setAcceptTouchEvents(false);
    m_view3D->installEventFilter(this);
}

QVector3D MouseArea3D::rayIntersectsPlane(const QVector3D &rayPos0,
                                          const QVector3D &rayPos1,
                                          const QVector3D &planePos,
                                          const QVector3D &planeNormal) const
{
    QVector3D rayDirection = rayPos1 - rayPos0;
    QVector3D rayPos0RelativeToPlane = rayPos0 - planePos;

    float dotPlaneRayDirection = QVector3D::dotProduct(planeNormal, rayDirection);
    float dotPlaneRayPos0 = -QVector3D::dotProduct(planeNormal, rayPos0RelativeToPlane);

    if (qFuzzyIsNull(dotPlaneRayDirection)) {
        // The ray is is parallel to the plane. Note that if dotLinePos0 == 0, it
        // additionally means that the line lies in plane as well. In any case, we
        // signal that we cannot find a single intersection point.
        return QVector3D(0, 0, -1);
    }

    // Since we work with a ray (that has a start), distanceFromLinePos0ToPlane
    // must be above 0. If it was a line segment (with an end), it also need to be less than 1.
    // (Note: a third option would be a "line", which is different from a ray or segment in that
    // it has neither a start, nor an end). Then we wouldn't need to check the distance at all.
    // But that would also mean that the line could intersect the plane behind the camera, if
    // the line were directed away from the plane when looking forward.
    float distanceFromRayPos0ToPlane = dotPlaneRayPos0 / dotPlaneRayDirection;
    if (distanceFromRayPos0ToPlane <= 0)
        return QVector3D(0, 0, -1);
    return rayPos0 + distanceFromRayPos0ToPlane * rayDirection;
}

QVector3D MouseArea3D::getMousePosInPlane(const QPointF &mousePosInView) const
{
    const QVector3D mousePos1(float(mousePosInView.x()), float(mousePosInView.y()), 0);
    const QVector3D mousePos2(float(mousePosInView.x()), float(mousePosInView.y()), 1);
    const QVector3D rayPos0 = m_view3D->mapTo3DScene(mousePos1);
    const QVector3D rayPos1 = m_view3D->mapTo3DScene(mousePos2);
    const QVector3D globalPlanePosition = mapPositionToScene(QVector3D(0, 0, 0));
    const QVector3D intersectGlobalPos = rayIntersectsPlane(rayPos0, rayPos1,
                                                            globalPlanePosition, forward());
    if (qFuzzyCompare(intersectGlobalPos.z(), -1))
        return intersectGlobalPos;

    return mapPositionFromScene(intersectGlobalPos);
}

bool MouseArea3D::eventFilter(QObject *, QEvent *event)
{
    switch (event->type()) {
    case QEvent::HoverMove: {
        if (m_grabsMouse && s_mouseGrab && s_mouseGrab != this)
            break;

        auto const mouseEvent = static_cast<QMouseEvent *>(event);
        const QVector3D mousePosInPlane = getMousePosInPlane(mouseEvent->pos());
        if (qFuzzyCompare(mousePosInPlane.z(), -1))
            break;

        const bool mouseOnTopOfMouseArea =
                mousePosInPlane.x() >= float(m_x) &&
                mousePosInPlane.x() <= float(m_x + m_width) &&
                mousePosInPlane.y() >= float(m_y) &&
                mousePosInPlane.y() <= float(m_y + m_height);

        const bool buttonPressed = QGuiApplication::mouseButtons().testFlag(Qt::LeftButton);

        // The filter will detect a mouse press on the view, but not a mouse release, since the
        // former is not accepted by the view, which means that the release will end up being
        // sent elsewhere. So we need this extra logic inside HoverMove, rather than in
        // MouseButtonRelease, which would otherwise be more elegant.

        if (m_hovering != mouseOnTopOfMouseArea) {
            m_hovering = mouseOnTopOfMouseArea;
            emit hoveringChanged();
        }

        if (!m_dragging && m_hovering && buttonPressed) {
            m_dragging = true;
            emit pressed(mousePosInPlane);
            emit draggingChanged();
        } else if (m_dragging && !buttonPressed) {
            m_dragging = false;
            emit released(mousePosInPlane);
            emit draggingChanged();
        }

        if (m_grabsMouse)
            s_mouseGrab = m_hovering || m_dragging ? this : nullptr;

        if (m_dragging)
            emit dragged(mousePosInPlane);

        break; }
    default:
        break;
    }

    return false;
}

}
}

#endif // QUICK3D_MODULE
