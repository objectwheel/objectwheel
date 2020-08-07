#ifndef PLANMANAGER_H
#define PLANMANAGER_H

#include <QObject>

namespace PlanManager {

Q_NAMESPACE

enum Plans : quint32 {
    None = 0,
    Free = 0x100,
    Indie = 0x23b,
    Pro = 0x5ad
};
Q_ENUM_NS(Plans)

} // PlanManager

#endif // PLANMANAGER_H
