import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Styles
//import QtGraphicalEffects 1.12

Rectangle {
    id: root
//    color: "transparent"

    RoomView {
        id: history_view
        anchors.fill: parent
        model: room_model.history_model
    }
}


