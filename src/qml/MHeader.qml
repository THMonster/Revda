import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
//import QtQuick.Controls.Styles
//import QtGraphicalEffects 1.12

Rectangle {
    id: root
    objectName: "header"
    height: 70
    signal open_clicked(string url)
    signal refresh_clicked()


    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 5

        TextField {
            id: tf
            Layout.minimumWidth: 50
            Layout.preferredWidth: 350
//            Layout.alignment: Qt.AlignLeft
            placeholderText: qsTr("在此输入房间代码, eg: do-9999")

            Keys.onReturnPressed: {
                open_clicked(tf.text);
                tf.text = '';
            }
        }

        Action {
            id: enter_action
            text: "进入房间"
            icon.source: "qrc:///artworks/enter.svg"
            onTriggered: {
                open_clicked(tf.text);
                tf.text = '';
            }
        }

        IconButton {
            id: enter_button
            action: enter_action
            Layout.preferredHeight: 50
            Layout.preferredWidth: 50
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

        }

        Action {
            id: refresh_action
            text: "刷新"
            icon.source: "qrc:///artworks/refresh.svg"
            onTriggered: {
                refresh_clicked();
            }
        }

        IconButton {
            id: refresh_button
            action: refresh_action
            Layout.preferredHeight: 47
            Layout.preferredWidth: 47
        }

    }
}
