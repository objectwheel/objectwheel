#ifndef BUILDSSCREEN_H
#define BUILDSSCREEN_H

#include <QQuickWidget>

class BuildsScreenPrivate;

class BuildsScreen : public QQuickWidget
{
		Q_OBJECT
	public:
        explicit BuildsScreen(QWidget *parent = 0);
        ~BuildsScreen();
        static BuildsScreen* instance();

    private:
        static BuildsScreenPrivate* m_d;

    public slots:
        void handleBuildButtonClicked();
};

#endif // BUILDSSCREEN_H
