#ifndef CORECONSTANTS_H
#define CORECONSTANTS_H

#include <QtGlobal>

namespace CoreConstants {

#if 0 // LOCAL DEBUG ENVIRONMENT
const char API_HOST[]      = "localhost";
const char WSS_URL[]       = "ws://localhost:5454";
const char UPDATE_URL[]    = "http://localhost:4443";
#else
const char API_HOST[]      = "api.objectwheel.com";
const char WSS_URL[]       = "wss://api.objectwheel.com:5454";
const char UPDATE_URL[]    = "https://update.objectwheel.com";
#endif

const char TAC_URL[]       = "https://objectwheel.com/terms-and-conditions";
const char EMAIL_ADDRESS[] = "support@objectwheel.com";
const short PAYLAOD_PORT   = 5455;

}

#endif // CORECONSTANTS_H
