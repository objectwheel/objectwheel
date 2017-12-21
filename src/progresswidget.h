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
        void show();
        void show(const QString& text);

    protected:
        virtual void paintEvent(QPaintEvent* event) override;

    private:
        QString _text;
        QMovie* _movie;
        QTimer* _waitEffectTimer;
        QString _waitEffectString;
};

#endif // PROGRESSWIDGET_H
