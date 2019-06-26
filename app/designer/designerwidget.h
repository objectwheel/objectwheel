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
class SignalChooserDialog;

class DesignerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DesignerWidget(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget* parent = nullptr);
    DesignerScene* designerScene() const;

    qreal scalingRatio() const;

public slots:
    void discharge();
    void refresh() { onRefreshButtonClick(); }
    void onControlDoubleClick(Control*);
    void onInspectorItemDoubleClick(Control*);
    void onAssetsFileOpen(const QString& relativePath, int line, int column);
    void onDesignsFileOpen(Control* control, const QString& relativePath, int line, int column);
    void onControlDrop(Control* targetParentControl, const QString& controlRootPath, const QPointF& pos);
    void onControlSelectionChange(const QList<Control*>& selectedControls);

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

signals:
    void hideDockWidgetTitleBars(bool);
    void scalingRatioChanged();

private:
    void scaleScene(qreal ratio);

private:
    qreal m_lastScale;
    SignalChooserDialog* m_signalChooserDialog;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    QVBoxLayout* m_layout;
    DesignerScene* m_designerScene;
    DesignerView* m_designerView;
    QToolBar* m_toolBar;
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
