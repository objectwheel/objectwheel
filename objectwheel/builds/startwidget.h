#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QWidget>

class QListWidget;
class StartWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(StartWidget)

public:
    explicit StartWidget(QWidget* parent = nullptr);

private:
    QListWidget* m_platformList;
};

#endif // STARTWIDGET_H
