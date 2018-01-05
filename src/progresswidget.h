#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>

class QMovie;
class QTimer;
class QProgressBar;

class ProgressWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ProgressWidget(QWidget* parent = nullptr);

    public slots:
        void hide();
        void show(QWidget* parent = nullptr);
        void show(const QString& text, QWidget* parent = nullptr);
        void busy(int progress, const QString& text);
        void done(const QString& text);

    protected:
        void paintEvent(QPaintEvent* event) override;

    private:
        int _progress;
        QString _text;
        QMovie* _movie;
        QTimer* _waitEffectTimer;
        QString _waitEffectString;
};

#endif // PROGRESSWIDGET_H
