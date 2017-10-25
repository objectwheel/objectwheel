#ifndef PROGRESSWIDGET_H
#define PROGRESSWIDGET_H

#include <QWidget>
#include <QMovie>
#include <QTimer>
#include <centralwidget.h>

class ProgressWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit ProgressWidget(CentralWidget* parent);
        const QString& msg() const;
        void setMsg(const QString& msg);

    public slots:
        void showProgress(const QString& msg);
        void showProgress();
        void hideProgress();

    protected:
        virtual void paintEvent(QPaintEvent* event) override;

    private:
        CentralWidget* _centralWidget;
        int _lastUid;
        QString _msg;
        QMovie _movie;
        QTimer _waitEffectTimer;
        QString _waitEffectString;
        const QPixmap _logoPixmap;

};

#endif // PROGRESSWIDGET_H
