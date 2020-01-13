import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.12


Rectangle {
    id: root
    width: 330; height: 100
    color: "#e6e6e6"

    RowLayout {
        anchors.fill: parent
        Rectangle {
            id: online_flag
            Layout.preferredHeight: 100
            Layout.preferredWidth: 5
            color: model.display.status === 1 ? "#8bc443" : "#cdcdcd"
        }

        Image {
            Layout.alignment: Qt.AlignCenter
            Layout.preferredHeight: 100
            Layout.preferredWidth: 100

            Behavior on opacity {
                NumberAnimation { duration: 300 }
            }

            opacity: 0
            fillMode: Image.PreserveAspectCrop
            id: img
            asynchronous: true
            source: model.display.cover
            sourceSize.width: 200
            onStatusChanged: {
                if (img.status === Image.Ready) {
                    img.opacity = 1;
                } else {
                    img.opacity = 0;
                }
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Label {
                Layout.fillWidth: true
                id: title
                horizontalAlignment: Text.AlignRight
                maximumLineCount: 1
                text: model.display.title
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
            }

            Label {
                Layout.fillWidth: true
                id: owner
                maximumLineCount: 1
                horizontalAlignment: Text.AlignRight
                text: model.display.owner
                wrapMode: Text.WordWrap
                elide: Text.ElideRight
            }

//            Item {
//                Layout.fillHeight: true
//                Layout.fillWidth: true
//            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Item {
                    Layout.fillWidth: true
                }

                Action {
                    property int like: model.display.like
                    id: like_action
                    text: "收藏/取消收藏"
                    icon.source: like === 0 ? "qrc:///artworks/like_selected.svg" : "qrc:///artworks/like.svg"
                    onTriggered: {
                        if (like === 0) {
                            like = 1;
                            gv.like_clicked(1, model.display.url);
                        } else {
                            like = 0;
                            gv.like_clicked(0, model.display.url);
                        }
                    }
                }

                IconButton {
                    id: like_button
                    action: like_action
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                }

                Action {
                    id: open_action
                    text: "打开房间"
                    icon.source: "qrc:///artworks/open.svg"
                    onTriggered: {
                        gv.room_clicked(model.display.url);
                    }
                }

                IconButton {
                    id: open_button
                    action: open_action
                    Layout.preferredHeight: 30
                    Layout.preferredWidth: 30
                }
            }

        }

        Rectangle {
            color: "transparent"
            Layout.fillHeight: true
            Layout.preferredWidth: 5
        }



//        MouseArea {
//            id: ma
//            hoverEnabled: true
//            anchors.fill: parent

//            onEntered: {
//                root.border.color = "#409eff"
//            }
//            onExited: {
//                root.border.color = "transparent"
//            }

//            onClicked: {
//                blur.opacity = 1;
//                gv.enabled = false;
//                gv.album_clicked(model.display.upc, model.display.title);

//            }
//        }
    }
}
