#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class MainWindowPrivate;
class CoverMenu;

class MainWindow : public QWidget
{
		Q_OBJECT
		Q_DISABLE_COPY(MainWindow)

	public:
        static MainWindowPrivate* _d;

	private:
		CoverMenu* m_RightMenu;
		CoverMenu* m_LeftMenu;

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
		void SetupGui();
		void SetupManagers();
		void resizeEvent(QResizeEvent *event);
		const QPixmap DownloadPixmap(const QUrl& url);
		void clearStudio();

    public slots:
        void on_secureExitButton_clicked();
		void handleToolboxUrlboxChanges(const QString& text);
		void handleToolboxNameboxChanges(QString name);
		void toolboxEditButtonToggled(bool);
		void toolboxRemoveButtonClicked();
		void toolboxAddButtonClicked();
		void toolboxResetButtonClicked();
		void toolboxImportButtonClicked();
		void toolboxExportButtonClicked();
		void handleImports(const QStringList& fileNames);
        void handleIndicatorChanges();
        void cleanupObjectwheel();

	signals:
		void resized();
};

#endif // MAINWINDOW_H
