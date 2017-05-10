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
        void resizeEvent(QResizeEvent *event) override;

    signals:
        void resized() const;

    public slots:
        void handleBuildButtonClicked();
        void handleBtnOkClicked();
        void handleBtnCancelClicked();
};

#endif // BUILDSSCREEN_H
