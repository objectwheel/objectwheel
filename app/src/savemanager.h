#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>

class SaveManager : public QObject
{
		Q_OBJECT
	public:
		explicit SaveManager(QObject *parent = 0);

	signals:

	public slots:
};

#endif // SAVEMANAGER_H