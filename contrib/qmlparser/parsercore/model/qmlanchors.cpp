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

#include "qmlanchors.h"
#include "bindingproperty.h"
#include "nodeabstractproperty.h"
#include "rewritertransaction.h"
#include "abstractview.h"

namespace QmlDesigner {


static PropertyName lineTypeToString(AnchorLineType lineType)
{
    switch (lineType) {
        case AnchorLineLeft:             return PropertyName("left");
        case AnchorLineTop:              return PropertyName("top");
        case AnchorLineRight:            return PropertyName("right");
        case AnchorLineBottom:           return PropertyName("bottom");
        case AnchorLineHorizontalCenter: return PropertyName("horizontalCenter");
        case AnchorLineVerticalCenter:   return PropertyName("verticalCenter");
        case AnchorLineBaseline:         return PropertyName("baseline");
        case AnchorLineFill:             return PropertyName("fill");
        case AnchorLineCenter:           return PropertyName("centerIn");
        default:                           return PropertyName();
    }
}

static AnchorLineType propertyNameToLineType(const PropertyName & name)
{
    if (name == "left")
        return AnchorLineLeft;
    else if (name == "top")
        return AnchorLineTop;
    else if (name == "right")
        return AnchorLineRight;
    else if (name == "bottom")
        return AnchorLineBottom;
    else if (name == "horizontalCenter")
        return AnchorLineHorizontalCenter;
    else if (name == "verticalCenter")
        return AnchorLineVerticalCenter;
    else if (name == "baseline")
        return AnchorLineVerticalCenter;
    else if (name == "centerIn")
        return AnchorLineCenter;
    else if (name == "fill")
        return AnchorLineFill;

    return AnchorLineInvalid;
}

static PropertyName marginPropertyName(AnchorLineType lineType)
{
    switch (lineType) {
        case AnchorLineLeft:             return PropertyName("anchors.leftMargin");
        case AnchorLineTop:              return PropertyName("anchors.topMargin");
        case AnchorLineRight:            return PropertyName("anchors.rightMargin");
        case AnchorLineBottom:           return PropertyName("anchors.bottomMargin");
        case AnchorLineHorizontalCenter: return PropertyName("anchors.horizontalCenterOffset");
        case AnchorLineVerticalCenter:   return PropertyName("anchors.verticalCenterOffset");
        default:                           return PropertyName();
    }
}

static PropertyName anchorPropertyName(AnchorLineType lineType)
{
    const PropertyName typeString = lineTypeToString(lineType);

    if (typeString.isEmpty())
        return PropertyName();
    else
        return PropertyName("anchors.") + typeString;
}


QmlAnchors::QmlAnchors(const QmlItemNode &fxItemNode) : m_qmlItemNode(fxItemNode)
{
}

QmlItemNode QmlAnchors::qmlItemNode() const
{
    return m_qmlItemNode;
}

bool QmlAnchors::modelHasAnchors() const
{
    return modelHasAnchor(AnchorLineLeft)
            || modelHasAnchor(AnchorLineRight)
            || modelHasAnchor(AnchorLineTop)
            || modelHasAnchor(AnchorLineBottom)
            || modelHasAnchor(AnchorLineHorizontalCenter)
            || modelHasAnchor(AnchorLineVerticalCenter)
            || modelHasAnchor(AnchorLineBaseline);
}

bool QmlAnchors::modelHasAnchor(AnchorLineType sourceAnchorLineType) const
{
    const PropertyName propertyName = anchorPropertyName(sourceAnchorLineType);

    if (sourceAnchorLineType & AnchorLineFill)
        return qmlItemNode().modelNode().hasBindingProperty(propertyName) || qmlItemNode().modelNode().hasBindingProperty("anchors.fill");

    if (sourceAnchorLineType & AnchorLineCenter)
        return qmlItemNode().modelNode().hasBindingProperty(propertyName) || qmlItemNode().modelNode().hasBindingProperty("anchors.centerIn");

    return qmlItemNode().modelNode().hasBindingProperty(anchorPropertyName(sourceAnchorLineType));
}

AnchorLine QmlAnchors::modelAnchor(AnchorLineType sourceAnchorLineType) const
{
 QPair<PropertyName, ModelNode> targetAnchorLinePair;
 if (sourceAnchorLineType & AnchorLineFill && qmlItemNode().modelNode().hasBindingProperty("anchors.fill")) {
     targetAnchorLinePair.second = qmlItemNode().modelNode().bindingProperty("anchors.fill").resolveToModelNode();
     targetAnchorLinePair.first = lineTypeToString(sourceAnchorLineType);
 } else if (sourceAnchorLineType & AnchorLineCenter && qmlItemNode().modelNode().hasBindingProperty("anchors.centerIn")) {
     targetAnchorLinePair.second = qmlItemNode().modelNode().bindingProperty("anchors.centerIn").resolveToModelNode();
     targetAnchorLinePair.first = lineTypeToString(sourceAnchorLineType);
 } else {
     AbstractProperty binding = qmlItemNode().modelNode().bindingProperty(anchorPropertyName(sourceAnchorLineType)).resolveToProperty();
     targetAnchorLinePair.first = binding.name();
     targetAnchorLinePair.second = binding.parentModelNode();
 }

 AnchorLineType targetAnchorLine = propertyNameToLineType(targetAnchorLinePair.first);

 if (targetAnchorLine == AnchorLineInvalid )
     return AnchorLine();


 return AnchorLine(QmlItemNode(targetAnchorLinePair.second), targetAnchorLine);
}

bool QmlAnchors::isValid() const
{
    return m_qmlItemNode.isValid();
}

void QmlAnchors::setAnchor(AnchorLineType sourceAnchorLine,
                          const QmlItemNode &targetQmlItemNode,
                          AnchorLineType targetAnchorLine)
{
    RewriterTransaction transaction = qmlItemNode().view()->beginRewriterTransaction(QByteArrayLiteral("QmlAnchors::setAnchor"));
    if (qmlItemNode().isInBaseState()) {
        if ((sourceAnchorLine & AnchorLineFill)
             || (sourceAnchorLine & AnchorLineCenter)) {
            removeAnchor(sourceAnchorLine);
        }

        const PropertyName propertyName = anchorPropertyName(sourceAnchorLine);
        ModelNode targetModelNode = targetQmlItemNode.modelNode();
        QString targetExpression = targetModelNode.validId();
        if (targetQmlItemNode.modelNode() == qmlItemNode().modelNode().parentProperty().parentModelNode())
            targetExpression = QLatin1String("parent");
        if (sourceAnchorLine != AnchorLineCenter && sourceAnchorLine != AnchorLineFill)
            targetExpression = targetExpression + QLatin1Char('.') + QString::fromLatin1(lineTypeToString(targetAnchorLine));
        qmlItemNode().modelNode().bindingProperty(propertyName).setExpression(targetExpression);
    }
}

bool detectHorizontalCycle(const ModelNode &node, QList<ModelNode> knownNodeList)
{
    if (knownNodeList.contains(node))
        return true;

    knownNodeList.append(node);

    static const PropertyNameList validAnchorLines({"right", "left", "horizontalCenter"});
    static const PropertyNameList anchorNames({"anchors.right", "anchors.left", "anchors.horizontalCenter"});

    foreach (const PropertyName &anchorName, anchorNames) {
        if (node.hasBindingProperty(anchorName)) {
            AbstractProperty targetProperty = node.bindingProperty(anchorName).resolveToProperty();
            if (targetProperty.isValid()) {
                if (!validAnchorLines.contains(targetProperty.name()))
                    return true;

                if (detectHorizontalCycle(targetProperty.parentModelNode(), knownNodeList))
                    return true;
            }
        }

    }

    static const PropertyNameList anchorShortcutNames({"anchors.fill", "anchors.centerIn"});
    foreach (const PropertyName &anchorName, anchorShortcutNames) {
        if (node.hasBindingProperty(anchorName)) {
            ModelNode targetNode = node.bindingProperty(anchorName).resolveToModelNode();

            if (targetNode.isValid() && detectHorizontalCycle(targetNode, knownNodeList))
                return true;
        }
    }

    return false;
}

bool detectVerticalCycle(const ModelNode &node, QList<ModelNode> knownNodeList)
{
    if (!node.isValid())
        return false;

    if (knownNodeList.contains(node))
        return true;

    knownNodeList.append(node);

    static const PropertyNameList validAnchorLines({"top", "bottom", "verticalCenter", "baseline"});
    static const PropertyNameList anchorNames({"anchors.top", "anchors.bottom", "anchors.verticalCenter", "anchors.baseline"});

    foreach (const PropertyName &anchorName, anchorNames) {
        if (node.hasBindingProperty(anchorName)) {
            AbstractProperty targetProperty = node.bindingProperty(anchorName).resolveToProperty();
            if (targetProperty.isValid()) {
                if (!validAnchorLines.contains(targetProperty.name()))
                    return true;

                if (detectVerticalCycle(targetProperty.parentModelNode(), knownNodeList))
                    return true;
            }
        }

    }

    static const PropertyNameList anchorShortcutNames({"anchors.fill", "anchors.centerIn"});
    foreach (const PropertyName &anchorName, anchorShortcutNames) {
        if (node.hasBindingProperty(anchorName)) {
            ModelNode targetNode = node.bindingProperty(anchorName).resolveToModelNode();

            if (targetNode.isValid() && detectVerticalCycle(targetNode, knownNodeList))
                return true;
        }
    }

    return false;
}

bool QmlAnchors::canAnchor(const QmlItemNode &targetModelNode) const
{
    if (!qmlItemNode().isInBaseState())
        return false;

    return false;
}

AnchorLineType QmlAnchors::possibleAnchorLines(AnchorLineType sourceAnchorLineType,
                                               const QmlItemNode &targetQmlItemNode) const
{
    if (!canAnchor(targetQmlItemNode))
        return AnchorLineInvalid;

    if (AnchorLine::isHorizontalAnchorLine(sourceAnchorLineType)) {
        if (!detectHorizontalCycle(targetQmlItemNode, QList<ModelNode>() << qmlItemNode().modelNode()))
            return AnchorLineHorizontalMask;
    }

    if (AnchorLine::isVerticalAnchorLine(sourceAnchorLineType)) {
        if (!detectVerticalCycle(targetQmlItemNode, QList<ModelNode>() << qmlItemNode().modelNode()))
            return AnchorLineVerticalMask;
    }

    return AnchorLineInvalid;
}

void QmlAnchors::removeAnchor(AnchorLineType sourceAnchorLine)
{
   RewriterTransaction transaction = qmlItemNode().view()->beginRewriterTransaction(QByteArrayLiteral("QmlAnchors::removeAnchor"));
    if (qmlItemNode().isInBaseState()) {
        const PropertyName propertyName = anchorPropertyName(sourceAnchorLine);
        if ((sourceAnchorLine & AnchorLineFill)) {
            qmlItemNode().modelNode().removeProperty("anchors.fill");
            qmlItemNode().modelNode().bindingProperty("anchors.top").setExpression(QLatin1String("parent.top"));
            qmlItemNode().modelNode().bindingProperty("anchors.left").setExpression(QLatin1String("parent.left"));
            qmlItemNode().modelNode().bindingProperty("anchors.bottom").setExpression(QLatin1String("parent.bottom"));
            qmlItemNode().modelNode().bindingProperty("anchors.right").setExpression(QLatin1String("parent.right"));

        } else if ((sourceAnchorLine & AnchorLineCenter)) {
            qmlItemNode().modelNode().removeProperty("anchors.centerIn");
            qmlItemNode().modelNode().bindingProperty("anchors.horizontalCenter").setExpression(QLatin1String("parent.horizontalCenter"));
            qmlItemNode().modelNode().bindingProperty("anchors.verticalCenter").setExpression(QLatin1String("parent.verticalCenter"));
        }

        qmlItemNode().modelNode().removeProperty(propertyName);
    }
}

void QmlAnchors::removeAnchors()
{

}

void QmlAnchors::setMargin(AnchorLineType sourceAnchorLineType, double margin) const
{
    PropertyName propertyName = marginPropertyName(sourceAnchorLineType);
    qmlItemNode().setVariantProperty(propertyName, qRound(margin));
}

static bool checkForHorizontalCycleRecusive(const QmlAnchors &anchors, QList<QmlItemNode> &visitedItems)
{
    if (!anchors.isValid())
        return false;

    return false;
}

static bool checkForVerticalCycleRecusive(const QmlAnchors &anchors, QList<QmlItemNode> &visitedItems)
{
    if (!anchors.isValid())
        return false;

    visitedItems.append(anchors.qmlItemNode());

    return false;
}

bool QmlAnchors::checkForHorizontalCycle(const QmlItemNode &sourceItem) const
{
    QList<QmlItemNode> visitedItems;
    visitedItems.append(sourceItem);

    return checkForHorizontalCycleRecusive(*this, visitedItems);
}

bool QmlAnchors::checkForVerticalCycle(const QmlItemNode &sourceItem) const
{
    QList<QmlItemNode> visitedItems;
    visitedItems.append(sourceItem);

    return checkForVerticalCycleRecusive(*this, visitedItems);
}

void QmlAnchors::removeMargin(AnchorLineType sourceAnchorLineType)
{
    if (qmlItemNode().isInBaseState()) {
        PropertyName propertyName = marginPropertyName(sourceAnchorLineType);
        qmlItemNode().modelNode().removeProperty(propertyName);
    }
}

void QmlAnchors::removeMargins()
{
    RewriterTransaction transaction = qmlItemNode().view()->beginRewriterTransaction(QByteArrayLiteral("QmlAnchors::removeMargins"));
    removeMargin(AnchorLineLeft);
    removeMargin(AnchorLineRight);
    removeMargin(AnchorLineTop);
    removeMargin(AnchorLineBottom);
    removeMargin(AnchorLineHorizontalCenter);
    removeMargin(AnchorLineVerticalCenter);
}

void QmlAnchors::fill()
{
    qmlItemNode().modelNode().bindingProperty("anchors.fill").setExpression(QLatin1String("parent"));
}

void QmlAnchors::centerIn()
{
    qmlItemNode().modelNode().bindingProperty("anchors.centerIn").setExpression(QLatin1String("parent"));
}

bool QmlAnchors::checkForCycle(AnchorLineType anchorLineTyp, const QmlItemNode &sourceItem) const
{
    if (anchorLineTyp & AnchorLineHorizontalMask)
        return checkForHorizontalCycle(sourceItem);
    else
        return checkForVerticalCycle(sourceItem);
}

} //QmlDesigner
