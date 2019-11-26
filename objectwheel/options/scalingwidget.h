#ifndef SCALINGWIDGET_H
#define SCALINGWIDGET_H

#include <QGroupBox>

class QVBoxLayout;
class QRadioButton;

class ScalingWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit ScalingWidget(QWidget* parent = nullptr);

public slots:
    void charge();

private slots:
    void saveTheme();

private:
    QVBoxLayout* m_layout;
    QRadioButton* m_highDpiScalingButton;
    QRadioButton* m_noScalingButton;
};

#endif // SCALINGWIDGET_H