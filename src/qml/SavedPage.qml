import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.12

Rectangle {
    id: root
//    color: "transparent"

    RoomView {
        id: saved_view
        anchors.fill: parent
        model: room_model.saved_model
    }
}


