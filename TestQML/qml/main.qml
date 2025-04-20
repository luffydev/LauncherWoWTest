import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import QtQuick.Controls.Material
import QtQuick.Dialogs 

import "."

ApplicationWindow {
    id: window
    visible: true
    width: 600
    height: 600
    title: "Launcher animé"
    color: "#1e1e1e"

    maximumHeight: 600
    maximumWidth: 600
    minimumHeight: 600
    minimumWidth: 600

    Material.theme: Material.Dark
    Material.accent: "#00BCD4"

    property bool isLoginFailed: false
    property bool isLoginSucceed: false

    property int downloadSpeed: 0  // Débit de téléchargement en Ko/s

    function onCheckUpdateFinished(){
        spinnerText.text = "Check for update..."
    }

    function displayMessage(pTitle, pMessage){
        messageBox.title = pTitle
        messageBox.message = pMessage;
        messageBox.visible = true
        
    }

    function setLoginSucceed(){
        isLoginSucceed = true
        isLoginFailed = false

        spinnerText.text = "Login complete... "

        console.log("LoginSucceed called ! ");
    }

    function showLoginError(pError){
        isLoginFailed = true
        errorText.text = pError
    }

    function resetLoginError(){
        isLoginFailed = false
        errorText.text = ""
    }
   

    FancyMessageBox {
        id: messageBox
        title: "Erreur"
        message: "Une erreur est survenue pendant la connexion."
        acceptText: "Réessayer"
        cancelText: "Annuler"
        cancellable: true

        onAccepted: {
            console.log("Réessayer cliqué")
        }

        onRejected: {
            Qt.quit()
        }
    }

    // Conteneur principal
    Rectangle {
        id: container
        anchors.fill: parent
        color: "transparent"
        objectName: "mainContainer"

        Component.onCompleted: {
            messageBox.visible = true
            messageBox.type = "critical"
        }

        // Déclarations des états et transitions
        states: [
            State {
                name: "connecting"
                PropertyChanges { target: loginForm; enabled: false }
                PropertyChanges { target: spinnerArea; visible: true }
                PropertyChanges { target: loadingContainer; visible: false }
                PropertyChanges { target: loadingContainer; opacity: 0.0; }
                PropertyChanges { target: spinnerText; text: "Connecting ..." }
            },
            State {
                name: "idle"
                PropertyChanges { target: loginForm; enabled: true }
                PropertyChanges { target: spinnerArea; visible: false }
                PropertyChanges { target: loginForm; opacity: 1.0 }
                PropertyChanges { target: loadingContainer; visible: false }
                PropertyChanges { target: loadingContainer; opacity: 0.0; }
            },
            State {
                name: "updating"
                PropertyChanges { target: loginForm; enabled: false }
                PropertyChanges { target: spinnerArea; visible: true }
                PropertyChanges { target: loadingContainer; visible: true }
                PropertyChanges { target: loadingContainer; opacity: 1.0; }
                PropertyChanges { target: spinnerText; text: "Downloading Update..." }
            }
        ]

        transitions: [
            Transition {
                from: "idle"
                to: "connecting"
                SequentialAnimation {
                    PropertyAnimation { target: loginForm; property: "opacity"; to: 0.0; duration: 300 }
                    PropertyAnimation { target: spinnerArea; property: "opacity"; to: 1.0; duration: 300 }
                }
            },
            Transition {
                from: "connecting"
                to: "idle"
                SequentialAnimation {
                    PropertyAnimation { target: spinnerArea; property: "opacity"; to: 0.0; duration: 300 }
                    PropertyAnimation { target: loginForm; property: "opacity"; to: 1.0; duration: 300 }
                }
            },
            Transition {
                from: "updating"
                to: "idle"
                SequentialAnimation {
                    PropertyAnimation { target: spinnerArea; property: "opacity"; to: 0.0; duration: 300 }
                    PropertyAnimation { target: loadingContainer; property: "opacity"; to: 0.0; duration: 300 }
                    PropertyAnimation { target: loginForm; property: "opacity"; to: 1.0; duration: 300 }
                }
            }
        ]

        // Contenu principal
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 16
            Layout.alignment: Qt.AlignHCenter

            Item { Layout.preferredHeight: 50 }

            // Logo animé
            Image {
                id: logo
                source: "logo.png"
                width: 150
                height: 150
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignHCenter
                opacity: 0.0
                scale: 0.8

                SequentialAnimation on opacity {
                    running: true
                    NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.OutCubic }
                }
                SequentialAnimation on scale {
                    running: true
                    NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.OutBack }
                }
            }

            // Message d'erreur
            Text {
                id: errorText
                objectName: errorText
                text: "Nom d'utilisateur ou mot de passe incorrect !"
                color: "#e74c3c"
                font.pixelSize: 16
                visible: isLoginFailed
                opacity: isLoginFailed ? 1.0 : 0.0
                Layout.alignment: Qt.AlignHCenter

                Behavior on opacity {
                    NumberAnimation { duration: 300 }
                }
            }

            // Spinner de chargement
            ColumnLayout {
                id: spinnerArea
                visible: true
                opacity: 1.0
                spacing: 8
                Layout.alignment: Qt.AlignHCenter

                BusyIndicator {
                    running: true
                    width: 40
                    height: 40
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Checking for update..."
                    color: isLoginSucceed ? "#11ab24" : "white"
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                    id: spinnerText
                    font.bold: isLoginSucceed ? true : false
                }
            }

            // Spinner de chargement
            ColumnLayout {
                id: loadingContainer
                visible: false
                opacity: 0.0
                spacing: 8
                Layout.alignment: Qt.AlignHCenter

                LoadingBar {
                    width: 300
                    height: 20
                    opacity: 1.0
                    objectName: "loadingBar"
                    Layout.alignment: Qt.AlignHCenter
                }

                // Affichage du débit de téléchargement
                Label {
                    text: "0 Ko/s"
                    color: "white"
                    objectName: "downloadSpeed"
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            // Formulaire de connexion
            ColumnLayout {
                id: loginForm
                spacing: 10
                opacity: 0.0
                enabled: false
                Layout.alignment: Qt.AlignHCenter

                CustomTextInput {
                    id: username
                    placeholderText: "Nom d'utilisateur"
                    iconSource: "qrc:/img/user.png"
                    borderColor: isLoginFailed ? "#e44b3b" : "#cccccc"
                    borderWidth: isLoginFailed ? 3 : 1
                    placeHolderColor: isLoginFailed ? "#e44b3b" : "#aaaaaa"
                }

                CustomTextInput {
                    id: password
                    placeholderText: "Mot de passe"
                    echoMode: TextInput.Password
                    iconSource: "qrc:/img/password.png"
                    borderColor: isLoginFailed ? "#e44b3b" : "#cccccc"
                    borderWidth: isLoginFailed ? 3 : 1
                    placeHolderColor: isLoginFailed ? "#e44b3b" : "#aaaaaa"
                }

                AnimatedButton {
                    text: "Connexion"
                    onClicked: checkLogin()
                    Layout.preferredWidth: 250
                }

                AnimatedButton {
                    text: "Quitter"
                    bgColor: "#c0392b"
                    onClicked: Qt.quit()
                    Layout.preferredWidth: 250
                }
            }

            Item { Layout.preferredHeight: 50 }
        }
    }

    // Fonction pour vérifier la connexion
    function checkLogin() {
        isLoginFailed = false

        if (username.text.trim() === "" || password.text.trim() === "") {
            errorText.text = "Login field cannot be empty ! "
            isLoginFailed = true
            return
        }
        
        container.state = "connecting"
        handler.onConnectionClicked(username.text, password.text)
    }

    // Timer de délai avant de changer d'état
    Timer {
        id: delayTimer
        interval: 100  // Petit délai avant de changer l'état
        repeat: false
        onTriggered: {
            container.state = "connecting"  // Changer l'état après le délai
            loginTimer.start()  // Démarrer le timer de connexion
        }
    }

}
