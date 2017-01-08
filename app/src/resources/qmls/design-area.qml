import QtQuick 2.0
import QtQuick.Controls 2.0

Rectangle {
    id: designArea
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
            designArea.currentPageChanged(swipeView.currentItem, swipeView.currentIndex)
        }
        Item {
            id: page
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
