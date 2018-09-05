#ifndef QMLCODEEDITORTOOLBAR_H
#define QMLCODEEDITORTOOLBAR_H

#include <QToolBar>
#include <QPointer>

class QToolButton;
class QLabel;
class QmlCodeDocument;

class QmlCodeEditorToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit QmlCodeEditorToolBar(QWidget *parent = nullptr);
    void setDocument(QmlCodeDocument* document);

private:
    QPointer<QmlCodeDocument> m_document;
    QToolButton* m_pinButton;
    QToolButton* m_undoButton;
    QToolButton* m_redoButton;
    QToolButton* m_closeButton;
    QToolButton* m_saveButton;
    QToolButton* m_cutButton;
    QToolButton* m_copyButton;
    QToolButton* m_pasteButton;
    QLabel* m_lineColumnLabel;
};

#endif // QMLCODEEDITORTOOLBAR_H