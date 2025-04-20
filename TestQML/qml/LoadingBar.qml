import QtQuick 2.15
import QtQuick.Controls 2.15

Rectangle {
    id: root
    width: 250
    height: 20
    radius: 10
    color: "#333333"
    visible: true

    property int progress: 0

    function startLoading() {
        progress = 0
        timer.start()
    }

    Rectangle {
        width: root.width * (progress / 100)  // Barre de progression ajustée selon le progrès
        height: root.height
        radius: 10
        color: "#00bc8c"

       
    }

     Label {
            text: progress + "%"  // Affiche le pourcentage de progression
            anchors.centerIn: parent  // Centrer le texte dans le rectangle de la barre de progression
            font.pixelSize: 12
            color: "white"
        }

    Timer {
        id: timer
        interval: 50
        running: false
        repeat: true
        onTriggered: {
            if (progress < 100) {
                progress += 1
            } else {
                timer.stop()
            }
        }
    }
}