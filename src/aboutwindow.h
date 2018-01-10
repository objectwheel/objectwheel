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
        QVBoxLayout* _layout;
        QLabel* _titleLabel;
        QLabel* _logoLabel;
        QLabel* _versionLabel;
        QPushButton* _okButton;
        QLabel* _legalLabel;
};

#endif // ABOUTWINDOW_H
