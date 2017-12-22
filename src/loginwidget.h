#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QQuickWidget>

class LoginWidget : public QQuickWidget
{
		Q_OBJECT

	public:
        explicit LoginWidget(QWidget *parent = 0);

	public slots:
        void handleAutoLoginButtonClick();
        void handleLoginButtonClick(const QVariant& json);
        void handleSessionStart();

    private slots:
        void startSession();

    signals:
        void done();
        void busy(const QString& text);
};

#endif // LOGINWIDGET_H
