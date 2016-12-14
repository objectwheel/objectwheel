#ifndef CONTAINER_H
#define CONTAINER_H

#include <QStackedWidget>

class Container : public QStackedWidget
{
		Q_OBJECT

	public:
		explicit Container(QWidget* const parent = 0);

	public slots:
		void handleAction();

};

#endif // CONTAINER_H
