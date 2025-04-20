import QtQuick 6.5
import QtQuick.Controls 6.5
import QtQuick.Layouts 6.5

Item {
    id: root
    anchors.fill: parent
    visible: false
    z: 999

    property string title: "Message"
    property string message: "Texte du message ici"
    property string acceptText: "OK"
    property string cancelText: "Annuler"
    property bool cancellable: true
    property string type: "info" // "info", "warning", "critical"

    signal accepted()
    signal rejected()

    Rectangle {
        anchors.fill: parent
        color: "#00000080"
        MouseArea {
            anchors.fill: parent
            onClicked: if (cancellable) root.visible = false
        }
    }

    Dialog {
        id: dialogBox
        modal: true
        standardButtons: Dialog.NoButton
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 420
        background: Rectangle {
            color: "#2c2f33"
            radius: 12
            border.color: "#444"
            border.width: 1
        }

        contentItem: ColumnLayout {
            spacing: 20
            anchors.margins: 24

            Label {
                text: root.title
                font.pixelSize: 22
                font.bold: true
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                text: root.message
                wrapMode: Text.WordWrap
                font.pixelSize: 16
                color: "#dcdcdc"
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            RowLayout {
                spacing: 12
                Layout.alignment: Qt.AlignHCenter

                // Bouton "OK" ou "Quitter" selon le type
                Button {
                    text: root.type === "critical" ? "Quitter" : root.acceptText
                    font.bold: true
                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 36
                        radius: 6
                        color: root.type === "critical" ? "#e74c3c" : "#43b581"
                    }
                    onClicked: {
                        root.visible = false
                        if (root.type === "critical") {
                            Qt.quit() // Quitte l'application en cas de message critique
                        } else {
                            root.accepted() // Appelle signal accepté pour d'autres types
                        }
                    }
                }

                // Bouton "Annuler", visible seulement si "cancellable" est true et type n'est pas critique
                Button {
                    visible: root.cancellable && root.type !== "critical"
                    text: root.cancelText
                    font.bold: true
                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 36
                        radius: 6
                        color: "#c0392b"
                    }
                    onClicked: {
                        root.visible = false
                        root.rejected()
                    }
                }
            }
        }

        visible: root.visible
        onVisibleChanged: {
            if (!visible) root.visible = false
        }
    }
}