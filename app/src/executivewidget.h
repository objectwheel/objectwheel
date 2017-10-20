#ifndef EXECUTIVEWIDGET_H
#define EXECUTIVEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <flatbutton.h>
#include <global.h>

class QQmlEngine;
class QQuickWindow;

class ExecutiveWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ExecutiveWidget(QWidget *parent = nullptr);
        Skin skin() const;
        void setSkin(const Skin& skin);
        void setData(QQmlEngine* engine, QQuickWindow* window);

    public slots:
        void handleExitButtonClick();

    protected:
        void paintEvent(QPaintEvent *event) override;
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        Skin _skin;
        QVBoxLayout _layout;
        FlatButton _exitButton;
        int _x, _y;
        bool _pressed;
        QQmlEngine* _engine;
        QWidget* _containerWidget;
};

#endif // EXECUTIVEWIDGET_H
