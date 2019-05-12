/****************************************************************************
 *
 *   (c) 2009-2016 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick              2.3
import QtQuick.Controls     1.2
import QtQuick.Dialogs      1.2
import QtQuick.Layouts      1.2

import QGroundControl               1.0
import QGroundControl.Palette       1.0
import QGroundControl.Controls      1.0
import QGroundControl.Controllers   1.0
import QGroundControl.ScreenTools   1.0

AnalyzePage {
    id:                 logDownloadPage
    pageComponent:      pageComponent
    pageName:           qsTr("XBStation Dropbox")
    pageDescription:    XBStationManager.links.length > 0 ? qsTr("XBStation Dropbox allows you to manage and upload videos from your vehicle to Dropbox. Click Refresh to get list of available videos.") : qsTr("No XBStation available")

    property var dropboxController: XBStationManager.dropboxController
    property real _margin:          ScreenTools.defaultFontPixelWidth
    property real _butttonWidth:    ScreenTools.defaultFontPixelWidth * 10

    QGCPalette { id: palette; colorGroupEnabled: enabled }

    Component {
        id: pageComponent

        ColumnLayout {
            width: availableWidth
            height: availableHeight
            visible: XBStationManager.links.length > 0

            RowLayout {
                width: availableWidth
                QGCLabel {
                    text:               qsTr("Dropbox Access Token")
                }
                QGCTextField {
                    id: textfield_access_token
                    width: availableWidth * 50/100
                    text: dropboxController.accessToken
                }
                QGCButton {
                    text: "Set Token"
                    onClicked: {
                        dropboxController.setAccessToken(textfield_access_token.text)
                    }
                }
                QGCLabel {
                    text:               (dropboxController.accessTokenStatus === 1) ? qsTr("Pending") : ""
                }
            }

            RowLayout {
                width:  availableWidth
                height: availableHeight

                Connections {
                    target: dropboxController
                    onSelectionChanged: {
                        tableView.selection.clear()
                        for(var i = 0; i < dropboxController.model.count; i++) {
                            var o = dropboxController.model.get(i)
                            if (o && o.selected) {
                                tableView.selection.select(i, i)
                            }
                        }
                    }
                }

                TableView {
                    id: tableView
                    Layout.fillHeight:  true
                    model:              dropboxController.model
                    selectionMode:      SelectionMode.MultiSelection
                    Layout.fillWidth:   true

                    TableViewColumn {
                        title: qsTr("Id")
                        width: availableWidth * 5/100
                        horizontalAlignment: Text.AlignHCenter
                        delegate : Text  {
                            horizontalAlignment: Text.AlignHCenter
                            text: {
                                if(styleData.row >= 0) {
                                    var o = dropboxController.model.get(styleData.row)
                                    return o ? o.id : ""
                                }
                                return "";
                            }
                        }
                    }

                    TableViewColumn {
                        title: qsTr("Name")
                        width: availableWidth * 15/100
                        horizontalAlignment: Text.AlignHCenter
                        delegate : Text  {
                            text: {
                                if(styleData.row >= 0) {
                                    var o = dropboxController.model.get(styleData.row)
                                    return o ? o.name : ""
                                }
                                return "";
                            }
                        }
                    }

                    TableViewColumn {
                        title: qsTr("Path")
                        width: availableWidth * 35/100
                        horizontalAlignment: Text.AlignHCenter
                        delegate : Text  {
                            text: {
                                if(styleData.row >= 0) {
                                    var o = dropboxController.model.get(styleData.row)
                                    return o ? o.path : ""
                                }
                                return "";
                            }
                        }
                    }

                    TableViewColumn {
                        title: qsTr("Size")
                        width: availableWidth * 15/100
                        horizontalAlignment: Text.AlignHCenter
                        delegate : Text  {
                            text: {
                                if(styleData.row >= 0) {
                                    var o = dropboxController.model.get(styleData.row)
                                    return o ? ((o.size/1048576).toFixed(2) + " MB") : ""
                                }
                                return "";
                            }
                        }
                    }

                    TableViewColumn {
                        title: qsTr("Status")
                        width: availableWidth * 30/100
                        horizontalAlignment: Text.AlignHCenter
                        delegate : Text  {
                            text: {
                                if(styleData.row >= 0) {
                                    var o = dropboxController.model.get(styleData.row)
                                    return o ? o.status : ""
                                }
                                return "";
                            }
                        }
                    }
                }

                Column {
                    spacing:            _margin
                    Layout.alignment:   Qt.AlignTop | Qt.AlignLeft

                    QGCButton {
                        enabled:    !dropboxController.requestingList && !dropboxController.downloadingLogs
                        text:       qsTr("Refresh")
                        width:      _butttonWidth

                        onClicked: {
                            dropboxController.refresh()
                        }
                    }

                    QGCButton {
                        enabled:    !dropboxController.requestingList && !dropboxController.downloadingLogs && tableView.selection.count > 0
                        text:       qsTr("Upload")
                        width:      _butttonWidth
                        onClicked: {
                            tableView.selection.forEach(function(rowIndex){
                                dropboxController.upload(rowIndex)
                            })
                        }
                    }

                    QGCButton {
                        text:       qsTr("Remove")
                        width:      _butttonWidth
                        enabled:    !dropboxController.requestingList && !dropboxController.downloadingLogs && tableView.selection.count > 0
                        onClicked: {
                            tableView.selection.forEach(function(rowIndex){
                                dropboxController.removeFile(rowIndex)
                            })
                        }
                    }

                } // Column - Buttons
            } // RowLayout
        }

    } // Component
} // AnalyzePage
