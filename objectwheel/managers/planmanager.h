#ifndef PLANMANAGER_H
#define PLANMANAGER_H

#include <QObject>

namespace PlanManager {

Q_NAMESPACE

enum Plans {
    Free,
    Pro = 0x23b,
    Enterprise = 0x5ad
};
Q_ENUM_NS(Plans)

bool isEligibleForOfflineLogging(Plans plan);
bool isEligibleForNewProject(Plans plan, int projectCount);

} // PlanManager

#endif // PLANMANAGER_H
