#ifndef ANCHOREDITOR_H
#define ANCHOREDITOR_H

#include <QWidget>

class AnchorEditor final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorEditor)

public:
    explicit AnchorEditor(QWidget* parent = nullptr);

signals:

};

#endif // ANCHOREDITOR_H
