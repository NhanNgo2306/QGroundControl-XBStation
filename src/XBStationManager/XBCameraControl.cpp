#include "XBCameraControl.h"
#include <QDebug>
#include "xbstation.h"

XBCameraControl::XBCameraControl(QObject *parent) : QObject(parent)
{
    connect(&_xbMessageHandler, SIGNAL(textMessageCountChanged(int)),  this, SLOT(_handleTextMessage(int)));
    connect(&_xbMessageHandler, SIGNAL(textMessageReceived(XBMessage*)), this, SLOT(_handletextMessageReceived(XBMessage*)));
}

void XBCameraControl::showPortConfirm(int indexPixPort)
{

    if(indexPixPort == -1) {
        _isPortConfirm = false;
        emit indexPortChanged();
    }
    else if(indexPixPort == -2) {
        _isPortConfirm = false;
    }
    else {
        _isPortConfirm = !(indexPixPort == _indexPort);
    }
    emit isPortConfirmChanged();
}

void XBCameraControl::showBaudrateConfirm(int indexBaudrate)
{
    if(indexBaudrate == -1) {
        _isBaudrateConfirm = false;
        emit this->indexBaudrateChanged();
    }
    else if(indexBaudrate == -2) {
        _isBaudrateConfirm = false;
    }
    else {
        _isBaudrateConfirm = !(indexBaudrate == _indexBaudrate);
    }
    emit isBaudrateConfirmChanged();
}

void XBCameraControl::resetMessages()
{
    // Reset Counts
    int count = _currentMessageCount;
    MessageType_t type = _currentMessageType;
    _currentErrorCount   = 0;
    _currentWarningCount = 0;
    _currentNormalCount  = 0;
    _currentMessageCount = 0;
    _currentMessageType = MessageNone;
    if(count != _currentMessageCount) {
        emit newMessageCountChanged();
    }
    if(type != _currentMessageType) {
        emit messageTypeChanged();
    }
}

void XBCameraControl::clearMessages()
{
    _xbMessageHandler.clearMessages();
}

QString XBCameraControl::formatedMessages()
{
    QString messages;
    for(XBMessage* message: _xbMessageHandler.messages()) {
        messages += message->getFormatedText();
    }
    return messages;
}

void XBCameraControl::_handletextMessageReceived(XBMessage *message)
{
    if(message)
    {
        _formatedMessage = message->getFormatedText();
        emit formatedMessageChanged();
        emit showXBMessage();
    }
}

void XBCameraControl::_handleTextMessage(int newCount)
{
    // Reset?
    if(!newCount) {
        _currentMessageCount = 0;
        _currentNormalCount  = 0;
        _currentWarningCount = 0;
        _currentErrorCount   = 0;
        _messageCount        = 0;
        _currentMessageType  = MessageNone;
        emit newMessageCountChanged();
        emit messageTypeChanged();
        emit messageCountChanged();
        return;
    }

    MessageType_t type = newCount ? _currentMessageType : MessageNone;
    int errorCount     = _currentErrorCount;
    int warnCount      = _currentWarningCount;
    int normalCount    = _currentNormalCount;
    //-- Add current message counts
    errorCount  += _xbMessageHandler.getErrorCount();
    warnCount   += _xbMessageHandler.getWarningCount();
    normalCount += _xbMessageHandler.getNormalCount();
    //-- See if we have a higher level
    if(errorCount != _currentErrorCount) {
        _currentErrorCount = errorCount;
        type = MessageError;
    }
    if(warnCount != _currentWarningCount) {
        _currentWarningCount = warnCount;
        if(_currentMessageType != MessageError) {
            type = MessageWarning;
        }
    }
    if(normalCount != _currentNormalCount) {
        _currentNormalCount = normalCount;
        if(_currentMessageType != MessageError && _currentMessageType != MessageWarning) {
            type = MessageNormal;
        }
    }
    int count = _currentErrorCount + _currentWarningCount + _currentNormalCount;
    if(count != _currentMessageCount) {
        _currentMessageCount = count;
        // Display current total new messages count
        emit newMessageCountChanged();
    }
    if(type != _currentMessageType) {
        _currentMessageType = type;
        // Update message level
        emit messageTypeChanged();
    }
    // Update message count (all messages)
    if(newCount != _messageCount) {
        _messageCount = newCount;
        emit messageCountChanged();
    }
    QString errMsg = _xbMessageHandler.getLatestError();
    if(errMsg != _latestError) {
        _latestError = errMsg;
        emit latestErrorChanged();
    }
}

