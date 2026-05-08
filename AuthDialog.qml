import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import Quickshell
import Quickshell.Hyprland

ShellWindow {
    id: authWindow
    
    property string promptText: "Authentication Required"
    property string infoText: "Please enter your password to perform this action."
    property var onAccepted: null
    property var onRejected: null
    
    // Position it in the center of the focused monitor
    readonly property var targetMonitor: Hyprland.focusedMonitor
    
    width: screen.width
    height: screen.height
    color: "transparent"
    
    WlrLayershell.layer: WlrLayer.Overlay
    WlrLayershell.keyboardFocus: WlrKeyboardFocus.Exclusive
    
    // Background dim
    Rectangle {
        anchors.fill: parent
        color: "#aa000000"
        
        MouseArea {
            anchors.fill: parent
            onClicked: authWindow.reject()
        }
    }
    
    // Dialog box
    Rectangle {
        anchors.centerIn: parent
        width: 350
        height: 220
        radius: 16
        color: "#212226"
        border.color: "#33ffffff"
        border.width: 1
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 24
            spacing: 16
            
            Text {
                text: authWindow.promptText
                color: "#ffffff"
                font.pixelSize: 20
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }
            
            Text {
                text: authWindow.infoText
                color: "#bbbbbb"
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }
            
            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: "Password"
                echoMode: TextInput.Password
                focus: true
                color: "#ffffff"
                
                background: Rectangle {
                    color: "#11ffffff"
                    radius: 8
                    border.color: passwordField.activeFocus ? "#55ffffff" : "transparent"
                }
                
                onAccepted: authWindow.accept()
                
                Component.onCompleted: forceActiveFocus()
            }
            
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                
                Button {
                    Layout.fillWidth: true
                    text: "Cancel"
                    onClicked: authWindow.reject()
                    
                    background: Rectangle {
                        color: "#11ffffff"
                        radius: 8
                    }
                    palette.buttonText: "#ffffff"
                }
                
                Button {
                    Layout.fillWidth: true
                    text: "Confirm"
                    onClicked: authWindow.accept()
                    
                    background: Rectangle {
                        color: "#3d5afe" // Premium blue
                        radius: 8
                    }
                    palette.buttonText: "#ffffff"
                }
            }
        }
    }
    
    function accept() {
        if (onAccepted) onAccepted(passwordField.text);
        authWindow.destroy();
    }
    
    function reject() {
        if (onRejected) onRejected();
        authWindow.destroy();
    }
    
    Keys.onEscapePressed: reject()
}
