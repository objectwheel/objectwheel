#ifndef SIGNALEDITOR_H
#define SIGNALEDITOR_H

#include <QDialog>

namespace Ui { class SignalEditor; }

class SignalEditor final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(SignalEditor)

public:
    explicit SignalEditor(QWidget *parent = 0);
    ~SignalEditor() override;

    void setSignalList(const QStringList&);
    QString currentSignal() const;

public slots:
    void clear();

private slots:
    void filterList();

private:
    Ui::SignalEditor *ui;
};

#endif // SIGNALEDITOR_H
