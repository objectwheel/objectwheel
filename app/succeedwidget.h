#ifndef SUCCEEDWIDGET_H
#define SUCCEEDWIDGET_H

#include <QWidget>

class QVBoxLayout;
class QLabel;
class ButtonSlice;
class QMovie;

class SucceedWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit SucceedWidget(QWidget* parent = nullptr);
        using QWidget::update;

    public slots:
        void start();
        void update(const QString& title, const QString& description);

    protected:
        void paintEvent(QPaintEvent* event) override;

    signals:
        void done();

    private:
        QVBoxLayout* _layout;
        QMovie* _movie;
        QLabel* _iconLabel;
        QLabel* _titleLabel;
        QLabel* _descriptionLabel;
        ButtonSlice* _buttons;
};

#endif // SUCCEEDWIDGET_H