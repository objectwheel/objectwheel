#ifndef PAGESWITCHERPANE_H
#define PAGESWITCHERPANE_H

#include <QWidget>

class FlatButton;
class QVBoxLayout;

class PageSwitcherPane : public QWidget
{
        Q_OBJECT

    public:
        explicit PageSwitcherPane(QWidget *parent = nullptr);

    public slots:
        void reset();

    protected:
        void paintEvent(QPaintEvent *event) override;

    private:
        QVBoxLayout* m_layout;
        FlatButton* m_qmlCodeEditorButton;
        FlatButton* m_designerButton;
        FlatButton* m_projectSettingsButton;
        FlatButton* m_buildsButton;
        FlatButton* m_documentsButton;
        FlatButton* m_splitViewButton;

};

#endif // PAGESWITCHERPANE_H