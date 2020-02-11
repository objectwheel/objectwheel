#include <androidplatformwidget.h>
#include <utilityfunctions.h>

#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QToolButton>
#include <QRadioButton>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

// TODO: Add a "description" label under icon like "Enter final application details below right
//       before requesting a cloud build for your project"
// TODO: Make sure each line edit and everything matches safety regular exp
//       except "password" edits, find a solution for this
// TODO: Make sure we warn user before selecting a keystore "warning uploading it to our servers"
// TODO: Add "reset form"
// TODO: Generate certificate
// TODO: Test and check alias and keystore passwords
// TODO: Show aliases
// TODO: Clickable tooltips

const QMap<QString, QString> AndroidPlatformWidget::apiLevelMap {
    { "API 21: Android 5.0", "21" },
    { "API 22: Android 5.1", "22" },
    { "API 23: Android 6.0", "23" },
    { "API 24: Android 7.0", "24" },
    { "API 25: Android 7.1", "25" },
    { "API 26: Android 8.0", "26" },
    { "API 27: Android 8.1", "27" },
    { "API 28: Android 9.0", "28" },
    { "API 29: Android 10.0", "29"}
};

const QMap<QString, QString> AndroidPlatformWidget::orientationMap {
    { "Behind", "behind" },
    { "Full Sensor", "fullSensor" },
    { "Full User", "fullUser" },
    { "Landscape", "landscape" },
    { "Locked", "locked" },
    { "No Sensor", "nosensor" },
    { "Portrait", "portrait" },
    { "Reverse Landscape", "reverseLandscape" },
    { "Reverse Portrait", "reversePortrait" },
    { "Sensor", "sensor" },
    { "Sensor Landscape", "sensorLandscape" },
    { "Sensor Portrait", "sensorPortrait" },
    { "Unspecified", "unspecified" },
    { "User", "user" },
    { "User Landscape", "userLandscape" },
    { "User Portrait", "userPortrait" }
};

const QMap<QString, QString> AndroidPlatformWidget::qtModuleMap {
    { "Qt Bluetooth", "bluetooth" },
    { "Qt Charts", "charts" },
    { "Qt Concurrent", "concurrent" },
    { "Qt Data Visualization", "datavisualization" },
    { "Qt Gamepad", "gamepad" },
    { "Qt Location", "location" },
    { "Qt Multimedia", "multimedia" },
    { "Qt Network Authorization", "networkauth" },
    { "Qt Network", "network" },
    { "Qt Nfc", "nfc" },
    { "Qt OpenGL Extensions", "openglextensions" },
    { "Qt OpenGL", "opengl" },
    { "Qt Positioning Quick", "positioningquick" },
    { "Qt Positioning", "positioning" },
    { "Qt Print Support", "printsupport" },
    { "Qt Purchasing", "purchasing" },
    { "Qt Qml Models", "qmlmodels" },
    { "Qt Qml Worker Script", "qmlworkerscript" },
    { "Qt Quick Templates 2", "quicktemplates2" },
    { "Qt Quick Test", "qmltest" },
    { "Qt Remote Objects", "remoteobjects" },
    { "Qt Script Tools", "scripttools" },
    { "Qt Script", "script" },
    { "Qt Scxml", "scxml" },
    { "Qt Sensors", "sensors" },
    { "Qt Serial Port", "serialport" },
    { "Qt Sql", "sql" },
    { "Qt Svg", "svg" },
    { "Qt Test", "testlib" },
    { "Qt Text to Speech", "texttospeech" },
    { "Qt WebChannel", "webchannel" },
    { "Qt WebSockets", "websockets" },
    { "Qt WebView", "webview" },
    { "Qt Widgets", "widgets" },
    { "Qt Xml Patterns", "xmlpatterns" },
    { "Qt Xml", "xml" },
    { "Qt 3D Animation", "3danimation" },
    { "Qt 3D Core", "3dcore" },
    { "Qt 3D Extras", "3dextras" },
    { "Qt 3D Input", "3dinput" },
    { "Qt 3D Logic", "3dlogic" },
    { "Qt 3D Quick Animation", "3dquickanimation" },
    { "Qt 3D Quick Extras", "3dquickextras" },
    { "Qt 3D Quick Input", "3dquickinput" },
    { "Qt 3D Quick Render", "3dquickrender" },
    { "Qt 3D Quick", "3dquick" },
    { "Qt 3D QuickScene 2D", "3dquickscene2d" },
    { "Qt 3D Render", "3drender" }
};

