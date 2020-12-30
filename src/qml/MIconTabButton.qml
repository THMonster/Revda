import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Styles
//import QtGraphicalEffects

TabButton {
    id: root
    property string source_svg
    implicitHeight: 40
    implicitWidth: 40
    contentItem: Image {
        id: icon
        anchors.fill: parent
        smooth: true

        source: source_svg
        sourceSize.width: root.width
        sourceSize.height: root.height

        fillMode: Image.PreserveAspectFit
//        opacity: root.action.enabled ? 1 : 0.6
    }

    background: Rectangle {
        color: "transparent"
//        border.color: (parent.hovered || parent.activeFocus) ? "#409eff" : "transparent"
//        border.width: 1
    }
}
