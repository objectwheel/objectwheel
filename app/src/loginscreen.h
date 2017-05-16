#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QQuickWidget>

class LoginScreen : public QQuickWidget
{
		Q_OBJECT
	public:
        explicit LoginScreen(QWidget *parent = 0);

	public slots:
        void handleAutoLoginButtonClicked();
        void handleAboutButtonClicked();
        void handleLostPasswordButtonClicked();
        void handleLoginButtonClicked(const QVariant& json);
        void clearGUI();
};

#endif // LOGINSCREEN_H
