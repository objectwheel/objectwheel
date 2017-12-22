#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>

class QMovie;
class QTimer;

class ProgressWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ProgressWidget(QWidget* parent = nullptr);
        ~ProgressWidget();

        const QString& text() const;
        void setText(const QString& text);

    public slots:
        void hide();
        void show(QWidget* parent = nullptr);
        void show(const QString& text, QWidget* parent = nullptr);

    protected:
        void paintEvent(QPaintEvent* event) override;
        QSize sizeHint() const override;

    private:
        QString _text;
        QMovie* _movie;
        QTimer* _waitEffectTimer;
        QString _waitEffectString;
};

#endif // PROGRESSWIDGET_H
