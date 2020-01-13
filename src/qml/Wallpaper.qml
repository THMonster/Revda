import QtQuick 2.2

import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras


Item {
    id: root
    /* width: 1280 */
    /* height: 720 */
    implicitHeight: wallpaper.height + label.height
    Image {
        property real factor: 0
        property bool onButton: false
        id: wallpaper
        objectName: "wp_view"
        smooth: true
        height: units.largeSpacing * 10
        anchors {
            /* fill: parent */
            top: root.top
            left: root.left
            right: root.right
            /* bottom: label.top */
        }
        fillMode: Image.PreserveAspectCrop
        state: "off"

        states: [
            State {
                name: "on"
                PropertyChanges {
                    target: wallpaper
                    factor: 1
                }
            },
            State {
                name: "off"
                PropertyChanges {
                    target: wallpaper
                    factor: 0
                }
            },
            State {
                name: "busy"
                PropertyChanges {
                    target: wallpaper
                    factor: 1
                }
            },
            State {
                name: "generated"
                PropertyChanges {
                    target: wallpaper
                    factor: 1
                }
            }
        ]

        transitions: [
            Transition {
                from: "off"
                to: "on"
                ParallelAnimation {
                    NumberAnimation {
                        target: wallpaper
                        property: "factor"
                        duration: 200
                        /* easing.type: Easing.InOutQuad */
                    }
                }
            },
            Transition {
                from: "on"
                to: "off"
                ParallelAnimation {
                    NumberAnimation {
                        target: wallpaper
                        property: "factor"
                        duration: 200
                        /* easing.type: Easing.InOutQuad */
                    }
                }
            }
        ]
    }

    FastBlur {
        source: wallpaper
        id: blur
        anchors.fill: wallpaper
        radius: 60 * wallpaper.factor
    }

    MouseArea {
        id: mArea
        anchors.fill: wallpaper
        hoverEnabled: true

        onEntered: {
            if (wallpaper.state == "off") {
                wallpaper.state = "on"
            }
        }
        onExited: {
            if (wallpaper.state == "on" || wallpaper.state == "generated") {
                wallpaper.state = "off"
            }
        }

        /* onClicked: { */
        /*     if (wallpaper.state == "off") { */
        /*         wallpaper.state = "on" */
        /*     } else { */
        /*         wallpaper.state = "off" */
        /*     } */
        /* } */

        BusyIndicator {
            running: wallpaper.state == "busy"
            /* running: false */
            anchors.centerIn: parent
        }

        Row {
            opacity: wallpaper.state == "on" ? 1 : 0
            visible: opacity == 0 ? false : true
            spacing: units.smallSpacing

            Behavior on opacity {
                OpacityAnimator {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }

            anchors {
                /* horizontalCenter: wallpaper.horizontalCenter */
                centerIn: parent
            }
            WButton {
                id: wb_refresh
                objectName: "wb_refresh"
                text: i18n("Refresh wallpaper")
                anchors {
                    /* horizontalCenter: wallpaper.horizontalCenter */
                    /* centerIn: wallpaper */
                }
                iconSource: ':/artworks/refresh.svg'
                /* opacity: wallpaper.state == "on" ? 1 : 0 */
                onClicked: {
                    /* wallpaper.state = "busy" */
                }
            }
            WButton {
                id: wb_generate
                objectName: "wb_generate"
                text: i18n("Generate colors based on wallpaper")
                iconSource: ':/artworks/gen-colors.svg'
                onClicked: {
                    wallpaper.state = "generated"
                }
            }
        }

        Row {
            id: cuw_row
            objectName: "cuw_row"
            opacity: wallpaper.state == "generated" ? 1 : 0
            visible: opacity == 0 ? false : true
            spacing: units.smallSpacing

            signal cuw_clicked(string color)

            Behavior on opacity {
                OpacityAnimator {
                    duration: units.longDuration
                    /* easing.type: Easing.InOutQuad */
                }
            }
            anchors {
                centerIn: parent
            }

            ColorUnitW {
                objectName: "cuw"
                inner_color: "white"
            }
            ColorUnitW {
                objectName: "cuw"
                inner_color: "white"
            }
            ColorUnitW {
                objectName: "cuw"
                inner_color: "white"
            }
            ColorUnitW {
                objectName: "cuw"
                inner_color: "white"
            }

        }
    }


    PlasmaExtras.DescriptiveLabel {
        id: label
        /* text: 'testttttt' */
        anchors {
            top: wallpaper.bottom
            topMargin: units.smallSpacing
            left: parent.left
            right: parent.right
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
        /* wrapMode: Text.WordWrap */
        /* font.underline: root.activeFocus */
    }

}
