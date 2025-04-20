import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    property alias text: input.text
    property string placeholderText: "Placeholder"
    property alias echoMode: input.echoMode
    property alias font: input.font
    property real inputWidth: 250
    property real inputHeight: 40
    property url iconSource: ""
    property int iconSize: 24
    property string borderColor: "#cccccc"
    property int borderWidth:1
    property string placeHolderColor: "#aaaaaa"

    opacity:1
    width: inputWidth
    height: inputHeight

    Behavior on opacity {
        NumberAnimation { duration: 800; easing.type: Easing.OutCubic }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        radius: 8
        color: "#ffffff"
        border.color: root.borderColor
        border.width: root.borderWidth
    }

    Row {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 6

        // Icône à gauche
        Item {
            id: iconContainer
            width: iconSource !== "" ? iconSize : 0
            height: parent.height

            Image {
                id: icon
                anchors.centerIn: parent
                source: root.iconSource
                width: iconSize
                height: iconSize
                visible: root.iconSource !== ""
                fillMode: Image.PreserveAspectFit
            }
        }

        // Champ de texte
        Item {
            id: textFieldContainer
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - iconContainer.width - 12
            height: parent.height

            TextInput {
                id: input
                anchors.fill: parent
                anchors.margins: 0
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                color: root.placeHolderColor
                cursorVisible: false
                selectionColor: "#448aff"
                selectedTextColor: "black"
                echoMode: TextInput.Normal
                clip: true
                focus: false
                font.pixelSize: root.height * 0.45

                onTextChanged: placeholder.visible = input.text.length === 0
            }

            // Placeholder
            Text {
                id: placeholder
                text: root.placeholderText
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 4
                color: root.placeHolderColor
                font.pixelSize: input.font.pixelSize
                visible: input.text.length === 0 && !input.focus
                z: 0

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.IBeamCursor
                    onClicked: input.forceActiveFocus()
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.IBeamCursor
        onClicked: input.forceActiveFocus()
    }
}