import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4

Item {
    id: root
    property int page_index
//    property int page_index_in
    ColumnLayout {
        id: tablayout
        anchors.fill: parent
        MIconTabButton {
            id: saved_tb
            source_svg: checked ? "qrc:///artworks/saved_selected.svg" : "qrc:///artworks/saved.svg"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: 40
            checked: true
            onClicked: {
                root.page_index = 0
            }
        }
        MIconTabButton {
            id: history_tb
            source_svg: checked ? "qrc:///artworks/history_selected.svg" : "qrc:///artworks/history.svg"
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: 40
            Layout.preferredWidth: 40
//            checked: false
            onClicked: {
                root.page_index = 1
            }
        }
        Item {
            id: ph
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
    }
}
