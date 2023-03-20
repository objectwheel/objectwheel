#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include <QtGlobal>

namespace AppConstants {

const short SMTP_PORT      = 465;
const short WSS_PORT       = 5454;
const short PAYLOAD_PORT   = 5455;

const char NAME[]          = APP_NAME;
const char LABEL[]         = APP_LABEL;
const char VERSION[]       = APP_VER;
const char REVISION[]      = APP_REVISION;
const char BUILD_DATE[]    = APP_BUILD_DATE;
const char COMPANY[]       = "Objectwheel";
const char COMPANY_FULL[]  = "Objectwheel";
const char ROOT_DOMAIN[]   = "objectwheel.com";
const char SUPPORT_EMAIL[] = "support@objectwheel.com";
const char FORUM_URL[]     = "https://github.com/objectwheel/objectwheel/discussions";
const char DOWNLOAD_URL[]  = "https://objectwheel.com/download";
const char TAC_URL[]       = "https://objectwheel.com/terms-and-conditions";

#if 0 // LOCAL DEBUG ENVIRONMENT
const char API_DOMAIN[]    = "localhost";
const char WSS_URL[]       = "ws://localhost:5454";
const char UPDATE_URL[]    = "http://localhost:4443";
#else
const char API_DOMAIN[]    = "api.objectwheel.com";
const char WSS_URL[]       = "wss://api.objectwheel.com:5454";
const char UPDATE_URL[]    = "http://update.objectwheel.com";
#endif

} // AppConstants

#endif // APPCONSTANTS_H
