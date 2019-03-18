#ifndef RUNCONTROLLER_H
#define RUNCONTROLLER_H

#include <QObject>

class RunPane;

class RunController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(RunController)

public:
    explicit RunController(RunPane* m_runPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onRunButtonClick();

signals:
    void ran();

private:
    RunPane* m_runPane;
};

#endif // RUNCONTROLLER_H