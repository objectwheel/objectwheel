#ifndef ABOUTWIDGET_H
#define ABOUTWIDGET_H

#include <QWidget>

class QLabel;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;

class AboutWindow : public QWidget
{
		Q_OBJECT

	public:
        explicit AboutWindow(QWidget *parent = 0);

    signals:
        void done();

	private:
        QVBoxLayout* mainLayout;
        QHBoxLayout* iconLayout;
        QLabel* topLabel;
        QLabel* iconLabel;
        QLabel* titleLabel;
        QLabel* legalLabel;
        QPushButton* okButton;
};

#endif // ABOUTWIDGET_H
