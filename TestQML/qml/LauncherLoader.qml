import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    id: control

    property bool running: false  // Définir la propriété running

    Item {
        implicitWidth: 64
        implicitHeight: 64

        Item {
            id: item
            x: parent.width / 2 - 32
            y: parent.height / 2 - 32
            width: 40
            height: 40
            opacity: control.running ? 1 : 0  // Opacité selon la propriété running

            Behavior on opacity {
                OpacityAnimator {
                    duration: 250
                }
            }

            RotationAnimator {
                target: item
                running: control.visible && control.running
                from: 0
                to: 360
                loops: Animation.Infinite
                duration: 1250
            }

            Repeater {
                id: repeater
                model: 6

                Rectangle {
                    x: item.width / 2 - width / 2
                    y: item.height / 2 - height / 2
                    implicitWidth: 10
                    implicitHeight: 10
                    radius: 5
                    color: "#ffffff"
                    transform: [
                        Translate {
                            y: -Math.min(item.width, item.height) * 0.5 + 5
                        },
                        Rotation {
                            angle: index / repeater.count * 360
                            origin.x: 5
                            origin.y: 5
                        }
                    ]
                }
            }
        }
    }

    // Pour tester, nous allons changer la valeur de `running` avec un Timer
    Timer {
        interval: 2000
        running: true
        repeat: false
        onTriggered: {
            control.running = !control.running  // Inverser l'état de running à chaque événement du Timer
        }
    }
}
