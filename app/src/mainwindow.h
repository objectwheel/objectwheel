#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class MainWindowPrivate;
class QQuickItem;
class ResizerTick;
class RotatorTick;
class RemoverTick;
class CoverMenu;
class TitleBar;

class MainWindow : public QWidget
{
		Q_OBJECT
		Q_DISABLE_COPY(MainWindow)

	public:
		static MainWindowPrivate* m_d;

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
        void on_buildsButton_clicked();
		void on_clearButton_clicked();
        void on_playButton_clicked();
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
        void handleCurrentFormChanges(const QVariant& CurrentForm, const QVariant& index);
        void handleEditorOpenButtonClicked();
        void handleIndicatorChanges();
        void cleanupObjectwheel();

	signals:
		void resized();
		void centralWidgetMoved();
};

#endif // MAINWINDOW_H