const QStringList AndroidPlatformWidget::androidPermissionList {
    "android.permission.ACCEPT_HANDOVER",
    "android.permission.ACCESS_BACKGROUND_LOCATION",
    "android.permission.ACCESS_CHECKIN_PROPERTIES",
    "android.permission.ACCESS_COARSE_LOCATION",
    "android.permission.ACCESS_FINE_LOCATION",
    "android.permission.ACCESS_LOCATION_EXTRA_COMMANDS",
    "android.permission.ACCESS_MEDIA_LOCATION",
    "android.permission.ACCESS_MOCK_LOCATION",
    "android.permission.ACCESS_NETWORK_STATE",
    "android.permission.ACCESS_NOTIFICATION_POLICY",
    "android.permission.ACCESS_SURFACE_FLINGER",
    "android.permission.ACCESS_WIFI_STATE",
    "android.permission.ACCOUNT_MANAGER",
    "android.permission.ACTIVITY_RECOGNITION",
    "android.permission.ANSWER_PHONE_CALLS",
    "android.permission.AUTHENTICATE_ACCOUNTS",
    "android.permission.BATTERY_STATS",
    "android.permission.BIND_ACCESSIBILITY_SERVICE",
    "android.permission.BIND_APPWIDGET",
    "android.permission.BIND_AUTOFILL_SERVICE",
    "android.permission.BIND_CALL_REDIRECTION_SERVICE",
    "android.permission.BIND_CARRIER_MESSAGING_CLIENT_SERVICE",
    "android.permission.BIND_CARRIER_MESSAGING_SERVICE",
    "android.permission.BIND_CARRIER_SERVICES",
    "android.permission.BIND_CHOOSER_TARGET_SERVICE",
    "android.permission.BIND_CONDITION_PROVIDER_SERVICE",
    "android.permission.BIND_DEVICE_ADMIN",
    "android.permission.BIND_DREAM_SERVICE",
    "android.permission.BIND_INCALL_SERVICE",
    "android.permission.BIND_INPUT_METHOD",
    "android.permission.BIND_MIDI_DEVICE_SERVICE",
    "android.permission.BIND_NFC_SERVICE",
    "android.permission.BIND_NOTIFICATION_LISTENER_SERVICE",
    "android.permission.BIND_PRINT_SERVICE",
    "android.permission.BIND_QUICK_SETTINGS_TILE",
    "android.permission.BIND_REMOTEVIEWS",
    "android.permission.BIND_SCREENING_SERVICE",
    "android.permission.BIND_TELECOM_CONNECTION_SERVICE",
    "android.permission.BIND_TEXT_SERVICE",
    "android.permission.BIND_TV_INPUT",
    "android.permission.BIND_VISUAL_VOICEMAIL_SERVICE",
    "android.permission.BIND_VOICE_INTERACTION",
    "android.permission.BIND_VPN_SERVICE",
    "android.permission.BIND_VR_LISTENER_SERVICE",
    "android.permission.BIND_WALLPAPER",
    "android.permission.BLUETOOTH",
    "android.permission.BLUETOOTH_ADMIN",
    "android.permission.BLUETOOTH_PRIVILEGED",
    "android.permission.BODY_SENSORS",
    "android.permission.BRICK",
    "android.permission.BROADCAST_PACKAGE_REMOVED",
    "android.permission.BROADCAST_SMS",
    "android.permission.BROADCAST_STICKY",
    "android.permission.BROADCAST_WAP_PUSH",
    "android.permission.CALL_COMPANION_APP",
    "android.permission.CALL_PHONE",
    "android.permission.CALL_PRIVILEGED",
    "android.permission.CAMERA",
    "android.permission.CAPTURE_AUDIO_OUTPUT",
    "android.permission.CHANGE_COMPONENT_ENABLED_STATE",
    "android.permission.CHANGE_CONFIGURATION",
    "android.permission.CHANGE_NETWORK_STATE",
    "android.permission.CHANGE_WIFI_MULTICAST_STATE",
    "android.permission.CHANGE_WIFI_STATE",
    "android.permission.CLEAR_APP_CACHE",
    "android.permission.CLEAR_APP_USER_DATA",
    "android.permission.CONTROL_LOCATION_UPDATES",
    "android.permission.DELETE_CACHE_FILES",
    "android.permission.DELETE_PACKAGES",
    "android.permission.DEVICE_POWER",
    "android.permission.DIAGNOSTIC",
    "android.permission.DISABLE_KEYGUARD",
    "android.permission.DUMP",
    "android.permission.EXPAND_STATUS_BAR",
    "android.permission.FACTORY_TEST",
    "android.permission.FLASHLIGHT",
    "android.permission.FORCE_BACK",
    "android.permission.FOREGROUND_SERVICE",
    "android.permission.GET_ACCOUNTS",
    "android.permission.GET_ACCOUNTS_PRIVILEGED",
    "android.permission.GET_PACKAGE_SIZE",
    "android.permission.GET_TASKS",
    "android.permission.GLOBAL_SEARCH",
    "android.permission.HARDWARE_TEST",
    "android.permission.INJECT_EVENTS",
    "android.permission.INSTALL_LOCATION_PROVIDER",
    "android.permission.INSTALL_PACKAGES",
    "android.permission.INSTANT_APP_FOREGROUND_SERVICE",
    "android.permission.INTERNAL_SYSTEM_WINDOW",
    "android.permission.INTERNET",
    "android.permission.KILL_BACKGROUND_PROCESSES",
    "android.permission.LOCATION_HARDWARE",
    "android.permission.MANAGE_ACCOUNTS",
    "android.permission.MANAGE_APP_TOKENS",
    "android.permission.MANAGE_DOCUMENTS",
    "android.permission.MANAGE_OWN_CALLS",
    "android.permission.MASTER_CLEAR",
    "android.permission.MEDIA_CONTENT_CONTROL",
    "android.permission.MODIFY_AUDIO_SETTINGS",
    "android.permission.MODIFY_PHONE_STATE",
    "android.permission.MOUNT_FORMAT_FILESYSTEMS",
    "android.permission.MOUNT_UNMOUNT_FILESYSTEMS",
    "android.permission.NFC",
    "android.permission.NFC_TRANSACTION_EVENT",
    "android.permission.PACKAGE_USAGE_STATS",
    "android.permission.PERSISTENT_ACTIVITY",
    "android.permission.PROCESS_OUTGOING_CALLS",
    "android.permission.READ_CALENDAR",
    "android.permission.READ_CALL_LOG",
    "android.permission.READ_CONTACTS",
    "android.permission.READ_EXTERNAL_STORAGE",
    "android.permission.READ_FRAME_BUFFER",
    "android.permission.READ_INPUT_STATE",
    "android.permission.READ_LOGS",
    "android.permission.READ_PHONE_NUMBERS",
    "android.permission.READ_PHONE_STATE",
    "android.permission.READ_PROFILE",
    "android.permission.READ_SMS",
    "android.permission.READ_SOCIAL_STREAM",
    "android.permission.READ_SYNC_SETTINGS",
    "android.permission.READ_SYNC_STATS",
    "android.permission.READ_USER_DICTIONARY",
    "android.permission.REBOOT",
    "android.permission.RECEIVE_BOOT_COMPLETED",
    "android.permission.RECEIVE_MMS",
    "android.permission.RECEIVE_SMS",
    "android.permission.RECEIVE_WAP_PUSH",
    "android.permission.RECORD_AUDIO",
    "android.permission.REORDER_TASKS",
    "android.permission.REQUEST_COMPANION_RUN_IN_BACKGROUND",
    "android.permission.REQUEST_COMPANION_USE_DATA_IN_BACKGROUND",
    "android.permission.REQUEST_DELETE_PACKAGES",
    "android.permission.REQUEST_IGNORE_BATTERY_OPTIMIZATIONS",
    "android.permission.REQUEST_INSTALL_PACKAGES",
    "android.permission.REQUEST_PASSWORD_COMPLEXITY",
    "android.permission.RESTART_PACKAGES",
    "android.permission.SEND_RESPOND_VIA_MESSAGE",
    "android.permission.SEND_SMS",
    "android.permission.SET_ACTIVITY_WATCHER",
    "android.permission.SET_ALWAYS_FINISH",
    "android.permission.SET_ANIMATION_SCALE",
    "android.permission.SET_DEBUG_APP",
    "android.permission.SET_ORIENTATION",
    "android.permission.SET_POINTER_SPEED",
    "android.permission.SET_PREFERRED_APPLICATIONS",
    "android.permission.SET_PROCESS_LIMIT",
    "android.permission.SET_TIME",
    "android.permission.SET_TIME_ZONE",
    "android.permission.SET_WALLPAPER",
    "android.permission.SET_WALLPAPER_HINTS",
    "android.permission.SIGNAL_PERSISTENT_PROCESSES",
    "android.permission.SMS_FINANCIAL_TRANSACTIONS",
    "android.permission.START_VIEW_PERMISSION_USAGE",
    "android.permission.STATUS_BAR",
    "android.permission.SUBSCRIBED_FEEDS_READ",
    "android.permission.SUBSCRIBED_FEEDS_WRITE",
    "android.permission.SYSTEM_ALERT_WINDOW",
    "android.permission.TRANSMIT_IR",
    "android.permission.UPDATE_DEVICE_STATS",
    "android.permission.USE_BIOMETRIC",
    "android.permission.USE_CREDENTIALS",
    "android.permission.USE_FINGERPRINT",
    "android.permission.USE_FULL_SCREEN_INTENT",
    "android.permission.USE_SIP",
    "android.permission.VIBRATE",
    "android.permission.WAKE_LOCK",
    "android.permission.WRITE_APN_SETTINGS",
    "android.permission.WRITE_CALENDAR",
    "android.permission.WRITE_CALL_LOG",
    "android.permission.WRITE_CONTACTS",
    "android.permission.WRITE_EXTERNAL_STORAGE",
    "android.permission.WRITE_GSERVICES",
    "android.permission.WRITE_PROFILE",
    "android.permission.WRITE_SECURE_SETTINGS",
    "android.permission.WRITE_SETTINGS",
    "android.permission.WRITE_SMS",
    "android.permission.WRITE_SOCIAL_STREAM",
    "android.permission.WRITE_SYNC_SETTINGS",
    "android.permission.WRITE_USER_DICTIONARY",
    "com.android.alarm.permission.SET_ALARM",
    "com.android.browser.permission.READ_HISTORY_BOOKMARKS",
    "com.android.browser.permission.WRITE_HISTORY_BOOKMARKS",
    "com.android.launcher.permission.INSTALL_SHORTCUT",
    "com.android.launcher.permission.UNINSTALL_SHORTCUT",
    "com.android.voicemail.permission.ADD_VOICEMAIL",
    "com.android.voicemail.permission.READ_VOICEMAIL",
    "com.android.voicemail.permission.WRITE_VOICEMAIL"
};

