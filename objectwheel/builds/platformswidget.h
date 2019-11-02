#ifndef PLATFORMSWIDGET_H
#define PLATFORMSWIDGET_H

#include <global.h>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

class PlatformsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PlatformsWidget(QWidget *parent = nullptr);

signals:
    void platformSelected(OTargets::Targets);

private slots:
    void handleBtnNextClicked();

private:
    QVBoxLayout _layout;
    QLabel _lblLogo;
    QLabel _lblTitle;
    QLabel _lblMsg;
    QListWidget _listWidget;
//  FIXME  FlarButton _btnNext;
};

#endif // PLATFORMSWIDGET_H
