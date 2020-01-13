import QtQuick 2.2

import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    width: units.iconSizes.large
    height: width
    property alias inner_color: inner.color
    property string c_name: "null"
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
                /* easing.type: Easing.InOutQuad */
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
                /* easing.type: Easing.InOutQuad */
                easing.type: Easing.OutCubic
            }
        }
    ]

    Rectangle {
        id: inner
        width: units.iconSizes.large
        height: width
        radius: width / 2
        border.width: width * 0.03
        border.color: PlasmaCore.ColorScope.textColor
        /* border.width: width / 10 */
        /* border.color: "transparent" */

        /* Behavior on border.color { */
        /*     ColorAnimation { */
        /*         duration: units.longDuration * 2 */
        /*         /\* easing.type: Easing.InOutQuad *\/ */
        /*     } */
        /* } */

        MouseArea {
            id: ma
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                /* inner.border.color = PlasmaCore.ColorScope.highlightColor */
                /* wallpaper.state = "on" */
                root.state = "on"
            }
            onExited: {
                /* inner.border.color = "transparent" */
                root.state = "off"
                /* wallpaper.state = "off" */
            }
            onClicked: cuw_row.cuw_clicked(inner.color)
        }

    }
}
