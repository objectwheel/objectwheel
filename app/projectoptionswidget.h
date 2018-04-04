#ifndef PROJECTOPTIONSWIDGET_H
#define PROJECTOPTIONSWIDGET_H

#include <QWidget>

class QScrollArea;
class QVBoxLayout;
class QGridLayout;
class ThemeChooserWidget;

class ProjectOptionsWidget : public QWidget
{
        Q_OBJECT
    public:
        explicit ProjectOptionsWidget(QWidget *parent = nullptr);

    protected:
        QSize sizeHint() const override;

    public slots:
        void reset();

    private:
        QVBoxLayout* m_layout;
        QScrollArea* m_scrollArea;
        QWidget* m_containerWidget;
        QGridLayout* m_gridLayout;
        ThemeChooserWidget* m_themeChooserWidget;
        ThemeChooserWidget* m_themeChooserWidget_2;

};

#endif // PROJECTOPTIONSWIDGET_H