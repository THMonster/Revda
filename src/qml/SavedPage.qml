import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Styles
//import QtGraphicalEffects

Rectangle {
    id: root
//    color: "transparent"

    RoomView {
        id: saved_view
        anchors.fill: parent
        model: room_model.saved_model
    }
}


