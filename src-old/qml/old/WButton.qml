import QtQuick 2.2

import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    property string text: ''
    property alias iconSource: icon.source
    /* property alias containsMouse: mouseArea.containsMouse */
    /* property alias onBtn: p.onButton */
    /* property alias iconSource: icon.source */
    /* property alias font: label.font */
    signal clicked

    activeFocusOnTab: true

    property int iconSize: units.gridUnit * 2

    implicitWidth: iconSize + units.largeSpacing * 2
    implicitHeight: iconSize + units.smallSpacing

    /* Behavior on opacity { */
    /*     OpacityAnimator { */
    /*         duration: units.longDuration */
    /*         easing.type: Easing.InOutQuad */
    /*     } */
    /* } */

    PlasmaCore.IconItem {
        id: icon
        anchors {
            top: parent.top
            horizontalCenter: parent.horizontalCenter
        }
        width: iconSize
        height: iconSize
        /* source: './refresh.png' */
        colorGroup: PlasmaCore.ColorScope.colorGroup
        /* active: mouseArea.containsMouse || root.activeFocus */
        active: mouseArea.containsMouse
            /* ? onBtn = true : ((onBtn = false) == "This must be false!") */
    }

    MouseArea {
        id: mouseArea
        hoverEnabled: true
        /* onClicked: root.clicked() */
        onClicked: {
            root.clicked()
        }
        anchors.fill: parent

        onEntered: {
            label.text = root.text
            /* wallpaper.state = "on" */
        }
        onExited: {
            label.text = ''
            /* wallpaper.state = "off" */
            /* onBtn = false */
        }
    }

    /* Keys.onEnterPressed: clicked() */
    /* Keys.onReturnPressed: clicked() */
    /* Keys.onSpacePressed: clicked() */

    /* Accessible.onPressAction: clicked() */
    /* Accessible.role: Accessible.Button */
    /* Accessible.name: label.text */
}
