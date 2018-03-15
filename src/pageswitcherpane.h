#ifndef PAGESWITCHERPANE_H
#define PAGESWITCHERPANE_H

#include <QWidget>

class FlatButton;
class QVBoxLayout;

class PageSwitcherPane : public QWidget
{
        Q_OBJECT

    public:
        enum Pages {
            Builds,
            Designer,
            SplitView,
            Documents,
            QmlCodeEditor,
            ProjectOptions
        };

    public:
        explicit PageSwitcherPane(QWidget *parent = nullptr);
        void setPage(const Pages& page);
        Pages page() const;

    public slots:
        void reset();

    protected:
        void paintEvent(QPaintEvent *event) override;

    signals:
        void buildsActivated();
        void designerActivated();
        void splitViewActivated();
        void documentsActivated();
        void qmlCodeEditorActivated();
        void projectOptionsActivated();

    private:
        QVBoxLayout* m_layout;
        FlatButton* m_qmlCodeEditorButton;
        FlatButton* m_designerButton;
        FlatButton* m_projectOptionsButton;
        FlatButton* m_buildsButton;
        FlatButton* m_documentsButton;
        FlatButton* m_splitViewButton;

};

#endif // PAGESWITCHERPANE_H