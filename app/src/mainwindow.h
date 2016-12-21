#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace Ui {
	class MainWindow;
}

class QQuickItem;
class ResizerTick;
class RotatorTick;
class RemoverTick;
class CoverMenu;
class TitleBar;

class MainWindow : public QWidget
{
		Q_OBJECT

	public:
		typedef QList<QQuickItem*> QQuickItemList;

	private:
		Ui::MainWindow *ui;
		QString m_ToolsDir;
		ResizerTick* m_ResizerTick;
		RotatorTick* m_RotatorTick;
		RemoverTick* m_RemoverTick;
		QQuickItem* m_RootItem;
		QQuickItemList m_Items;
		CoverMenu* m_LeftMenu;
		CoverMenu* m_RightMenu;

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();
		void SetToolsDir();
		void SetupGui();
		bool CheckTools(const QJsonObject& toolsObject) const;
		void DownloadTools(const QUrl& url);
		void AddTool(const QString& name);
		void ExtractZip(const QByteArray& zipData, const QString& path) const;
		QQuickItem* GetDeepestDesignItemOnPoint(const QPoint& point) const;
		const QQuickItemList GetAllChildren(QQuickItem* const item) const;
		void ShowSelectionTools(QQuickItem* const selectedItem);
		void HideSelectionTools();
		bool eventFilter(QObject* object, QEvent* event);
		void resizeEvent(QResizeEvent *event);
		void fixWebViewPosition(QQuickItem* const item);

	private slots:
		void on_clearButton_clicked();
		void on_editButton_clicked();
		void fixWebViewPositions();

	signals:
		void resized();
		void centralWidgetMoved();
		void selectionHided();
		void selectionShowed(QObject* const selectedItem);
};

#endif // MAINWINDOW_H
