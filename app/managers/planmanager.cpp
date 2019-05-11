#include <planmanager.h>

namespace PlanManager {

bool isEligibleForOfflineLogging(Plans plan)
{
    if (plan == Pro || plan == Enterprise)
        return true;
    return false;
}

bool isEligibleForNewProject(Plans plan, int projectCount)
{
    if (plan == Pro || plan == Enterprise)
        return true;
    if (projectCount < 3)
        return true;
    return false;
}

} // PlanManager
