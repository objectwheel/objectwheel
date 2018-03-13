#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include <QWidget>

class DesignerWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit DesignerWidget(QWidget* parent = nullptr);

    public slots:
        void reset();
        void onControlClick(Control*);
        void onControlDoubleClick(Control*);
        void onControlDrop(Control*, const QPointF&, const QString&);

    protected:
        void paintEvent(QPaintEvent *event) override;

    private slots:
        void onFitButtonClick();
        void onUndoButtonClick();
        void onRedoButtonClick();
        void onClearButtonClick();
        void onRefreshButtonClick();
        void onOutlineButtonClick(bool value);
        void onSnappingButtonClick(bool value);
        void onZoomLevelChange(const QString& text);


    private:
        void scaleScene(qreal ratio);

    private:
        qreal m_lastScale;
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
        QComboBox* m_zoomlLevelCombobox;
};

#endif // DESIGNERWIDGETm_H
