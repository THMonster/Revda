import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Styles
//import QtGraphicalEffects

Rectangle {
    id: root
    property var model

    Component {
        id: roomDelegate
        RoomDelegate {

        }
    }

    GridView {
        id: gv
        objectName: "grid_view"
        clip: true
        model: root.model
        delegate: roomDelegate
        signal room_clicked(string url)
        signal like_clicked(int like, string url)


        anchors.fill: parent
        cellWidth: 340
        cellHeight: 110

//        function unsetBusy() {
////            blur.opacity = 0;
//            gv.enabled = true;
//        }



    }
}