void XBCameraControl::handleMessage(QString type, QString text)
{
    _xbMessageHandler.handleTextMessage(type, text);
}

void XBCameraControl::handleSignalStatus(bool isConnected)
{
    _isSignalConnected = isConnected;
    emit this->signalStatusChanged();
}

void XBCameraControl::handlePixPort(QString portName)
{
    _indexPort = _pixPorts.indexOf(portName);
    emit indexPortChanged();
}

void XBCameraControl::handleCameraStatus(int status)
{
    if(status != _videoState) {
        _videoState = status;
        emit infoChanged();
    }

}

void XBCameraControl::handleRecordingStatus(int status)
{
    if(status != _recordingState) {
        _recordingState = status;
        emit infoChanged();
    }
}

void XBCameraControl::handleResolutions(QStringList res, int currentIndex)
{
    res.prepend("");
    _resolutions = res;
    _recordResolutions = res;
    emit recordResolutionsChanged();
    emit resolutionsChanged();

    _currentRecordingIndex = currentIndex + 1;
    _currentIndex = currentIndex + 1;
    emit currentIndexChanged();

}

void XBCameraControl::handleByteReceive(int amount)
{
    _amountRecv += amount;
    _byteRecv = QString::number(_amountRecv / 1048576.0, 'f', 2);
    emit this->byteRecvChanged();
}

void XBCameraControl::handleByteSent(int amount)
{
    _amountSent += amount;
    _byteSent = QString::number(_amountSent / 1024.0, 'f', 2);
    emit this->byteSentChanged();
}

void XBCameraControl::handleCameraType(int type)
{
    _indexCameraType = type;
    emit indexCameraTypeChanged();
}

void XBCameraControl::handleStationControllingConnected(int type)
{
    if(type == XBStation::TYPE_CONTROLLING_SOCKET && _indexControllingMethod != -4 && _indexControllingMethod < 0) {
        _indexControllingMethod += 3;
    }
    if(type == XBStation::TYPE_CONTROLLING_DGRAM && _indexControllingMethod != -3 && _indexControllingMethod < 0) {
        _indexControllingMethod += 4;
    }
    if(_indexControllingMethod >= 0 && _isTheFirstConnect) {
        _isTheFirstConnect = false;
        _indexControllingMethod = 1;
    }
    emit indexControllingMethodChanged();
}

void XBCameraControl::handleStationControllingDisconnected(int type)
{
    if(type == XBStation::TYPE_CONTROLLING_SOCKET) {
        _indexControllingMethod = -3;
    }
    if(type == XBStation::TYPE_CONTROLLING_DGRAM) {
        _indexControllingMethod = -4;
    }
}

void XBCameraControl::handlePixPorts(QStringList availablePorts, QString currentPixPort)
{
    for(int i = 0; i < availablePorts.size(); i++) {
        if(!_pixPorts.contains(availablePorts.at(i))) {
            _pixPorts.append(availablePorts.at(i));
        }
    }
    emit pixPortsChanged();
    _indexPort = _pixPorts.indexOf(currentPixPort);
    emit indexPortChanged();

}

void XBCameraControl::handlePixBaudrate(QString baudrate)
{
    _indexBaudrate = _baudrates.indexOf(baudrate);
    emit indexBaudrateChanged();
}

void XBCameraControl::handleResolutionChanged(int res)
{
    _currentIndex = res + 1;
    emit this->currentIndexChanged();
}

bool XBCameraControl::changeCameraType(int type)
{
    if(type != _indexCameraType) {
        _indexCameraType = type;
        return true;
    }
    return false;
}

bool XBCameraControl::changeControllingMethod(int type)
{
    if(_indexControllingMethod != type) {
        _indexControllingMethod = type;
        emit this->indexControllingMethodChanged();
        return true;
    }
    return false;

}

bool XBCameraControl::changeCameraResolution(int res)
{
    if(res == _currentIndex)
        return false;
    _currentIndex = res;
    return  true;
}

QString XBCameraControl::changePixPort(int indexPixPort)
{
    if(indexPixPort == _indexPort)
        return "";
    _indexPort = indexPixPort;
    return _pixPorts[indexPixPort];
}

QString XBCameraControl::changePixBaudrate(int indexBaudrate)
{
    if(indexBaudrate == _indexBaudrate)
        return "";
    _indexBaudrate = indexBaudrate;
    return _baudrates[indexBaudrate];
}
