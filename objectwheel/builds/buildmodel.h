#ifndef BUILDMODEL_H
#define BUILDMODEL_H

#include <QAbstractListModel>

struct Animal
{
    QString name;
    QString version;
    QString abis;
};

class AnimalModel final : public QAbstractListModel
{
    Q_OBJECT
    Q_DISABLE_COPY(AnimalModel)

public:
    enum AnimalRoles {
        TypeRole = Qt::UserRole + 1,
        SizeRole
    };

public:
    explicit AnimalModel(QObject* parent = nullptr);

    void addAnimal(const Animal& animal);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

private:
    QList<Animal> m_animals;
};

#endif // BUILDMODEL_H
