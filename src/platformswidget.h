#ifndef PLATFORMSWIDGET_H
#define PLATFORMSWIDGET_H

#include <flatbutton.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

class PlatformsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit PlatformsWidget(QWidget *parent = nullptr);

    private:
        QVBoxLayout _layout;
        QLabel _lblLogo;
        QLabel _lblTitle;
        QLabel _lblMsg;
        QListWidget _listWidget;
        FlatButton _btnNext;
};

#endif // PLATFORMSWIDGET_H
