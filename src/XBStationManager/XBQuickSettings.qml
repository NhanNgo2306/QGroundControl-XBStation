import QtQuick                      2.3
import QtQuick.Controls             1.2
import QtQuick.Dialogs              1.2
import QtQuick.Layouts              1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0

QGCFlickable {

    signal showXBSettingsDialog();
    property var _cameraControl : XBStationManager.cameraControl

    anchors.fill:       parent
    contentHeight:      column.height
    flickableDirection: Flickable.VerticalFlick
    clip:               true
    anchors.top: parent.top

    Column {
        id:             column
        anchors.left:   parent.left
        width: parent.width - 10
        spacing:        ScreenTools.defaultFontPixelHeight

        QGCComboBox {
            Layout.fillWidth:   true
            model           :   XBStationManager.links
            anchors.left:   parent.left
            anchors.right:  parent.right
            onCurrentIndexChanged: XBStationManager.changeStation(currentIndex)

        }

        GridLayout {
            anchors.left:   parent.left
            anchors.right:  parent.right
            columns:        2



            QGCLabel {
                text: "Recv (MB)"
                color: "white"

            }
            QGCLabel {
                text:               (_cameraControl !== null) ? _cameraControl.byteRecv : ""
                Layout.fillWidth:   true
                color: "white"

            }

            QGCLabel {
                text: qsTr("Sent (KB)")
                color: "white"

            }
            QGCLabel {
                text:               (_cameraControl !== null) ? _cameraControl.byteSent : ""
                Layout.fillWidth:   true
                color: "white"

            }

            QGCLabel {
                text: qsTr("Camera")
                color: "white"
            }
            QGCButton {
                text: "Turn "+ ((_cameraControl !== null && _cameraControl.videoState === 1) ? " OFF" : "ON")
                Layout.fillWidth:   true
                onClicked: {
                    if(_cameraControl.videoState == 1)
                        XBStationManager.closeCamera()
                    else {
                        XBStationManager.openCamera()
                    }
                }
            }

            QGCLabel {
                text: qsTr("Recording")
                color: "white"
            }
            QGCButton {
                text: "Turn "+ ((_cameraControl !== null &&_cameraControl.recordingState == 1) ? " OFF" : "ON")
                Layout.fillWidth:   true
                enabled:  _cameraControl != null && (_cameraControl.recordingState == 0 || _cameraControl.recordingState == 1)
                onClicked: {
                    if(_cameraControl.recordingState == 1) {
                        XBStationManager.closeRecording();
                    }
                    else if(_cameraControl.recordingState == 0) {
                        XBStationManager.openRecording();
                    }
                }
            }

            QGCLabel {
                text: qsTr("Method")
                color: "white"
            }
            QGCComboBox {
                id              : xb_settings_cam_method
                model           : (_cameraControl !== null &&_cameraControl.indexControllingMethod >= 0) ? _cameraControl.controllingMethods : ((_cameraControl !== null && _cameraControl.indexControllingMethod == -4) ? "SOCKET" : "DGRAM")
                enabled         : _cameraControl != null && _cameraControl.indexControllingMethod >= 0
                currentIndex    : _cameraControl != null ? _cameraControl.indexControllingMethod : -1
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    if(_cameraControl !== null && _cameraControl.indexControllingMethod >= 0) {
                        XBStationManager.changeControlingMethod(xb_settings_cam_method.currentIndex)
                    }
                }
            }

            QGCLabel {
                text: qsTr("Type")
                color: "white"
            }

            QGCComboBox {
                id              : xb_settings_cam_type
                model           : (_cameraControl !== null &&_cameraControl.indexCameraType > -1 ) ?_cameraControl.cameraTypes : ""
                enabled         : _cameraControl != null && _cameraControl.indexCameraType > -1
                currentIndex    : _cameraControl != null ? _cameraControl.indexCameraType : -1
                Layout.fillWidth: true
                onCurrentIndexChanged: {
                    if(_cameraControl.indexCameraType > -1) {
                        XBStationManager.changeCameraType(xb_settings_cam_type.currentIndex)
                    }
                }

            }

            QGCLabel {
                text: qsTr("Resolution")
                color: "white"
            }
            QGCComboBox {
                id              : xb_settings_cam_res
                Layout.fillWidth: true
                model           : _cameraControl != null ? _cameraControl.resolutions : ""
                enabled         : _cameraControl != null && _cameraControl.resolutions.length > 1
                currentIndex    : _cameraControl != null ? _cameraControl.currentIndex : 0
                onCurrentIndexChanged: {
                    if(_cameraControl != null &&_cameraControl.resolutions.length > 0) {
                        XBStationManager.changeCameraResolution(xb_settings_cam_res.currentIndex)
                    }
                }
            }

        }
        QGCButton {
            Layout.fillWidth:   true
            text: "Full Settings"
            onClicked: showXBSettingsDialog()
        }

    }
}
