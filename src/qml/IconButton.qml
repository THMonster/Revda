import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Styles
//import QtGraphicalEffects 1.12

Button {
    id: root

//    activeFocusOnTab: true
    focusPolicy: Qt.NoFocus
    Keys.onReturnPressed: action.trigger()

    contentItem: Image {
        anchors.fill: parent

        source: root.action.icon.source
        sourceSize.width: root.width
        sourceSize.height: root.height

        fillMode: Image.PreserveAspectFit
//        opacity: root.action.enabled ? 1 : 0.6
    }

    background: Rectangle {
        color: parent.pressed ? "#409eff" : "transparent"
        border.color: (parent.hovered || parent.activeFocus) ? "#409eff" : "transparent"
        border.width: 1
    }

    ToolTip.visible: hovered
    ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
    ToolTip.text: root.action.text
}
