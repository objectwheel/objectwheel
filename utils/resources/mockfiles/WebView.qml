import QtQuick 2.12
import QtWebView 1.13 // FIXME: On 5.12

Item {
    property url url
    property string httpUserAgent
    signal titleChanged()
    signal loadingChanged(WebViewLoadRequest loadRequest)
    signal loadProgressChanged()
}
