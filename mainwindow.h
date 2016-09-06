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

	private slots:
		void on_clearButton_clicked();
		void on_editButton_clicked();
		void ClearSelectionEffect();

	private:
		Ui::MainWindow *ui;
		QString m_ToolsDir;
		ResizerTick* m_ResizerTick;
		RemoverTick* m_RemoverTick;
		QQuickItem* m_SelectionEffect;

		void SetupGui();
		bool CheckTools(const QJsonObject& toolsObject) const;
		void DownloadTools(const QUrl& url);
		void AddTool(const QString& name);
		void ExtractZip(const QByteArray& zipData, const QString& path) const;
		void ShowSelectionTools(QQuickItem* const selectedItem);
		void HideSelectionTools();
		bool eventFilter(QObject* object, QEvent* event);
};

#endif // MAINWINDOW_H
