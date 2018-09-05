#ifndef QMLCODEEDITORTOOLBAR_H
#define QMLCODEEDITORTOOLBAR_H

#include <QToolBar>
#include <QPointer>

class QToolButton;
class QLabel;
class QmlCodeEditor;
class QmlCodeDocument;

class QmlCodeEditorToolBar : public QToolBar
{
    Q_OBJECT

public:
    explicit QmlCodeEditorToolBar(QmlCodeEditor* parent = nullptr);
    void setDocument(QmlCodeDocument* document);

public slots:
    void sweep();

private slots:
    void onCursorPositionChange();
    void onPinButtonToggle(bool pinned);
    void onShowButtonToggle(bool showed);

signals:
    void saved();
    void closed();
    void pinned(bool);
    void showed(bool);

private:
    QSize sizeHint() const override;

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
    QToolButton* m_showButton;
    QLabel* m_lineColumnLabel;
};

#endif // QMLCODEEDITORTOOLBAR_H