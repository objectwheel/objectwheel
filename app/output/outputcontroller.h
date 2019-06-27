#ifndef OUTPUTCONTROLLER_H
#define OUTPUTCONTROLLER_H

#include <QObject>

class OutputPane;
class OutputController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputController)

public:
    explicit OutputController(OutputPane* outputPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:


private:
    OutputPane* m_outputPane;
};

#endif // OUTPUTCONTROLLER_H
