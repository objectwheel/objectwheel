#ifndef ROBOTWIDGET_H
#define ROBOTWIDGET_H

#include <QWidget>

class QLabel;
class ButtonSlice;
class QVBoxLayout;
class QWebChannel;
class QWebEngineView;

class RobotWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit RobotWidget(QWidget* parent = nullptr);
        const QString& response() const;

    public slots:
        void load();
        void reset();
        void captchaExpired();
        void updateResponse(const QString& response);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void onNextClicked();

    signals:
        void cancel();
        void done(const QString& response);

    private:
        QString _response;
        QVBoxLayout* _layout;
        QWebChannel* _webChannel;
        QWebEngineView* _recaptchaView;
        QWidget* _recaptchaWidget;
        QVBoxLayout* _recaptchaLayout;
        QLabel* _iconLabel;
        QLabel* _robotLabel;
        QWidget* _space;
        ButtonSlice* _buttons;
};

#endif // ROBOTWIDGET_H