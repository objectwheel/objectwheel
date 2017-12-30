#ifndef LOADINGBAR_H
#define LOADINGBAR_H

#include <QWidget>

class LoadingBar : public QWidget
{
        Q_OBJECT

    public:
        explicit LoadingBar(QWidget *parent = nullptr);
        void setText(const QString& text);

    public slots:
        void busy(int progress, const QString& text);
        void done(const QString& text);
        void error(const QString& text);

    protected:
        void paintEvent(QPaintEvent *event) override;

    private slots:
        void handleEnding();
        void handleFader();

    private:
        int _progress;
        QImage _image;
        QString _text;
        QColor _color;
        QTimer* _timer,* _timerFader;
};

#endif // LOADINGBAR_H