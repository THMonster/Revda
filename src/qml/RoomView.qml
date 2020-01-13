import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.12

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

//    FastBlur {
//        source: gv
//        opacity: 0
//        visible: opacity === 0 ? false : true
//        id: blur
//        anchors.fill: gv
//        radius: 70
//        transparentBorder: false
//        layer.enabled: true
//        layer.effect: HueSaturation {
//            cached: true
//            lightness: 0.6
//        }

//        Behavior on opacity {
//            NumberAnimation { duration: 300 }
//        }
//    }

//    BusyIndicator {
//        opacity: 0
//        visible: opacity === 0 ? false : true
//        running: opacity === 0 ? false : true
//        width: gv.height * 0.15
//        height: gv.height * 0.15
//        anchors.centerIn: gv
//        Behavior on opacity {
//            NumberAnimation { duration: 300 }
//        }
//    }
}
