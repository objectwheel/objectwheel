#ifndef OUTPUTBAR_H
#define OUTPUTBAR_H

#include <QWidget>

class QAbstractButton;
class OutputBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputBar)

public:
    explicit OutputBar(QWidget* parent = nullptr);

    QList<QAbstractButton*> buttons() const;

    QAbstractButton* addButton();

public slots:
    void flash(QAbstractButton* button);

private:
    void paintEvent(QPaintEvent* event) override;
};

#endif // OUTPUTBAR_H