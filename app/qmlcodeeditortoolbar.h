#ifndef QMLCODEEDITORTOOLBAR_H
#define QMLCODEEDITORTOOLBAR_H

#include <QToolBar>

class QToolButton;

class QmlCodeEditorToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit QmlCodeEditorToolBar(QWidget *parent = nullptr);

private:
    QToolButton* m_pinButton;
    QToolButton* m_undoButton;
    QToolButton* m_redoButton;
    QToolButton* m_closeButton;
    QToolButton* m_saveButton;
    QToolButton* m_cutButton;
    QToolButton* m_copyButton;
    QToolButton* m_pasteButton;
};

#endif // QMLCODEEDITORTOOLBAR_H