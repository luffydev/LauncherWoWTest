import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    id: btn
    signal buttonClicked()  // Déclaration du signal

    property color bgColor: "#0078d7"
    property color hoverColor: "#3498db"
    property color textColor: "white"

    background: Rectangle {
        color: btn.hovered ? hoverColor : bgColor
        radius: 8
        Behavior on color {
            ColorAnimation { duration: 200 }
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            enabled: false
        }
    }

    onClicked: {
        buttonClicked()  // Émet le signal
    }

    font.pixelSize: 18
    scale: hovered ? 1.2 : 1.0
    Behavior on opacity {
        NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
    }

    /*MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: btn.scale = 1.20
        onExited: btn.scale = 1.0
        cursorShape: Qt.PointingHandCursor

        // Lors du clic, émettre le signal
        onClicked: {
            onButtonClicked()  // Émet le signal
            console.debug("test");
        }
    }*/
}