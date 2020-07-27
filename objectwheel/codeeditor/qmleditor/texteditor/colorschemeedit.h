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

#pragma once

#include "colorscheme.h"
#include <fontcolorssettings.h>
#include <QDialog>
#include <QAbstractListModel>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace TextEditor {
namespace Internal {

namespace Ui { class ColorSchemeEdit; }

class FormatsModel;

class SchemeListModel : public QAbstractListModel
{
public:
    SchemeListModel(QObject* parent = 0): QAbstractListModel(parent)
    {
    }

    int rowCount(const QModelIndex& parent) const
    { return parent.isValid() ? 0 : m_colorSchemes.size(); }

    QVariant data(const QModelIndex& index, int role) const
    {
        if (role == Qt::DisplayRole)
            return m_colorSchemes.at(index.row()).name;

        return QVariant();
    }

    void removeColorScheme(int index)
    {
        beginRemoveRows(QModelIndex(), index, index);
        m_colorSchemes.removeAt(index);
        endRemoveRows();
    }

    void setColorSchemes(const QList<ColorSchemeEntry>& colorSchemes)
    {
        beginResetModel();
        m_colorSchemes = colorSchemes;
        endResetModel();
    }

    const ColorSchemeEntry& colorSchemeAt(int index) const
    { return m_colorSchemes.at(index); }

    const QList<ColorSchemeEntry>& colorSchemes() const
    { return m_colorSchemes; }

private:
    QList<ColorSchemeEntry> m_colorSchemes;
};

/*!
  A widget for editing a color scheme. Used in the Font Settings Page.
  */
class ColorSchemeEdit : public QWidget
{
    Q_OBJECT

public:
    ColorSchemeEdit(QWidget *parent = 0);
    ~ColorSchemeEdit();

    void setFormatDescriptions(const FormatDescriptions &descriptions);
    void setBaseFont(const QFont &font);
    void setReadOnly(bool readOnly);

    void setOriginalColorScheme(const ColorScheme &colorScheme)
    { m_originalScheme = colorScheme; }

    void setColorScheme(const ColorScheme &colorScheme);
    const ColorScheme &colorScheme() const;
    bool isReadOnly() const;

    bool isModified() const
    { return m_originalScheme != m_scheme; }

protected:
    QSize sizeHint() const override;

private:
    void currentItemChanged(const QModelIndex &index);
    void changeForeColor();
    void changeBackColor();
    void eraseForeColor();
    void eraseBackColor();
    void changeRelativeForeColor();
    void changeRelativeBackColor();
    void eraseRelativeForeColor();
    void eraseRelativeBackColor();
    void checkCheckBoxes();
    void changeUnderlineColor();
    void eraseUnderlineColor();
    void changeUnderlineStyle(int index);

    void updateControls();
    void updateForegroundControls();
    void updateBackgroundControls();
    void updateRelativeForegroundControls();
    void updateRelativeBackgroundControls();
    void updateFontControls();
    void updateUnderlineControls();
    void setItemListBackground(const QColor &color);
    void populateUnderlineStyleComboBox();

private:
    FormatDescriptions m_descriptions;
    ColorScheme m_originalScheme, m_scheme;
    int m_curItem = -1;
    Ui::ColorSchemeEdit *m_ui;
    FormatsModel *m_formatsModel;
    bool m_readOnly = false;
};

} // namespace Internal
} // namespace TextEditor
