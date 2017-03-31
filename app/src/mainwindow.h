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
		typedef QList<QQuickItem*> QQuickItemList;
		typedef QList<QUrl> QQuickUrlList;
		static MainWindowPrivate* m_d;

	private:
		ResizerTick* m_ResizerTick;
		RotatorTick* m_RotatorTick;
		RemoverTick* m_RemoverTick;
		QQuickItem* m_RootItem;
		QQuickItem* m_CurrentPage;
		CoverMenu* m_RightMenu;
		CoverMenu* m_LeftMenu;

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
		void SetupGui();
		void SetupManagers();
		QQuickItem* GetDeepestDesignItemOnPoint(const QPoint& point) const;
		static const QQuickItemList GetAllChildren(QQuickItem* const item);
		bool eventFilter(QObject* object, QEvent* event);
		void resizeEvent(QResizeEvent *event);
		void fixWebViewPosition(QQuickItem* const item);
		const QPixmap DownloadPixmap(const QUrl& url);
		static bool addControlWithoutSave(const QUrl& url, const QString& parent);
		void clearStudio();

	private slots:
		void on_clearButton_clicked();
		void on_editButton_clicked();
		void handleToolboxUrlboxChanges(const QString& text);
		void handleToolboxNameboxChanges(QString name);
		void toolboxEditButtonToggled(bool);
		void toolboxRemoveButtonClicked();
		void toolboxAddButtonClicked();
		void toolboxResetButtonClicked();
		void toolboxImportButtonClicked();
		void toolboxExportButtonClicked();
		void handleImports(const QStringList& fileNames);
		void fixWebViewPositions();
		void HideSelectionTools();
		void ShowSelectionTools(QQuickItem* const selectedItem);
		void handleCurrentPageChanges(const QVariant& CurrentPage, const QVariant& index);
        void handleBubbleHeadClicked();

	signals:
		void resized();
		void centralWidgetMoved();
		void selectionHided();
		void selectionShowed(QObject* const selectedItem);

};

#endif // MAINWINDOW_H
