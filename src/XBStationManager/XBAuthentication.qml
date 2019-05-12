import QtQuick                      2.3
import QtQuick.Controls             1.2
import QtQuick.Dialogs              1.2
import QtQuick.Layouts              1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0


QGCView {

    width: parent.width
    height: column.height
    function saveSettings() {
        // No need
    }

    Component.onCompleted: {
        if(subEditConfig.username !== "") {
            busyIndicator.visible = true
            loginBtn.visible = false
            XBStationManager.auth(subEditConfig.username)
        }
    }

    Column {
        id:             column
        anchors.horizontalCenter: parent.horizontalCenter
        width: ScreenTools.defaultFontPixelWidth * 42
        spacing:        ScreenTools.defaultFontPixelHeight
        QGCLabel {
            id: error
            color:                  qgcPal.colorRed
            text: ""
            visible: false
        }

        QGCLabel {
            id: success
            color:                  qgcPal.colorGreen
            text: "Logged In. " + ( (subEditConfig && subEditConfig.permission) ? ("Expired date: "+subEditConfig.permission) : "" )
            visible: !loginBtn.visible
        }

        BusyIndicator {
            id: busyIndicator
            visible: false
        }

        GridLayout {
            anchors.left:   parent.left
            anchors.right:  parent.right
            columnSpacing:  10
            rowSpacing:     10
            columns:        2

            QGCLabel {
                text: qsTr("Email")
                visible: loginBtn.visible
            }
            QGCTextField {
                id: username
                visible: loginBtn.visible
                Layout.fillWidth:   true
                text: (subEditConfig) ? subEditConfig.username : ""
                onEditingFinished: {
                    if(subEditConfig) {
                        if(username.text !== "") {
                            subEditConfig.addUsername(username.text)
                        }
                    }
                }
            }

            QGCLabel {
                text: qsTr("Password")
                visible: loginBtn.visible
            }
            QGCTextField {
                id: password
                visible: loginBtn.visible
                echoMode: TextInput.Password
                text: ""
                Layout.fillWidth:   true
            }

            QGCButton {
                id: loginBtn
                text: "Login"
                visible: !((subEditConfig !== null) && (subEditConfig.token !== ""))
                Layout.fillWidth:   true
                onClicked: {
                    if(username.text === "") {
                        error.text = "Email is required"
                        error.visible = true;
                    }
                    else if(password.text === "") {
                        error.text = "Password is required"
                        error.visible = true;
                    }
                    else {
                        XBStationManager.auth(username.text, password.text)
                        error.visible = false
                        busyIndicator.visible = true;
                    }

                }
            }
            QGCButton {
                id: logoutBtn
                text: "LogOut"
                visible: !loginBtn.visible
                Layout.fillWidth:   true
                onClicked: {
                    if(subEditConfig) {
                        subEditConfig.permission = "";
                        subEditConfig.token = "";
                        loginBtn.visible = true;
                        XBStationManager.logOut(subEditConfig.username)
                    }
                }
            }
            QGCLabel {
            }
            ColumnLayout {
                visible: !busyIndicator.visible && !loginBtn.visible
                Layout.fillWidth:   true
                anchors.leftMargin: parent.width/2
                QGCLabel {
                    text: "Connection"
                }
                GridLayout {
                    anchors.left:   parent.left
                    anchors.right:  parent.right
                    columnSpacing:  10
                    rowSpacing:     10
                    columns:        2
                    ExclusiveGroup { id: connecitonGroup }
                    QGCRadioButton {
                        id: xbServerRBtn
                        text: "XBServer"
                        exclusiveGroup: connecitonGroup
                        checked: subEditConfig && subEditConfig.connectionType === "xb"
                        onCheckedChanged: {
                            if(checked) {
                                subEditConfig.connectionType = "xb"
                            }
                        }
                    }
                    QGCRadioButton {
                        id: p2pRBtn
                        text: "VPN"
                        exclusiveGroup: connecitonGroup
                        checked: subEditConfig && subEditConfig.connectionType === "p2p"
                        onCheckedChanged: {
                            if(checked) {
                                subEditConfig.connectionType = "p2p"
                            }
                        }
                    }
                }
                GridLayout {
                    anchors.left:   parent.left
                    anchors.right:  parent.right
                    columnSpacing:  10
                    rowSpacing:     10
                    columns:        2
                    QGCLabel {
                        text: "Firm IP"
                    }
                    QGCTextField {
                        enabled: p2pRBtn.checked
                        text: (subEditConfig) ? subEditConfig.firmIP : ""
                        onEditingFinished: {
                            if(subEditConfig) {
                                if(text !== "") {
                                    subEditConfig.firmIP = text
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: XBStationManager
        onAuthSuccess: {
            if(subEditConfig) {
                subEditConfig.addPermission(permission)
                loginBtn.visible = false
                success.text = "Logged In. Expired date: "+permission
                error.visible = false
                xbServerRBtn.enabled = isXBServer
            }
            else {
                loginBtn.visible = true
                error.text = "Configuration was not prepared"
            }
            busyIndicator.visible = false
        }
        onAuthFail: {
            error.text = "Auth Failed"
            error.visible = true
            success.text = ""
            loginBtn.visible = true
            busyIndicator.visible = false
        }
    }

}
