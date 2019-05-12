/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/


import QtQuick                          2.11
import QtQuick.Controls                 2.4

import QGroundControl                   1.0
import QGroundControl.FlightDisplay     1.0
import QGroundControl.FlightMap         1.0
import QGroundControl.ScreenTools       1.0
import QGroundControl.Controls          1.0
import QGroundControl.Palette           1.0
import QGroundControl.Vehicle           1.0
import QGroundControl.Controllers       1.0

Item {
    id: root
    property double _ar:                QGroundControl.videoManager.aspectRatio
    property bool   _showGrid:          QGroundControl.settingsManager.videoSettings.gridLines.rawValue > 0
    property var    _videoReceiver:     XBStationManager.cameraControl.videoReceiver
    property var    _videoProvider:     XBStationManager.xbVideoProvider
    property var    _cameraControl:     XBStationManager.cameraControl
    property var    _activeVehicle:     QGroundControl.multiVehicleManager.activeVehicle
    property var    _dynamicCameras:    _activeVehicle ? _activeVehicle.dynamicCameras : null
    property bool   _connected:         _activeVehicle ? !_activeVehicle.connectionLost : false
    property int    _curCameraIndex:    _dynamicCameras ? _dynamicCameras.currentCamera : 0
    property bool   _isCamera:          _dynamicCameras ? _dynamicCameras.cameras.count > 0 : false
    property var    _camera:            _isCamera ? _dynamicCameras.cameras.get(_curCameraIndex) : null
    property bool   _hasZoom:           _camera && _camera.hasZoom
    property int    _fitMode:           QGroundControl.settingsManager.videoSettings.videoFit.rawValue
    property var    _links:             XBStationManager.links
    property var    _providerLink:      "image://XBVideoProvider-"+XBStationManager.links[0]+"/camera"
    Rectangle {
        id:             noVideo
        anchors.fill:   parent
        color:          Qt.rgba(0,0,0,0.75)
        visible: true
        //        visible:        !(_cameraControl && _cameraControl.videoRunning)
        QGCLabel {
            text:              (_links.length > 0) ? qsTr("WAITING FOR VIDEO") : qsTr("VIDEO DISABLED")
            font.family:        ScreenTools.demiboldFontFamily
            color:              "white"
            font.pointSize:     _mainIsMap ? ScreenTools.smallFontPointSize : ScreenTools.largeFontPointSize
            anchors.centerIn:   parent
        }
        MouseArea {
            anchors.fill: parent
            onDoubleClicked: {
                QGroundControl.videoManager.fullScreen = !QGroundControl.videoManager.fullScreen
            }
        }
    }
    Rectangle {
        anchors.fill:   parent
        color:          "black"
        visible:        _cameraControl && _cameraControl.videoState == 1
        function getWidth() {
            //-- Fit Width or Stretch
            if(_fitMode === 0 || _fitMode === 2) {
                return parent.width
            }
            //-- Fit Height
            return _ar != 0.0 ? parent.height * _ar : parent.width
        }
        function getHeight() {
            //-- Fit Height or Stretch
            if(_fitMode === 1 || _fitMode === 2) {
                return parent.height
            }
            //-- Fit Width
            return _ar != 0.0 ? parent.width * (1 / _ar) : parent.height
        }
        Image {
            id:             videoContent
            height:         parent.getHeight()
            width:          parent.getWidth()
            anchors.centerIn: parent
            anchors.fill: parent
//            fillMode:       Image.PreserveAspectFit
//            source: "qrc:/qmlimages/cover.jpg"
            visible:        _cameraControl && _cameraControl.videoState == 1
            Connections {
                target: _videoProvider
                onImageChanged: {
                    videoContent.source = "image://XBVideoProvider/camera" + Math.random()
                }
            }
            Rectangle {
                color:  Qt.rgba(1,1,1,0.5)
                height: parent.height
                width:  1
                x:      parent.width * 0.33
                visible: _showGrid && !QGroundControl.videoManager.fullScreen
            }
            Rectangle {
                color:  Qt.rgba(1,1,1,0.5)
                height: parent.height
                width:  1
                x:      parent.width * 0.66
                visible: _showGrid && !QGroundControl.videoManager.fullScreen
            }
            Rectangle {
                color:  Qt.rgba(1,1,1,0.5)
                width:  parent.width
                height: 1
                y:      parent.height * 0.33
                visible: _showGrid && !QGroundControl.videoManager.fullScreen
            }
            Rectangle {
                color:  Qt.rgba(1,1,1,0.5)
                width:  parent.width
                height: 1
                y:      parent.height * 0.66
                visible: _showGrid && !QGroundControl.videoManager.fullScreen
            }

        }
        MouseArea {
            anchors.fill: parent
            onDoubleClicked: {
                QGroundControl.videoManager.fullScreen = !QGroundControl.videoManager.fullScreen
            }
        }
        PinchArea {
            id:             pinchZoom
            enabled:        _hasZoom
            anchors.fill:   parent
            onPinchStarted: pinchZoom.zoom = 0
            onPinchUpdated: {
                if(_hasZoom) {
                    var z = 0
                    if(pinch.scale < 1) {
                        z = Math.round(pinch.scale * -10)
                    } else {
                        z = Math.round(pinch.scale)
                    }
                    if(pinchZoom.zoom != z) {
                        _camera.stepZoom(z)
                    }
                }
            }
            property int zoom: 0
        }
    }
}
