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

#pragma once

#include <QMetaType>
#include <QVector>

#include "informationcontainer.h"

namespace QmlDesigner {

class Change3DViewCommand
{
    friend QDataStream &operator>>(QDataStream &in, Change3DViewCommand &command);
    friend QDebug operator <<(QDebug debug, const Change3DViewCommand &command);

public:
    Change3DViewCommand();
    explicit Change3DViewCommand(const QVector<InformationContainer> &auxiliaryChangeVector);

    QVector<InformationContainer> informationVector() const;

private:
    QVector<InformationContainer> m_informationVector;
};

QDataStream &operator<<(QDataStream &out, const Change3DViewCommand &command);
QDataStream &operator>>(QDataStream &in, Change3DViewCommand &command);

QDebug operator <<(QDebug debug, const Change3DViewCommand &command);

} // namespace QmlDesigner

Q_DECLARE_METATYPE(QmlDesigner::Change3DViewCommand)
