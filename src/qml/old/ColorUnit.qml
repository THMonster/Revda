import QtQuick 2.2

import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Dialogs 1.0
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    property alias inner_color: inner.color
    property string c_name: "null"
    property int cu_index
    width: inner.width * 1.3
    height: width
    state: "off"
    states: [
        State {
            name: "on"
            PropertyChanges {
                target: root
                scale: 1.3
            }
        },
        State {
            name: "off"
            PropertyChanges {
                target: root
                scale: 1
            }
        }
    ]
    transitions: [
        Transition {
            from: "off"
            to: "on"
            NumberAnimation {
                target: root
                property: "scale"
                duration: 200
                easing.type: Easing.OutCubic
            }
        },
        Transition {
            from: "on"
            to: "off"
            NumberAnimation {
                target: root
                property: "scale"
                duration: 200
                easing.type: Easing.OutCubic
            }
        }
    ]
    Rectangle {
        id: inner
        width: units.iconSizes.medium * 1.2
        height: width
        radius: width / 2
        border.width: width * 0.03
        border.color: PlasmaCore.ColorScope.textColor
        anchors.centerIn: parent

        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onEntered: {
                root.state = "on"
            }
            onExited: {
                root.state = "off"
            }
            onClicked: {
                if(mouse.button & Qt.RightButton) {
                    if (root.c_name != "own" && root.c_name != "default") {
                        gv.remove_color(root.cu_index);
                    }
                } else {
                    if (root.c_name != "own") {
                        gv.cu_clicked(c)
                    } else {
                        colorDialog.open()
                    }
                }
            }
        }

        ColorDialog {
            id: colorDialog
            /* title: "Please choose a color" */
            onAccepted: {
                /* console.log("You chose: " + colorDialog.color) */
                gv.add_color("" + colorDialog.color)
                Qt.quit()
            }
            onRejected: {
                /* console.log("Canceled") */
                Qt.quit()
            }
            /* Component.onCompleted: visible = true */
        }

        PlasmaCore.IconItem {
            id: icon
            anchors {
                /* top: parent.top */
                /* horizontalCenter: parent.horizontalCenter */
                centerIn: parent
            }
            source: 'kcolorchooser'
            width: parent.width * 0.7
            height: width
            /* source: './refresh.png' */
            colorGroup: PlasmaCore.ColorScope.colorGroup
            /* active: mouseArea.containsMouse || root.activeFocus */
            visible: root.c_name != "own" ? false : true
        }

    }
}
