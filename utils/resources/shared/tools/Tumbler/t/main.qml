import QtQuick 2.7
import QtQuick.Extras 1.4

Tumbler {
    id: tumbler
    height: 50
    width: 160
    clip: true
    TumblerColumn {
        model: [0, 1, 2, 3, 4]
    }
    TumblerColumn {
        model: ["A", "B", "C", "D", "E"]
    }
}
