import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.12

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
