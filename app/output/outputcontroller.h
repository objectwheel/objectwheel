#ifndef OUTPUTCONTROLLER_H
#define OUTPUTCONTROLLER_H

#include <QProcess>

class OutputPane;
class Control;

class OutputController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputController)

public:
    explicit OutputController(OutputPane* outputPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:
    void onBarButtonClick();
    void onWidgetMinimize();
    void onFlash();
    void onApplicationReadyOutput(const QString& output);
    void onApplicationFinish(int exitCode, QProcess::ExitStatus exitStatus);
    void onApplicationErrorOccur(QProcess::ProcessError error, const QString& errorString);
    void onControlImageChange(Control* control, int codeChanged);

private:
    OutputPane* m_outputPane;
};

#endif // OUTPUTCONTROLLER_H
