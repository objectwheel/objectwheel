import QtQml 2.12
import QtMultimedia 5.12

QtObject {

    property var captureMode : Camera.CaptureMode.CaptureStillImage
    //    property Camera.Position position : Camera.UnspecifiedPosition
    property real digitalZoom : 1
    property real opticalZoom : 1
    property var viewfinder : QtObject {
        property size resolution
        property real minimumFrameRate : 1
        property real maximumFrameRate : 30
    }
}


//Component {
//    name: "QCamera"
//    prototype: "QMediaObject"
//    Enum {
//        name: "Status"
//        values: {
//            "UnavailableStatus": 0,
//            "UnloadedStatus": 1,
//            "LoadingStatus": 2,
//            "UnloadingStatus": 3,
//            "LoadedStatus": 4,
//            "StandbyStatus": 5,
//            "StartingStatus": 6,
//            "StoppingStatus": 7,
//            "ActiveStatus": 8
//        }
//    }
//    Enum {
//        name: "State"
//        values: {
//            "UnloadedState": 0,
//            "LoadedState": 1,
//            "ActiveState": 2
//        }
//    }
//    Enum {
//        name: "CaptureMode"
//        values: {
//            "CaptureViewfinder": 0,
//            "CaptureStillImage": 1,
//            "CaptureVideo": 2
//        }
//    }
//    Enum {
//        name: "Error"
//        values: {
//            "NoError": 0,
//            "CameraError": 1,
//            "InvalidRequestError": 2,
//            "ServiceMissingError": 3,
//            "NotSupportedFeatureError": 4
//        }
//    }
//    Enum {
//        name: "LockStatus"
//        values: {
//            "Unlocked": 0,
//            "Searching": 1,
//            "Locked": 2
//        }
//    }
//    Enum {
//        name: "LockChangeReason"
//        values: {
//            "UserRequest": 0,
//            "LockAcquired": 1,
//            "LockFailed": 2,
//            "LockLost": 3,
//            "LockTemporaryLost": 4
//        }
//    }
//    Enum {
//        name: "LockType"
//        values: {
//            "NoLock": 0,
//            "LockExposure": 1,
//            "LockWhiteBalance": 2,
//            "LockFocus": 4
//        }
//    }
//    Enum {
//        name: "Position"
//        values: {
//            "UnspecifiedPosition": 0,
//            "BackFace": 1,
//            "FrontFace": 2
//        }
//    }
//    Property { name: "state"; type: "QCamera::State"; isReadonly: true }
//    Property { name: "status"; type: "QCamera::Status"; isReadonly: true }
//    Property { name: "captureMode"; type: "QCamera::CaptureModes" }
//    Property { name: "lockStatus"; type: "QCamera::LockStatus"; isReadonly: true }
//    Signal {
//        name: "stateChanged"
//        Parameter { name: "state"; type: "QCamera::State" }
//    }
//    Signal {
//        name: "captureModeChanged"
//        Parameter { type: "QCamera::CaptureModes" }
//    }
//    Signal {
//        name: "statusChanged"
//        Parameter { name: "status"; type: "QCamera::Status" }
//    }
//    Signal { name: "locked" }
//    Signal { name: "lockFailed" }
//    Signal {
//        name: "lockStatusChanged"
//        Parameter { name: "status"; type: "QCamera::LockStatus" }
//        Parameter { name: "reason"; type: "QCamera::LockChangeReason" }
//    }
//    Signal {
//        name: "lockStatusChanged"
//        Parameter { name: "lock"; type: "QCamera::LockType" }
//        Parameter { name: "status"; type: "QCamera::LockStatus" }
//        Parameter { name: "reason"; type: "QCamera::LockChangeReason" }
//    }
//    Signal {
//        name: "error"
//        Parameter { type: "QCamera::Error" }
//    }
//    Method {
//        name: "setCaptureMode"
//        Parameter { name: "mode"; type: "QCamera::CaptureModes" }
//    }
//    Method { name: "load" }
//    Method { name: "unload" }
//    Method { name: "start" }
//    Method { name: "stop" }
//    Method { name: "searchAndLock" }
//    Method { name: "unlock" }
//    Method {
//        name: "searchAndLock"
//        Parameter { name: "locks"; type: "QCamera::LockTypes" }
//    }
//    Method {
//        name: "unlock"
//        Parameter { name: "locks"; type: "QCamera::LockTypes" }
//    }
//}