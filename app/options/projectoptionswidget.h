#ifndef PROJECTOPTIONSWIDGET_H
#define PROJECTOPTIONSWIDGET_H

#include <QWidget>

class QScrollArea;
class QVBoxLayout;
class QGridLayout;
class ThemeChooserWidget;
class ScalingWidget;

class ProjectOptionsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectOptionsWidget(QWidget *parent = nullptr);

    ThemeChooserWidget* themeChooserWidget_2() const;

protected:
    QSize sizeHint() const override;

public slots:
    void charge();

signals:
    void themeChanged();

private:
    QVBoxLayout* m_layout;
    QScrollArea* m_scrollArea;
    QWidget* m_containerWidget;
    QGridLayout* m_gridLayout;
    ThemeChooserWidget* m_themeChooserWidget;
    ThemeChooserWidget* m_themeChooserWidget_2;
    ScalingWidget* m_scalingWidget;

};

#endif // PROJECTOPTIONSWIDGET_H