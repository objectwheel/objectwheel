import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    id: dashboard
    x: 0; y: 0
    width: 100
    height: 100
    color: "#e0e4e7"

    property alias swipeView : swipeView
    property alias current_page : swipeView.currentItem
    signal currentPageChanged(var currentPage, var index)

    SwipeView {
        id: swipeView
        anchors.fill: parent
        onCurrentItemChanged: {
            dashboard.currentPageChanged(swipeView.currentItem, swipeView.currentIndex)
        }
        Item {
            id: page1
            function show() {
                for (var i = 0; i < swipeView.count; i++) {
                    if (swipeView.itemAt(i) === page1) {
                        swipeView.currentIndex = i
                    }
                }
            }
        }
    }

    PageIndicator {
        id: indicator
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.bottom: swipeView.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
