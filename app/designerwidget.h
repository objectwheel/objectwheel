#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include <QWidget>

class QmlCodeEditorWidget;
class DesignerScene;
class DesignerView;
class QVBoxLayout;
class QToolBar;
class QToolButton;
class QComboBox;
class Control;

class DesignerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DesignerWidget(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget* parent = nullptr);
    DesignerScene* designerScene() const;

public slots:
    void sweep();
    void refresh() { onRefreshButtonClick(); }
    void handleControlDoubleClick(Control*);
    void handleControlDrop(Control*, const QPointF&, const QString&);

protected:
    QSize sizeHint() const override;

private slots:
    void onFitButtonClick();
    void onUndoButtonClick();
    void onRedoButtonClick();
    void onClearButtonClick();
    void onRefreshButtonClick();
    void onOutlineButtonClick(bool value);
    void onSnappingButtonClick(bool value);
    void onZoomLevelChange(const QString& text);
    void onControlSelectionChange(const QList<Control*>& selectedControls);

signals:
    void hideDockWidgetTitleBars(bool);

private:
    void scaleScene(qreal ratio);

private:
    qreal m_lastScale;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    QVBoxLayout* m_layout;
    DesignerScene* m_designerScene;
    DesignerView* m_designerView;
    QToolBar* m_toolbar;
    QToolButton* m_undoButton;
    QToolButton* m_redoButton;
    QToolButton* m_clearButton;
    QToolButton* m_refreshButton;
    QToolButton* m_snappingButton;
    QToolButton* m_fitButton;
    QToolButton* m_outlineButton;
    QToolButton* m_hideDockWidgetTitleBarsButton;
    QComboBox* m_zoomlLevelCombobox;
};

#endif // DESIGNERWIDGETm_H
