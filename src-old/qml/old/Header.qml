import QtQuick 2.2

import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtGraphicalEffects 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    property alias text: title.text
    id: root
    implicitHeight: title.height + 10

    PlasmaExtras.Heading {
        id: title
        text: 'Title'
        level: 2

        anchors {
            top: root.top
            topMargin: units.smallSpacing
            left: root.left
            right: root.right
        }

        /* horizontalAlignment: Text.AlignHCenter */
        /* verticalAlignment: Text.AlignTop */
        /* wrapMode: Text.WordWrap */
        /* font.underline: root.activeFocus */
    }

    /* Rectangle { */
    /*     id: line */
    /*     anchors { */
    /*         top: title.bottom */
    /*         /\* topMargin: units.smallSpacing *\/ */
    /*         left: root.left */
    /*         right: root.right */
    /*     } */
    /*     /\* width: root.width *\/ */
    /*     height: 1 */
    /*     color: "black" */
    /* } */

}
