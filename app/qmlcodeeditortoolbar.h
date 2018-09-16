#ifndef QMLCODEEDITORTOOLBAR_H
#define QMLCODEEDITORTOOLBAR_H

#include <QToolBar>
#include <QPointer>

class QToolButton;
class QLabel;
class QmlCodeEditor;
class QmlCodeDocument;
class QComboBox;

class QmlCodeEditorToolBar : public QToolBar
{
    Q_OBJECT

public:
    enum DocumentAction {
        NoAction = 0x00,
        LeftAction = 0x01,
        RightAction = 0x02,
        CloseAction = 0x04,
        FileActions = 0x08,
        LineColAction = 0x10,
        ShowAction = 0x20,
        AllActions = LeftAction | RightAction | CloseAction | FileActions | LineColAction | ShowAction
    };
    enum Combo { LeftCombo, RightCombo };
    enum Scope { Global, Internal, External };

    Q_ENUM(Scope)
    Q_DECLARE_FLAGS(DocumentActions, DocumentAction)

public:
    explicit QmlCodeEditorToolBar(QmlCodeEditor* codeEditor);

    void setPinned(bool);
    void setShowed(bool);
    void setScope(Scope);
    void setScopeWide(Scope, bool);
    void setDocument(QmlCodeDocument* document);
    void setHiddenActions(DocumentActions action);

    Scope scope() const;
    QComboBox* combo(Combo) const;

public slots:
    void sweep();

private slots:
    void onPinButtonClick();
    void onShowButtonClick();
    void onClipboardDataChange();
    void onCursorPositionChange();
    void onScopeActivation(QAction*);
    void onComboActivation();

signals:
    void saved();
    void closed();
    void newFile();
    void openFile();
    void pinned(bool);
    void showed(bool);
    void scopeActivated(Scope);
    void comboActivated(Combo);

private:
    QSize sizeHint() const override;

private:
    QmlCodeEditor* m_codeEditor;
    QPointer<QmlCodeDocument> m_document;
    QList<QAction*> m_actions;
    QToolButton* m_pinButton;
    QToolButton* m_undoButton;
    QToolButton* m_redoButton;
    QToolButton* m_closeButton;
    QToolButton* m_newFileButton;
    QToolButton* m_openFileButton;
    QToolButton* m_saveButton;
    QToolButton* m_cutButton;
    QToolButton* m_copyButton;
    QToolButton* m_pasteButton;
    QToolButton* m_showButton;
    QToolButton* m_scopeButton;
    QLabel* m_lineColumnLabel;
    QComboBox* m_leftCombo;
    QComboBox* m_rightCombo;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QmlCodeEditorToolBar::DocumentActions)

#endif // QMLCODEEDITORTOOLBAR_H