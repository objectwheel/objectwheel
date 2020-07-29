#ifndef SUCCEEDWIDGET_H
#define SUCCEEDWIDGET_H

#include <QLabel>

class QMovie;
class SucceedWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SucceedWidget)

public:
    explicit SucceedWidget(QWidget* parent = nullptr);

public slots:
    void play(const QString& title, const QString& description);

signals:
    void done();

private:
    QMovie* m_movie;
    QLabel* m_titleLabel;
    QLabel* m_descriptionLabel;
};

#endif // SUCCEEDWIDGET_H