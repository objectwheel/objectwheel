#include <planmanager.h>

namespace PlanManager {

bool isEligibleForOfflineLogging(Plans plan)
{
    if (plan == Pro || plan == Enterprise)
        return true;
    return false;
}

} // PlanManager
