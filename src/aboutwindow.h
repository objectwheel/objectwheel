#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QPushButton;

class AboutWindow : public QWidget
{
		Q_OBJECT

	public:
        explicit AboutWindow(QWidget* parent = nullptr);

    signals:
        void done();

	private:
        QVBoxLayout* m_layout;
        QLabel* m_titleLabel;
        QLabel* m_logoLabel;
        QLabel* m_versionLabel;
        QPushButton* m_okButton;
        QLabel* m_legalLabel;
};

#endif // ABOUTWINDOW_H
