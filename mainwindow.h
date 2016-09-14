#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class MainWindow;
}

class QQuickItem;
class ResizerTick;
class RemoverTick;

class MainWindow : public QMainWindow
{
		Q_OBJECT

	public:
		explicit MainWindow(QWidget *parent = 0);
		~MainWindow();

	public slots:
		void RemoveItem(QQuickItem* const item);

	private slots:
		void on_clearButton_clicked();
		void on_editButton_clicked();

	private:
		typedef QList<QQuickItem*> QQuickItemList;

		Ui::MainWindow *ui;
		QString m_ToolsDir;
		ResizerTick* m_ResizerTick;
		RemoverTick* m_RemoverTick;
		QQuickItem* m_RootItem;
		QQuickItemList m_Items;

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
};

#endif // MAINWINDOW_H
