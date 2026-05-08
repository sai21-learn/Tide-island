import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import quickshell

Item {
    id: root
    implicitWidth: 300
    implicitHeight: 400

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Text {
            text: "Island Settings"
            font.pixelSize: 18
            font.bold: true
            color: "#ffffff"
        }

        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#33ffffff"
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 10

                // Example settings
                SwitchDelegate {
                    Layout.fillWidth: true
                    text: "Show Battery Percentage"
                    checked: true
                    onCheckedChanged: console.log("Battery toggle:", checked)
                }

                SwitchDelegate {
                    Layout.fillWidth: true
                    text: "Enable Lyrics"
                    checked: true
                }

                Text {
                    text: "Log History"
                    font.bold: true
                    color: "#aaaaaa"
                    Layout.topMargin: 10
                }

                ListView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    model: Logger.logs
                    delegate: Text {
                        text: modelData
                        color: "#88ffffff"
                        font.pixelSize: 10
                        width: parent.width
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}
