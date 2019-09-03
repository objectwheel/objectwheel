#ifndef SIGNALCHOOSERDIALOG_H
#define SIGNALCHOOSERDIALOG_H

#include <QDialog>

namespace Ui { class SignalChooserDialog; }

class SignalChooserDialog final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SignalChooserDialog)

public:
    explicit SignalChooserDialog(QWidget *parent = 0);
    ~SignalChooserDialog() override;

    void setSignalList(const QStringList&);
    QString currentSignal() const;

public slots:
    void discharge();

private slots:
    void filterList();

private:
    Ui::SignalChooserDialog *ui;
};

#endif // SIGNALCHOOSERDIALOG_H