AndroidPlatformWidget::AndroidPlatformWidget(QWidget* parent) : QWidget(parent)
  , m_nameEdit(new QLineEdit(this))
  , m_versionCodeSpin(new QSpinBox(this))
  , m_versionNameEdit(new QLineEdit(this))
  , m_organizationEdit(new QLineEdit(this))
  , m_domainEdit(new QLineEdit(this))
  , m_packageEdit(new QLineEdit(this))
  , m_screenOrientationCombo(new QComboBox(this))
  , m_minApiLevelCombo(new QComboBox(this))
  , m_targetApiLevelCombo(new QComboBox(this))
  , m_iconPictureLabel(new QLabel(this))
  , m_browseIconButton(new QPushButton(this))
  , m_clearIconButton(new QPushButton(this))
  , m_includePemissionsCheck(new QCheckBox(this))
  , m_permissionCombo(new QComboBox(this))
  , m_permissionList(new QListWidget(this))
  , m_addPermissionButton(new QPushButton(this))
  , m_removePermissionButton(new QPushButton(this))
  , m_aabCheck(new QCheckBox(this))
  , m_abiArmeabiV7aCheck(new QCheckBox(this))
  , m_abiArm64V8aCheck(new QCheckBox(this))
  , m_abiX86Check(new QCheckBox(this))
  , m_abiX8664Check(new QCheckBox(this))
  , m_includeQtModulesCheck(new QCheckBox(this))
  , m_qtModuleCombo(new QComboBox(this))
  , m_qtModuleList(new QListWidget(this))
  , m_addQtModuleButton(new QPushButton(this))
  , m_removeQtModuleButton(new QPushButton(this))
  , m_signingDisabled(new QRadioButton(this))
  , m_signingEnabled(new QRadioButton(this))
  , m_keystorePathEdit(new QLineEdit(this))
  , m_newKeystoreButton(new QPushButton(this))
  , m_browseKeystoreButton(new QPushButton(this))
  , m_clearKeystoreButton(new QPushButton(this))
  , m_keystorePasswordEdit(new QLineEdit(this))
  , m_showKeystorePasswordButton(new QToolButton(this))
  , m_keyAliasCombo(new QComboBox(this))
  , m_keyPasswordEdit(new QLineEdit(this))
  , m_showKeyPasswordButton(new QToolButton(this))
  , m_sameAsKeystorePasswordCheck(new QCheckBox(this))
{
    auto nameLabel = new QLabel(tr("Name:"), this);
    auto versionCodeLabel = new QLabel(tr("Version code:"), this);
    auto versionNameLabel = new QLabel(tr("Version name:"), this);
    auto organizationLabel = new QLabel(tr("Organization:"), this);
    auto domainLabel = new QLabel(tr("Domain:"), this);

    auto generalGroupBox = new QGroupBox(tr("General"), this);
    auto generalLayout = new QGridLayout(generalGroupBox);
    generalLayout->setSpacing(6);
    generalLayout->setContentsMargins(8, 8, 8, 8);
    generalLayout->addWidget(nameLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_nameEdit, 0, 1, 1, 2);
    generalLayout->addWidget(versionCodeLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_versionCodeSpin, 1, 1);
    generalLayout->addWidget(versionNameLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_versionNameEdit, 2, 1, 1, 2);
    generalLayout->addWidget(organizationLabel, 3, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_organizationEdit, 3, 1, 1, 2);
    generalLayout->addWidget(domainLabel, 4, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    generalLayout->addWidget(m_domainEdit, 4, 1, 1, 2);
    generalLayout->setColumnStretch(1, 1);
    generalLayout->setColumnStretch(2, 1);

    auto packageNameLabel = new QLabel(tr("Package name:"), this);
    auto screenOrientationLabel = new QLabel(tr("Screen orientation:"), this);
    auto minApiLevelLabel = new QLabel(tr("Minimum API level:"), this);
    auto targetApiLevelLabel = new QLabel(tr("Target API level:"), this);
    auto iconLabel = new QLabel(tr("Icon:"), this);
    auto permissionLabel = new QLabel(tr("Permissions:"), this);
    auto autoDetectPemissionsCheck = new QCheckBox(tr("Automatically detected"), this);

    auto iconLayout = new QGridLayout;
    iconLayout->setSpacing(4);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    iconLayout->addWidget(m_iconPictureLabel, 0, 0, 2, 1);
    iconLayout->addWidget(m_browseIconButton, 0, 1);
    iconLayout->addWidget(m_clearIconButton, 1, 1);

    auto permDetectionLayout = new QVBoxLayout;
    permDetectionLayout->setSpacing(0);
    permDetectionLayout->setContentsMargins(iconLayout->contentsMargins());
    permDetectionLayout->addWidget(autoDetectPemissionsCheck);
    permDetectionLayout->addWidget(m_includePemissionsCheck);

    auto permissionLayout = new QGridLayout;
    permissionLayout->setSpacing(iconLayout->spacing());
    permissionLayout->setContentsMargins(iconLayout->contentsMargins());
    permissionLayout->addLayout(permDetectionLayout, 0, 0);
    permissionLayout->addWidget(m_permissionCombo, 1, 0);
    permissionLayout->addWidget(m_addPermissionButton, 1, 1);
    permissionLayout->addWidget(m_permissionList, 2, 0, 2, 1);
    permissionLayout->addWidget(m_removePermissionButton, 2, 1);

    auto androidSpesificGroupBox = new QGroupBox(tr("Android Spesific"), this);
    auto androidSpesificLayout = new QGridLayout(androidSpesificGroupBox);
    androidSpesificLayout->setSpacing(generalLayout->spacing());
    androidSpesificLayout->setContentsMargins(generalLayout->contentsMargins());
    androidSpesificLayout->addWidget(packageNameLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_packageEdit, 0, 1, 1, 2);
    androidSpesificLayout->addWidget(screenOrientationLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_screenOrientationCombo, 1, 1);
    androidSpesificLayout->addWidget(minApiLevelLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_minApiLevelCombo, 2, 1);
    androidSpesificLayout->addWidget(targetApiLevelLabel, 3, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addWidget(m_targetApiLevelCombo, 3, 1);
    androidSpesificLayout->addWidget(iconLabel, 4, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addLayout(iconLayout, 4, 1);
    androidSpesificLayout->addWidget(permissionLabel, 5, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    androidSpesificLayout->addLayout(permissionLayout, 5, 1, 1, 2);
    androidSpesificLayout->setColumnStretch(1, 1);
    androidSpesificLayout->setColumnStretch(2, 1);

    auto aabLabel = new QLabel(tr("Android App Bundle:"), this);
    auto abisLabel = new QLabel(tr("Build ABIs:"), this);
    auto qtModulesLabel = new QLabel(tr("Qt modules:"), this);
    auto autoDetectQtModulesCheck = new QCheckBox(tr("Automatically detected"), this);

    auto abisLayout = new QVBoxLayout;
    abisLayout->setSpacing(0);
    abisLayout->setContentsMargins(iconLayout->contentsMargins());
    abisLayout->addWidget(m_abiArmeabiV7aCheck);
    abisLayout->addWidget(m_abiArm64V8aCheck);
    abisLayout->addWidget(m_abiX86Check);
    abisLayout->addWidget(m_abiX8664Check);

    auto qtDetectionLayout = new QVBoxLayout;
    qtDetectionLayout->setSpacing(0);
    qtDetectionLayout->setContentsMargins(iconLayout->contentsMargins());
    qtDetectionLayout->addWidget(autoDetectQtModulesCheck);
    qtDetectionLayout->addWidget(m_includeQtModulesCheck);

    auto qtModuleLayout = new QGridLayout;
    qtModuleLayout->setSpacing(iconLayout->spacing());
    qtModuleLayout->setContentsMargins(iconLayout->contentsMargins());
    qtModuleLayout->addLayout(qtDetectionLayout, 0, 0);
    qtModuleLayout->addWidget(m_qtModuleCombo, 1, 0);
    qtModuleLayout->addWidget(m_addQtModuleButton, 1, 1);
    qtModuleLayout->addWidget(m_qtModuleList, 2, 0, 2, 1);
    qtModuleLayout->addWidget(m_removeQtModuleButton, 2, 1);

    auto buildingGroupBox = new QGroupBox(tr("Building"), this);
    auto buildingLayout = new QGridLayout(buildingGroupBox);
    buildingLayout->setSpacing(generalLayout->spacing());
    buildingLayout->setContentsMargins(generalLayout->contentsMargins());
    buildingLayout->addWidget(aabLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    buildingLayout->addWidget(m_aabCheck, 0, 1, 1, 2);
    buildingLayout->addWidget(abisLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    buildingLayout->addLayout(abisLayout, 1, 1, 1, 2);
    buildingLayout->addWidget(qtModulesLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    buildingLayout->addLayout(qtModuleLayout, 2, 1, 1, 2);
    buildingLayout->setColumnStretch(1, 1);
    buildingLayout->setColumnStretch(2, 1);

    auto statusLabel = new QLabel(tr("Status:"), this);
    auto keystorePathLabel = new QLabel(tr("Keystore path:"), this);
    auto keystorePasswordLabel = new QLabel(tr("Keystore password:"), this);
    auto keyAliasLabel = new QLabel(tr("Key alias:"), this);
    auto keyPasswordLabel = new QLabel(tr("Key password:"), this);

    auto statusLayout = new QHBoxLayout;
    statusLayout->setSpacing(iconLayout->spacing());
    statusLayout->setContentsMargins(iconLayout->contentsMargins());
    statusLayout->addWidget(m_signingDisabled);
    statusLayout->addWidget(m_signingEnabled);
    statusLayout->addStretch();

    auto keystorePathLayout = new QGridLayout;
    keystorePathLayout->setSpacing(iconLayout->spacing());
    keystorePathLayout->setContentsMargins(iconLayout->contentsMargins());
    keystorePathLayout->addWidget(m_keystorePathEdit, 0, 0, 1, 4);
    keystorePathLayout->addWidget(m_newKeystoreButton, 1, 0);
    keystorePathLayout->addWidget(m_browseKeystoreButton, 1, 1);
    keystorePathLayout->addWidget(m_clearKeystoreButton, 1, 2);
    keystorePathLayout->setColumnStretch(3, 1);

    auto keystorePasswordLayout = new QHBoxLayout;
    keystorePasswordLayout->setSpacing(iconLayout->spacing());
    keystorePasswordLayout->setContentsMargins(iconLayout->contentsMargins());
    keystorePasswordLayout->addWidget(m_keystorePasswordEdit);
    keystorePasswordLayout->addWidget(m_showKeystorePasswordButton);

    auto keyPasswordLayout = new QGridLayout;
    keyPasswordLayout->setSpacing(iconLayout->spacing());
    keyPasswordLayout->setContentsMargins(iconLayout->contentsMargins());
    keyPasswordLayout->addWidget(m_keyPasswordEdit, 0, 0);
    keyPasswordLayout->addWidget(m_showKeyPasswordButton, 0, 1);
    keyPasswordLayout->addWidget(m_sameAsKeystorePasswordCheck, 1, 0);

    auto signingGroupBox = new QGroupBox(tr("Signing"), this);
    auto signingLayout = new QGridLayout(signingGroupBox);
    signingLayout->setSpacing(generalLayout->spacing());
    signingLayout->setContentsMargins(generalLayout->contentsMargins());
    signingLayout->addWidget(statusLabel, 0, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(statusLayout, 0, 1, 1, 2);
    signingLayout->addWidget(keystorePathLabel, 1, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(keystorePathLayout, 1, 1, 1, 2);
    signingLayout->addWidget(keystorePasswordLabel, 2, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(keystorePasswordLayout, 2, 1, 1, 2);
    signingLayout->addWidget(keyAliasLabel, 3, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addWidget(m_keyAliasCombo, 3, 1, 1, 2);
    signingLayout->addWidget(keyPasswordLabel, 4, 0, 1, 1, Qt::AlignTop | Qt::AlignRight);
    signingLayout->addLayout(keyPasswordLayout, 4, 1, 1, 2);
    signingLayout->setColumnStretch(1, 1);
    signingLayout->setColumnStretch(2, 1);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);
    layout->addWidget(generalGroupBox);
    layout->addWidget(androidSpesificGroupBox);
    layout->addWidget(buildingGroupBox);
    layout->addWidget(signingGroupBox);

    m_versionCodeSpin->setCursor(Qt::PointingHandCursor);
    m_screenOrientationCombo->setCursor(Qt::PointingHandCursor);
    m_minApiLevelCombo->setCursor(Qt::PointingHandCursor);
    m_targetApiLevelCombo->setCursor(Qt::PointingHandCursor);
    m_clearIconButton->setCursor(Qt::PointingHandCursor);
    m_browseIconButton->setCursor(Qt::PointingHandCursor);
    m_includePemissionsCheck->setCursor(Qt::PointingHandCursor);
    m_permissionCombo->setCursor(Qt::PointingHandCursor);
    m_addPermissionButton->setCursor(Qt::PointingHandCursor);
    m_removePermissionButton->setCursor(Qt::PointingHandCursor);
    m_aabCheck->setCursor(Qt::PointingHandCursor);
    m_abiX8664Check->setCursor(Qt::PointingHandCursor);
    m_abiX86Check->setCursor(Qt::PointingHandCursor);
    m_abiArm64V8aCheck->setCursor(Qt::PointingHandCursor);
    m_abiArmeabiV7aCheck->setCursor(Qt::PointingHandCursor);
    m_includeQtModulesCheck->setCursor(Qt::PointingHandCursor);
    m_qtModuleCombo->setCursor(Qt::PointingHandCursor);
    m_addQtModuleButton->setCursor(Qt::PointingHandCursor);
    m_removeQtModuleButton->setCursor(Qt::PointingHandCursor);
    m_signingDisabled->setCursor(Qt::PointingHandCursor);
    m_signingEnabled->setCursor(Qt::PointingHandCursor);
    m_newKeystoreButton->setCursor(Qt::PointingHandCursor);
    m_browseKeystoreButton->setCursor(Qt::PointingHandCursor);
    m_clearKeystoreButton->setCursor(Qt::PointingHandCursor);
    m_keyAliasCombo->setCursor(Qt::PointingHandCursor);
    m_showKeystorePasswordButton->setCursor(Qt::PointingHandCursor);
    m_showKeyPasswordButton->setCursor(Qt::PointingHandCursor);
    m_sameAsKeystorePasswordCheck->setCursor(Qt::PointingHandCursor);

    m_versionCodeSpin->setMaximum(std::numeric_limits<int>::max());
    autoDetectPemissionsCheck->setChecked(true);
    autoDetectQtModulesCheck->setChecked(true);
    autoDetectPemissionsCheck->setEnabled(false);
    autoDetectQtModulesCheck->setEnabled(false);
    m_keystorePathEdit->setEnabled(false);
    m_showKeystorePasswordButton->setCheckable(true);
    m_showKeyPasswordButton->setCheckable(true);

    m_browseIconButton->setText(tr("Browse"));
    m_clearIconButton->setText(tr("Clear"));
    m_addPermissionButton->setText(tr("Add"));
    m_removePermissionButton->setText(tr("Remove"));
    m_addQtModuleButton->setText(tr("Add"));
    m_removeQtModuleButton->setText(tr("Remove"));
    m_aabCheck->setText(tr("Build AAB package instead of APK"));
    m_abiArmeabiV7aCheck->setText(tr("armeabi-v7a"));
    m_abiArm64V8aCheck->setText(tr("arm64-v8a"));
    m_abiX86Check->setText(tr("x86"));
    m_abiX8664Check->setText(tr("x86_64"));
    m_includeQtModulesCheck->setText(tr("Include manual additions below too:"));
    m_includePemissionsCheck->setText(tr("Include manual additions below too:"));
    m_signingDisabled->setText(tr("Disabled"));
    m_signingEnabled->setText(tr("Enabled"));
    m_newKeystoreButton->setText(tr("New"));
    m_browseKeystoreButton->setText(tr("Browse"));
    m_clearKeystoreButton->setText(tr("Clear"));
    m_sameAsKeystorePasswordCheck->setText(tr("Same as keystore password"));

    m_nameEdit->setToolTip(tr("Application name"));
    m_versionCodeSpin->setToolTip(tr("Application version (int)"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "An internal version number. This number is used to determine whether one<br>"
                                     "version is more recent than another, with higher numbers indicating more<br>"
                                     "recent versions. This is not the version number shown to users; that<br>"
                                     "number is set by the version name field.</p>"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "The value must be set as an integer, such as 100. You can define it<br>"
                                     "however you want, as long as each successive version has a higher number.<br>"
                                     "For example, it could be a build number. Or you could translate a version<br>"
                                     "number in x.y format to an integer by encoding the x and y separately in<br>"
                                     "the lower and upper 16 bits. Or you could simply increase the number by<br>"
                                     "one each time a new version is released.</p>"));
    m_versionNameEdit->setToolTip(tr("Application version (string)"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "The version number shown to users. The version code field holds the<br>"
                                     "significant version number used internally.</p>"));
    m_organizationEdit->setToolTip(tr("Your organization name"));
    m_domainEdit->setToolTip(tr("Your organization domain"));
    m_packageEdit->setToolTip(tr("<p style='white-space:nowrap'>"
                                 "Application package name. "
                                 "For more information please refer to:<br><i>"
                                 "<a href='https://developer.android.com/guide/topics/manifest/manifest-element#package'>"
                                 "https://developer.android.com/guide/topics/manifest/manifest-element#package</a></i></p>"));
    m_screenOrientationCombo->setToolTip(tr("<p style='white-space:nowrap'>"
                                            "Application screen orientation. "
                                            "For more information please refer to:<br><i>"
                                            "<a href='https://developer.android.com/guide/topics/manifest/activity-element#screen'>"
                                            "https://developer.android.com/guide/topics/manifest/activity-element#screen</a></i></p>"));
    m_minApiLevelCombo->setToolTip(tr("<p style='white-space:nowrap'>"
                                      "Minimum API Level required for your application to run. "
                                      "For more information please refer to:<br><i>"
                                      "<a href='https://developer.android.com/guide/topics/manifest/uses-sdk-element#min'>"
                                      "https://developer.android.com/guide/topics/manifest/uses-sdk-element#min</a></i></p>"));
    m_targetApiLevelCombo->setToolTip(tr("<p style='white-space:nowrap'>"
                                         "The API Level that your application targets. "
                                         "For more information please refer to:<br><i>"
                                         "<a href='https://developer.android.com/guide/topics/manifest/uses-sdk-element#target'>"
                                         "https://developer.android.com/guide/topics/manifest/uses-sdk-element#target</a></i></p>"));
    m_iconPictureLabel->setToolTip(tr("<p style='white-space:nowrap'>"
                                      "Preview of your application icon (<i>If no icon provided, Android will<br>"
                                      "assign an default icon for your application</i>)</p>"));
    m_browseIconButton->setToolTip(tr("Select an application icon from your computer"));
    m_clearIconButton->setToolTip(tr("Clear application icon"));
    autoDetectPemissionsCheck->setToolTip(tr("Enable automatic detection of Android permissions"
                                             "<p style='white-space:nowrap;font-size:11px'>"
                                             "Our cloud build system will try to guess Android permissions based on<br>"
                                             "the QML imports and Qt modules you use in your project.</p>"));
    m_includePemissionsCheck->setToolTip(tr("Enable adding extra Android permissions manually"
                                            "<p style='white-space:nowrap;font-size:11px'>"
                                            "It could be useful to add some extra permissions manually in case our<br>"
                                            "cloud build system might not be able to detect all the necessary<br>"
                                            "permissions for your application to run.</p>"));
    m_permissionCombo->setToolTip(tr("<p style='white-space:nowrap'>"
                                     "Permission list to choose from. For more information about Android permissions please refer to:<br><i>"
                                     "<a href='https://developer.android.com/reference/android/Manifest.permission'>"
                                     "https://developer.android.com/reference/android/Manifest.permission</a></i></p>"));
    m_permissionList->setToolTip(tr("User defined Android permissions for the application"));
    autoDetectQtModulesCheck->setToolTip(tr("Enable automatic detection of Qt modules"
                                            "<p style='white-space:nowrap;font-size:11px'>"
                                            "Our cloud build system will try to guess Qt modules your application needs<br>"
                                            "to be linked against based on the QML imports you use in your project.</p>"));
    m_includeQtModulesCheck->setToolTip(tr("Enable adding extra Qt modules manually"
                                           "<p style='white-space:nowrap;font-size:11px'>"
                                           "It could be useful to add some extra Qt modules to link against manually<br>"
                                           "in case our cloud build system might not be able to detect all the<br>"
                                           "necessary Qt modules for your application to run. E.g. if you use svg<br>"
                                           "images in your project we might not be able to detect it (since our<br>"
                                           "algorithm only scans for the import statements in qml files).</p>"));
    m_qtModuleCombo->setToolTip(tr("<p style='white-space:nowrap'>"
                                   "Qt module list to choose from. For more information about Qt modules please refer to:<br><i>"
                                   "<a href='https://doc.qt.io/qt-5/qtmodules.html'>"
                                   "https://doc.qt.io/qt-5/qtmodules.html</a></i></p>"));
    m_qtModuleList->setToolTip(tr("User defined Qt modules for the application to link against"));
    m_aabCheck->setToolTip(tr("<p style='white-space:nowrap'>"
                              "For more information about Android App Bundles:<br><i>"
                              "<a href='https://developer.android.com/guide/app-bundle'>"
                              "https://developer.android.com/guide/app-bundle</a></i></p>"));
    m_abiArmeabiV7aCheck->setToolTip(tr("<p style='white-space:nowrap'>"""
                                        "Enable bundling the application package with the necessary assets and<br>"
                                        "libraries to support devices with armeabi-v7a ABI.</p>"));
    m_abiArm64V8aCheck->setToolTip(tr("<p style='white-space:nowrap'>"
                                      "Enable bundling the application package with the necessary assets and<br>"
                                      "libraries to support devices with arm64-v8a ABI.</p>"));
    m_abiX86Check->setToolTip(tr("<p style='white-space:nowrap'>"
                                 "Enable bundling the application package with the necessary assets and<br>"
                                 "libraries to support devices with x86 ABI.</p>"));
    m_abiX8664Check->setToolTip(tr("<p style='white-space:nowrap'>"
                                   "Enable bundling the application package with the necessary assets and<br>"
                                   "libraries to support devices with x86_64 ABI.</p>"));
    m_signingEnabled->setToolTip(tr("Enable package signing"
                                    "<p style='white-space:nowrap;font-size:11px'>"
                                    "Android (hence Google Play and other app stores) requires that all APKs be<br>"
                                    "digitally signed with a certificate before they are installed on a device<br>"
                                    "or updated.</p>"
                                    "<p style='white-space:nowrap;font-size:11px'>"
                                    "Your private key is required for signing all future versions of your app.<br>"
                                    "Thus you should sign your app with the same certificate throughout its<br>"
                                    "expected lifespan. If you lose or misplace your key, you will not be able<br>"
                                    "to publish updates to your existing app. You cannot regenerate a<br>"
                                    "previously generated key.<p>"
                                    "<p style='white-space:nowrap;font-size:11px'>"
                                    "If you sign the new version of your existing app with a different<br>"
                                    "certificate, you must assign a different package name to the app—in this<br>"
                                    "case, the user installs the new version as a completely new app (it's not<br>"
                                    "your existing app anymore).</p>"
                                    "<p style='white-space:nowrap;font-size:11px'>"
                                    "Securing your private key is of critical importance, both to you and to<br>"
                                    "the user. If you allow someone to use your key, or if you leave your<br>"
                                    "keystore and passwords in an unsecured location such that a third-party<br>"
                                    "could find and use them, your authoring identity and the trust of the user<br>"
                                    "are compromised.</p>"
                                    "<p style='white-space:nowrap;font-size:11px'>"
                                    "If a third party should manage to take your key without your knowledge or<br>"
                                    "permission, that person could sign and distribute apps that maliciously<br>"
                                    "replace your authentic apps or corrupt them. Such a person could also sign<br>"
                                    "and distribute apps under your identity that attack other apps or the<br>"
                                    "system itself, or corrupt or steal user data.</p>"
                                    "<p style='white-space:nowrap;font-size:11px'>"
                                    "Your reputation as a developer entity depends on your securing your app<br>"
                                    "signing key properly, at all times, until the key is expired. For more<br>"
                                    "information:<br>"
                                    "<a href='https://developer.android.com/studio/publish/app-signing#considerations'>"
                                    "https://developer.android.com/studio/publish/app-signing#considerations</a></i></p>"));
    m_signingDisabled->setToolTip(tr("Disable package signing"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "Android (hence Google Play and other app stores) requires that all APKs be<br>"
                                     "digitally signed with a certificate before they are installed on a device<br>"
                                     "or updated.</p>"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "When package signing disabled, our cloud build system automatically signs<br>"
                                     "your app with a common debug certificate generated by the Android SDK<br>"
                                     "tools. Thus, this option should only be used for debugging purposes. Do<br>"
                                     "not use this option for your production builds.</p>"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "Because the debug certificate is created by the build tools and is<br>"
                                     "insecure by design, most app stores (including the Google Play Store) do<br>"
                                     "not accept apps signed with a debug certificate for publishing.</p>"
                                     "<p style='white-space:nowrap;font-size:11px'>"
                                     "The debug certificate used to sign your app for debugging has an<br>"
                                     "expiration date of 30 years from its creation date. When the certificate<br>"
                                     "expires, you get a build error.</p>"));
    m_keystorePathEdit->setToolTip(tr("Path to keystore file"
                                      "<p style='white-space:nowrap;font-size:11px'>"
                                      "A keystore file is a binary file that serves as a repository of<br>"
                                      "certificates and private keys.</p>"));
    m_newKeystoreButton->setToolTip(tr("Establish new keystore"));
    m_browseKeystoreButton->setToolTip(tr("Select keystore file from your computer"));
    m_clearKeystoreButton->setToolTip(tr("Clear keystore settings"));
    m_keystorePasswordEdit->setToolTip(tr("Password for your keystore"));
    m_keyAliasCombo->setToolTip(tr("Select the name of the private key you want to use for signing"));
    m_keyPasswordEdit->setToolTip(tr("Password for private key"));
    m_showKeystorePasswordButton->setToolTip(tr("Toggle keystore password visibility"));
    m_showKeyPasswordButton->setToolTip(tr("Toggle key password visibility"));

    m_nameEdit->setPlaceholderText(tr("My Application"));
    m_versionNameEdit->setPlaceholderText(tr("1.1 Gold Edition"));
    m_organizationEdit->setPlaceholderText(tr("My Example Org, Inc."));
    m_domainEdit->setPlaceholderText(tr("example.com"));
    m_packageEdit->setPlaceholderText(tr("com.example.myapplication"));
    m_keystorePathEdit->setPlaceholderText(tr("Use 'Browse' button below to choose your keystore file"));
    m_keystorePasswordEdit->setPlaceholderText(tr("Type your keystore password"));
    m_keyPasswordEdit->setPlaceholderText(tr("Type your key password"));

    m_browseIconButton->setIcon(QIcon(QStringLiteral(":/images/builds/browse.svg")));
    m_clearIconButton->setIcon(QIcon(QStringLiteral(":/images/designer/clear.svg")));
    m_addPermissionButton->setIcon(QIcon(QStringLiteral(":/images/designer/plus.svg")));
    m_removePermissionButton->setIcon(QIcon(QStringLiteral(":/images/designer/minus.svg")));
    m_addQtModuleButton->setIcon(QIcon(QStringLiteral(":/images/designer/plus.svg")));
    m_removeQtModuleButton->setIcon(QIcon(QStringLiteral(":/images/designer/minus.svg")));
    m_newKeystoreButton->setIcon(QIcon(QStringLiteral(":/images/designer/new-file.svg")));
    m_browseKeystoreButton->setIcon(QIcon(QStringLiteral(":/images/builds/browse.svg")));
    m_clearKeystoreButton->setIcon(QIcon(QStringLiteral(":/images/designer/clear.svg")));
    m_showKeystorePasswordButton->setIcon(QIcon(QStringLiteral(":/images/builds/show.svg")));
    m_showKeyPasswordButton->setIcon(QIcon(QStringLiteral(":/images/builds/show.svg")));

    int iconPictureSize = m_browseIconButton->sizeHint().height()
            + iconLayout->spacing()
            + m_clearIconButton->sizeHint().height();
    m_iconPictureLabel->setFixedSize(iconPictureSize, iconPictureSize);
    m_iconPictureLabel->setFrameShape(QFrame::StyledPanel);

    int labelColMinSz = aabLabel->sizeHint().width(); // Longest label's width
    generalLayout->setColumnMinimumWidth(0, labelColMinSz);
    androidSpesificLayout->setColumnMinimumWidth(0, labelColMinSz);
    buildingLayout->setColumnMinimumWidth(0, labelColMinSz);
    signingLayout->setColumnMinimumWidth(0, labelColMinSz);

    m_permissionCombo->addItems(androidPermissionList);
    m_qtModuleCombo->addItems(qtModuleMap.keys());
    m_screenOrientationCombo->addItems(orientationMap.keys());
    m_minApiLevelCombo->addItems(apiLevelMap.keys());
    m_targetApiLevelCombo->addItems(apiLevelMap.keys());
    m_permissionCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_qtModuleCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_minApiLevelCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_targetApiLevelCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

    m_versionCodeSpin->setValue(1);
    m_screenOrientationCombo->setCurrentText(QStringLiteral("Unspecified"));
    m_minApiLevelCombo->setCurrentText(QStringLiteral("API 21: Android 5.0"));
    m_targetApiLevelCombo->setCurrentText(QStringLiteral("API 23: Android 6.0"));
    m_aabCheck->setChecked(false);
    m_abiArmeabiV7aCheck->setChecked(true);
    m_abiArm64V8aCheck->setChecked(false);
    m_abiX86Check->setChecked(false);
    m_abiX8664Check->setChecked(false);
    m_includeQtModulesCheck->setChecked(true);
    m_qtModuleList->clear();
    m_qtModuleList->addItem(QLatin1String("Qt Svg"));
    m_signingDisabled->setChecked(true);
    m_keystorePathEdit->clear();
    m_keystorePasswordEdit->clear();
    m_keyAliasCombo->clear();
    m_keyPasswordEdit->clear();
    m_keyPasswordEdit->setEchoMode(QLineEdit::Password);
    m_keystorePasswordEdit->setEchoMode(QLineEdit::Password);
    m_keyPasswordEdit->setEnabled(false);
    m_showKeyPasswordButton->setEnabled(false);
    m_showKeystorePasswordButton->setChecked(false);
    m_showKeyPasswordButton->setChecked(false);
    m_sameAsKeystorePasswordCheck->setChecked(true);

    UtilityFunctions::disableWheelEvent(m_versionCodeSpin);
    m_addPermissionButton->setEnabled(false);
    m_permissionList->setEnabled(false);
    m_permissionCombo->setEnabled(false);
    m_removePermissionButton->setEnabled(false);
    m_removeQtModuleButton->setEnabled(false);
    m_sameAsKeystorePasswordCheck->setEnabled(false);
    m_keyPasswordEdit->setEnabled(false);
    m_showKeyPasswordButton->setEnabled(false);
    m_keyAliasCombo->setEnabled(false);
    m_showKeystorePasswordButton->setEnabled(false);
    m_keystorePasswordEdit->setEnabled(false);
    m_newKeystoreButton->setEnabled(false);
    m_browseKeystoreButton->setEnabled(false);
    m_clearKeystoreButton->setEnabled(false);
}

QLineEdit* AndroidPlatformWidget::nameEdit() const
{
    return m_nameEdit;
}

QSpinBox* AndroidPlatformWidget::versionCodeSpin() const
{
    return m_versionCodeSpin;
}

QLineEdit* AndroidPlatformWidget::versionNameEdit() const
{
    return m_versionNameEdit;
}

QLineEdit* AndroidPlatformWidget::organizationEdit() const
{
    return m_organizationEdit;
}

QLineEdit* AndroidPlatformWidget::domainEdit() const
{
    return m_domainEdit;
}

QLineEdit* AndroidPlatformWidget::packageEdit() const
{
    return m_packageEdit;
}

QComboBox* AndroidPlatformWidget::screenOrientationCombo() const
{
    return m_screenOrientationCombo;
}

QComboBox* AndroidPlatformWidget::minApiLevelCombo() const
{
    return m_minApiLevelCombo;
}

QComboBox* AndroidPlatformWidget::targetApiLevelCombo() const
{
    return m_targetApiLevelCombo;
}

QLabel* AndroidPlatformWidget::iconPictureLabel() const
{
    return m_iconPictureLabel;
}

QPushButton* AndroidPlatformWidget::browseIconButton() const
{
    return m_browseIconButton;
}

QPushButton* AndroidPlatformWidget::clearIconButton() const
{
    return m_clearIconButton;
}

QCheckBox* AndroidPlatformWidget::includePemissionsCheck() const
{
    return m_includePemissionsCheck;
}

QComboBox* AndroidPlatformWidget::permissionCombo() const
{
    return m_permissionCombo;
}

QListWidget* AndroidPlatformWidget::permissionList() const
{
    return m_permissionList;
}

QPushButton* AndroidPlatformWidget::addPermissionButton() const
{
    return m_addPermissionButton;
}

QPushButton* AndroidPlatformWidget::removePermissionButton() const
{
    return m_removePermissionButton;
}

QCheckBox* AndroidPlatformWidget::aabCheck() const
{
    return m_aabCheck;
}

QCheckBox* AndroidPlatformWidget::abiArmeabiV7aCheck() const
{
    return m_abiArmeabiV7aCheck;
}

QCheckBox* AndroidPlatformWidget::abiArm64V8aCheck() const
{
    return m_abiArm64V8aCheck;
}

QCheckBox* AndroidPlatformWidget::abiX86Check() const
{
    return m_abiX86Check;
}

QCheckBox* AndroidPlatformWidget::abiX8664Check() const
{
    return m_abiX8664Check;
}

QCheckBox* AndroidPlatformWidget::includeQtModulesCheck() const
{
    return m_includeQtModulesCheck;
}

QComboBox* AndroidPlatformWidget::qtModuleCombo() const
{
    return m_qtModuleCombo;
}

QListWidget* AndroidPlatformWidget::qtModuleList() const
{
    return m_qtModuleList;
}

QPushButton* AndroidPlatformWidget::addQtModuleButton() const
{
    return m_addQtModuleButton;
}

QPushButton* AndroidPlatformWidget::removeQtModuleButton() const
{
    return m_removeQtModuleButton;
}

QRadioButton* AndroidPlatformWidget::signingDisabled() const
{
    return m_signingDisabled;
}

QRadioButton* AndroidPlatformWidget::signingEnabled() const
{
    return m_signingEnabled;
}

QLineEdit* AndroidPlatformWidget::keystorePathEdit() const
{
    return m_keystorePathEdit;
}

QPushButton* AndroidPlatformWidget::newKeystoreButton() const
{
    return m_newKeystoreButton;
}

QPushButton* AndroidPlatformWidget::browseKeystoreButton() const
{
    return m_browseKeystoreButton;
}

QPushButton* AndroidPlatformWidget::clearKeystoreButton() const
{
    return m_clearKeystoreButton;
}

QLineEdit* AndroidPlatformWidget::keystorePasswordEdit() const
{
    return m_keystorePasswordEdit;
}

QToolButton* AndroidPlatformWidget::showKeystorePasswordButton() const
{
    return m_showKeystorePasswordButton;
}

QComboBox* AndroidPlatformWidget::keyAliasCombo() const
{
    return m_keyAliasCombo;
}

QLineEdit* AndroidPlatformWidget::keyPasswordEdit() const
{
    return m_keyPasswordEdit;
}

QToolButton* AndroidPlatformWidget::showKeyPasswordButton() const
{
    return m_showKeyPasswordButton;
}

QCheckBox* AndroidPlatformWidget::sameAsKeystorePasswordCheck() const
{
    return m_sameAsKeystorePasswordCheck;
}
