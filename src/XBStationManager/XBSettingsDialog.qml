import QtQuick                      2.3
import QtQuick.Controls             1.2
import QtQuick.Dialogs              1.2
import QtQuick.Layouts              1.2

import QGroundControl               1.0
import QGroundControl.Controls      1.0
import QGroundControl.ScreenTools   1.0
import QGroundControl.FactSystem    1.0
import QGroundControl.FactControls  1.0

QGCViewDialog {
    id: xb_settings_dialog
    property var _cameraControl: XBStationManager.cameraControl
    property var _activeVehicle:       QGroundControl.multiVehicleManager.activeVehicle

    function closeAllConfirm() {
        _cameraControl.showBaudrateConfirm(-2)
        _cameraControl.showPortConfirm(-2)
    }

    QGCFlickable {
        anchors.fill:       parent
        contentHeight:      column.height
        flickableDirection: Flickable.VerticalFlick
        clip:               true
        Column {
            id:             column
            anchors.left:   parent.left
            anchors.right:  parent.right
            spacing:        ScreenTools.defaultFontPixelHeight

            GridLayout {
                anchors.left:   parent.left
                anchors.right:  parent.right
                columns:        2

                QGCLabel {
                    text:               qsTr("Signal Status ")
                }
                QGCLabel {
                    text:               _cameraControl.isSignalConnected ? "Connected" : "No Signal"
                    Layout.fillWidth:   true
                }

                QGCLabel {
                    text:               qsTr("MAVLINK Loss rate")
                }
                QGCLabel {
                    text:               _activeVehicle ? _activeVehicle.mavlinkLossPercent.toFixed(0) + '%' : qsTr("Not Connected")
                    Layout.fillWidth:   true
                }

                QGCLabel { text: qsTr("Camera") }
                QGCButton {
                    text: "Turn "+ (_cameraControl.videoState == 1 ? " OFF" : "ON")
                    Layout.fillWidth:   true
                    enabled: _cameraControl.videoState == 1 || _cameraControl.videoState == 2
                    onClicked: {
                        if(_cameraControl.videoState == 1) {
                            XBStationManager.closeCamera();
                        }
                        else if(_cameraControl.videoState == 2) {
                            XBStationManager.openCamera();
                        }
                    }
                }

                QGCLabel { text: qsTr("Recording") }
                QGCButton {
                    text: "Turn "+ (_cameraControl.recordingState == 1 ? " OFF" : "ON")
                    Layout.fillWidth:   true
                    enabled: _cameraControl.recordingState == 0 || _cameraControl.recordingState == 1
                    onClicked: {
                        if(_cameraControl.recordingState == 1) {
                            XBStationManager.closeRecording();
                        }
                        else if(_cameraControl.recordingState == 0) {
                            XBStationManager.openRecording();
                        }
                    }
                }



                QGCLabel { text: qsTr("Method") }
                QGCComboBox {
                    id              : xb_settings_cam_method
                    model           : _cameraControl.indexControllingMethod >= 0 ? _cameraControl.controllingMethods : ((_cameraControl.indexControllingMethod == -4) ? "SOCKET" : "DGRAM")
                    enabled         : _cameraControl.indexControllingMethod >= 0
                    currentIndex    : _cameraControl.indexControllingMethod
                    Layout.fillWidth: true
                    onCurrentIndexChanged: {
                        if(_cameraControl.indexControllingMethod >= 0) {
                            XBStationManager.changeControlingMethod(xb_settings_cam_method.currentIndex)
                        }
                    }
                }

                QGCLabel { text: qsTr("Video FPV Resolution") }
                QGCComboBox {
                    id              : xb_settings_cam_res
                    Layout.fillWidth: true
                    model           : _cameraControl.resolutions
                    enabled         : _cameraControl.resolutions.length > 1
                    currentIndex    : _cameraControl.currentIndex
                    onCurrentIndexChanged: {
                        if(_cameraControl.resolutions.length > 0) {
                            XBStationManager.changeCameraResolution(xb_settings_cam_res.currentIndex)
                        }
                    }
                }

                QGCLabel { text: qsTr("Video FPV Type") }
                QGCComboBox {
                    id              : xb_settings_cam_type
                    model           : _cameraControl.cameraTypes
                    enabled         : _cameraControl.indexCameraType > -1
                    currentIndex    : _cameraControl.indexCameraType > -1 ? _cameraControl.indexCameraType : 0
                    Layout.fillWidth: true
                    onCurrentIndexChanged: {
                        if(_cameraControl.indexCameraType > -1) {
                            XBStationManager.changeCameraType(xb_settings_cam_type.currentIndex)
                        }
                    }

                }

                QGCLabel { text: qsTr("Serial Baudrate") }
                QGCComboBox {
                    id: xb_settings_pix_baudrate
                    model: _cameraControl.baudrates
                    Layout.fillWidth:   true
                    enabled         : _cameraControl.indexBaudrate > -1
                    currentIndex    : (_cameraControl.indexBaudrate > -1) ? _cameraControl.indexBaudrate : 0
                    onCurrentIndexChanged: {
                        closeAllConfirm();
                        if(xb_settings_pix_baudrate.currentIndex > 0) {
                            _cameraControl.showBaudrateConfirm(xb_settings_pix_baudrate.currentIndex);
                        }
                    }
                }

                QGCLabel { text: qsTr("Serial Port") }
                QGCComboBox {
                    id: xb_settings_pix_port
                    model           : _cameraControl.pixPorts
                    enabled         : _cameraControl.pixPorts.length > 1
                    Layout.fillWidth:   true
                    currentIndex    : _cameraControl.indexPort > -1 ? _cameraControl.indexPort : 0
                    onCurrentIndexChanged: {
                        closeAllConfirm();
                        if(xb_settings_pix_port.currentIndex > 0) {
                            _cameraControl.showPortConfirm(xb_settings_pix_port.currentIndex);
                        }
                    }
                }

                QGCLabel { text: qsTr("Recording resolution") }
                QGCComboBox {
                    id              : xb_settings_recording_resolution
                    Layout.fillWidth: true
                    model           : _cameraControl.recordResolutions
                    enabled         : _cameraControl.recordResolutions.length > 1 && _cameraControl.recordingState !== 1
                    currentIndex    : _cameraControl.currentRecordIndex
                    onCurrentIndexChanged: {
                        if(_cameraControl.recordResolutions.length > 0) {
                            XBStationManager.changeRecordingResolution(xb_settings_recording_resolution.currentIndex)
                        }
                    }
                }

                QGCLabel { text: qsTr("Recording FPS") }
                QGCTextField {
                    id                  : xb_settings_recording_fps
                    Layout.fillWidth    : true
                    enabled             : _cameraControl.recordingState !== 1
//                    inputMethodHints  : Qt.ImhFormattedNumbersOnly
                    validator:              IntValidator {bottom: 5; top: 20;}
                    text                :   _cameraControl.recordingFps
                    onTextChanged: {
                        _cameraControl.recordingFps = parseInt(text)
                    }
                }
            }
        }
    }
    //Pix Port Slider
    XBSliderSwitch {
        id  : slider_serial_port
        width: parent.width
        anchors.bottom : xb_settings_dialog.bottom
        visible: _cameraControl.isPortConfirm
        title: "Serial Port";
        message: "Change to "+xb_settings_pix_port.currentText
        onAccepted: {
            _cameraControl.showPortConfirm(-2)
            XBStationManager.changePixPort(xb_settings_pix_port.currentIndex)
        }
        onCancelled: {
            _cameraControl.showPortConfirm(-1)
        }
    }
    //Pix Baudrate Slider
    XBSliderSwitch {
        id  : slider_serial_baudrate
        width: parent.width
        anchors.bottom : xb_settings_dialog.bottom
        visible: _cameraControl.isBaudrateConfirm
        title: "Serial Baudrate";
        message: "Change to "+xb_settings_pix_baudrate.currentText
        onAccepted: {
            _cameraControl.showBaudrateConfirm(-2)
            XBStationManager.changePixBaudrate(xb_settings_pix_baudrate.currentIndex)
        }
        onCancelled: {
            _cameraControl.showBaudrateConfirm(-1)
        }
    }
}
