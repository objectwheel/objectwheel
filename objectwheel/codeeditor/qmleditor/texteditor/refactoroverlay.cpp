/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "refactoroverlay.h"
#include <qmlcodeeditor.h>

#include <QPainter>
#include <QDebug>
#include <QPlainTextDocumentLayout>

namespace TextEditor {

RefactorOverlay::RefactorOverlay(QmlCodeEditor *editor) : QObject(editor)
    , m_editor(editor)
//    , m_maxWidth(0)
    , m_icon(QIcon(QStringLiteral(":/images/lightbulb.svg")))
{
}

void RefactorOverlay::paint(QPainter *painter, const QRect &clip)
{
//    m_maxWidth = 0;
    for (int i = 0; i < m_markers.size(); ++i) {
        paintMarker(m_markers.at(i), painter, clip);
    }

//    if (auto documentLayout = qobject_cast<QPlainTextDocumentLayout*>(m_editor->document()->documentLayout()))
//        documentLayout->setRequiredWidth(m_maxWidth);

}

RefactorMarker RefactorOverlay::markerAt(const QPoint &pos) const
{
    foreach (const RefactorMarker &marker, m_markers) {
        if (marker.rect.contains(pos))
            return marker;
    }
    return RefactorMarker();
}

void RefactorOverlay::paintMarker(const RefactorMarker& marker, QPainter *painter, const QRect &clip)
{
    const QPointF offset = m_editor->contentOffset();
    const QRectF geometry = m_editor->blockBoundingGeometry(marker.cursor.block()).translated(offset);

    if (geometry.top() > clip.bottom() + 10 || geometry.bottom() < clip.top() - 10)
        return; // marker not visible

    const QRect cursorRect = m_editor->cursorRect(marker.cursor);

    QIcon icon = marker.icon;
    if (icon.isNull())
        icon = m_icon;

    const qreal sz = 16;
    const qreal x = cursorRect.right() + 15;
    const qreal y = cursorRect.top() + ((cursorRect.height() - sz) / 2.0);
    marker.rect = QRectF(x, y, sz, sz).toRect();
    icon.paint(painter, marker.rect);
//    m_maxWidth = qMax(m_maxWidth, x + actualIconSize.width() - int(offset.x()));
}

} // namespace TextEditor