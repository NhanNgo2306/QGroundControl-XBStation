#include "XBLink.h"
#include <QDebug>
#include "QGCApplication.h"
#include "AutoConnectSettings.h"
#include "SettingsManager.h"
#include "XBStationManager.h"
#include <QPixmap>
#include "XBVideoProvider.h"
//#include "MainWindow.h"

bool XBLink::isConnected() const
{
    return _xbStation != nullptr && _xbStation->isConnected();
}

QString XBLink::getName() const
{
    return (_xbStation != nullptr) ? _xbStation->getName() : "XBLink";
}

qint64 XBLink::getConnectionSpeed() const
{
    return 54000000;
}

qint64 XBLink::getCurrentInDataRate() const
{
    return 0;
}

qint64 XBLink::getCurrentOutDataRate() const
{
    return 0;
}

void XBLink::changePassword()
{
    emit _xbStation->changePassword(_xbConfig->username());
}

void XBLink::run()
{

}


bool XBLink::disconnect()
{
    //    emit this->xbstation->disconnectStation();
    //    wait();
    //    qDebug()<< "XBlink::disconnect";
    _xbConfig->setState("Disconnected");
    emit disconnected();
    return true;
}

void XBLink::readBytes(QByteArray buffer)
{   
    if(buffer.size()) {
        emit bytesReceived(this, buffer);
    }
}

void XBLink::_writeBytes(const QByteArray data)
{
    emit _xbStation->sendControllingData(data);
}

void XBLink::handleEnabled(bool enabled)
{
    for(MavlinkMessagesTimer* timer: _mavlinkMessagesTimers) {
        qDebug() << timer->getVehicleID();
    }
    if(enabled) {
        QThread::connect(_xbStation, SIGNAL(onPixData(QByteArray)), this, SLOT(readBytes(QByteArray)));
        QThread::connect(_xbStation, SIGNAL(onCamStatus(int)), &_xbCameraControl, SLOT(handleCameraStatus(int)));
        QThread::connect(_xbStation, SIGNAL(onResolutions(QStringList, int)), &_xbCameraControl, SLOT(handleResolutions(QStringList, int)));
        QThread::connect(_xbStation, SIGNAL(onByteReceive(int)), &_xbCameraControl, SLOT(handleByteReceive(int)));
        QThread::connect(_xbStation, SIGNAL(onByteSend(int)), &_xbCameraControl, SLOT(handleByteSent(int)));
        QThread::connect(_xbStation, SIGNAL(onCameraType(int)), &_xbCameraControl, SLOT(handleCameraType(int)));
        QThread::connect(_xbStation, SIGNAL(onResolutionChanged(int)), &_xbCameraControl, SLOT(handleResolutionChanged(int)));
        QThread::connect(_xbStation, SIGNAL(onPixPorts(QStringList, QString)), &_xbCameraControl, SLOT(handlePixPorts(QStringList, QString)));
        QThread::connect(_xbStation, SIGNAL(onPixPort(QString)), &_xbCameraControl, SLOT(handlePixPort(QString)));
        QThread::connect(_xbStation, SIGNAL(onPixBaudrate(QString)), &_xbCameraControl, SLOT(handlePixBaudrate(QString)));
        QThread::connect(_xbStation, SIGNAL(onRecordingStatus(int)), &_xbCameraControl, SLOT(handleRecordingStatus(int)));
        QThread::connect(_xbStation, SIGNAL(onSignalStatus(bool)), &_xbCameraControl, SLOT(handleSignalStatus(bool)));
    }
    else {
        QThread::disconnect(_xbStation, SIGNAL(onPixData(QByteArray)), this, SLOT(readBytes(QByteArray)));
        QThread::disconnect(_xbStation, SIGNAL(onCamStatus(int)), &_xbCameraControl, SLOT(handleCameraStatus(int)));
        QThread::disconnect(_xbStation, SIGNAL(onResolutions(QStringList, int)), &_xbCameraControl, SLOT(handleResolutions(QStringList, int)));
        QThread::disconnect(_xbStation, SIGNAL(onByteReceive(int)), &_xbCameraControl, SLOT(handleByteReceive(int)));
        QThread::disconnect(_xbStation, SIGNAL(onByteSend(int)), &_xbCameraControl, SLOT(handleByteSent(int)));
        QThread::disconnect(_xbStation, SIGNAL(onCameraType(int)), &_xbCameraControl, SLOT(handleCameraType(int)));
        QThread::disconnect(_xbStation, SIGNAL(onResolutionChanged(int)), &_xbCameraControl, SLOT(handleResolutionChanged(int)));
        QThread::disconnect(_xbStation, SIGNAL(onPixPorts(QStringList, QString)), &_xbCameraControl, SLOT(handlePixPorts(QStringList, QString)));
        QThread::disconnect(_xbStation, SIGNAL(onPixPort(QString)), &_xbCameraControl, SLOT(handlePixPort(QString)));
        QThread::disconnect(_xbStation, SIGNAL(onPixBaudrate(QString)), &_xbCameraControl, SLOT(handlePixBaudrate(QString)));
        QThread::disconnect(_xbStation, SIGNAL(onRecordingStatus(int)), &_xbCameraControl, SLOT(handleRecordingStatus(int)));
        QThread::disconnect(_xbStation, SIGNAL(onSignalStatus(bool)), &_xbCameraControl, SLOT(handleSignalStatus(bool)));
    }
}

void XBLink::onDisconnectSuccess()
{
    qDebug() << "Disconnect succeess";
    //    emit disconnected();
    emit this->disconnected();
    emit this->onDisconnected(_xbConfig->name());
}

void XBLink::onDisconnectFail()
{

}

void XBLink::handleConnectSuccess()
{
//    QThread::disconnect(_xbStation, SIGNAL(onPrepareDone()), this, SLOT(handlePrepareDone()));
    QThread::disconnect(_xbStation, SIGNAL(onConnectSuccess()), this, SLOT(handleConnectSuccess()));
    QThread::disconnect(_xbStation, SIGNAL(onConnectFail(QString)), this, SLOT(handleConnectFail(QString)));
    emit LinkInterface::connected();
    _xbConfig->setState("Connected");
    qgcApp()->toolbox()->xbStationManager()->appendLink(_xbConfig->name(), this);
    emit _xbStation->getCameraResolutions();
    emit _xbStation->getPixPorts();
}

void XBLink::handleConnectFail(QString error)
{
    QThread::disconnect(_xbStation, SIGNAL(onConnectSuccess()), this, SLOT(handleConnectSuccess()));
    QThread::disconnect(_xbStation, SIGNAL(onConnectFail(QString)), this, SLOT(handleConnectFail(QString)));
    _xbConfig->setState("Failed: "+error);
    emit onDisconnected(_xbConfig->name());
}

void XBLink::handleStreamingData(QByteArray buffer)
{

}

void XBLink::handlePermission(int timeStamp)
{
    _xbConfig->setState("Connected - Exp: "+QDateTime::fromTime_t(timeStamp).toString("yyyy-MM-dd"));
}

XBLink::XBLink(SharedLinkConfigurationPointer& config)
    : LinkInterface(config)
    #if defined(QGC_ZEROCONF_ENABLED)
    , _dnssServiceRef(NULL)
    #endif
    , _running(false)
    , _xbStation(NULL)
    , _xbConfig(qobject_cast<XBConfiguration*>(config.data()))
    , _connectState(false)
{
    QThread::connect(this, SIGNAL(enabled(bool)), this, SLOT(handleEnabled(bool)));
    if (!_xbConfig) {
        qWarning() << "Internal error";
    }
}
XBLink::~XBLink()
{
    _xbConfig->setState("Disconnected");
    emit _xbStation->stopConnection();
    emit onDisconnected(_xbConfig->name());
    _running = false;
    quit();
    wait();
    this->deleteLater();
}

bool XBLink::_connect()
{
    if(!_xbStation) {
        _xbStation = new XBStation();
//        QThread::connect(_xbStation, SIGNAL(onPrepareDone()), this, SLOT(handlePrepareDone()));
        QThread::connect(_xbStation, SIGNAL(onConnectSuccess()), this, SLOT(handleConnectSuccess()));
        QThread::connect(_xbStation, SIGNAL(onConnectFail(QString)), this, SLOT(handleConnectFail(QString)));
        QThread::connect(_xbStation, SIGNAL(onPermission(int)), this, SLOT(handlePermission(int)));
        QThread::connect(_xbStation, SIGNAL(onTextMessageReceived(QString, QString)), &_xbCameraControl, SLOT(handleMessage(QString, QString)));
        QThread::connect(_xbStation, SIGNAL(onStationControllingConnected(int)), &_xbCameraControl, SLOT(handleStationControllingConnected(int)));
        QThread::connect(_xbStation, SIGNAL(onStationControllingDisconnected(int)), &_xbCameraControl, SLOT(handleStationControllingDisconnected(int)));
    }
    if(this->_xbConfig->connectionType() == "xb") {
        _xbConfig->setState("Connecting to XBServer...");
        emit _xbStation->start(_xbConfig->username());
    }
    else if(this->_xbConfig->connectionType() == "p2p") {
        if(_xbConfig->firmIP() == "") {
            _xbConfig->setState("Failed: Firm IP is empty");
            emit onDisconnected(_xbConfig->name());
            emit onDisconnected(_xbConfig->name());
            return false;
        }
        _xbConfig->setState("Connecting to "+_xbConfig->firmIP()+"...");
        emit _xbStation->startPeerToPeer(_xbConfig->username(), _xbConfig->firmIP());

    }
    else {
        _xbConfig->setState("Failed: Please choose connection type");
        emit onDisconnected(_xbConfig->name());
        return false;
    }
    return true;
}

void XBLink::_disconnect()
{

}

void XBLink::_restartConnection()
{
}

XBConfiguration::XBConfiguration(const QString &name) : LinkConfiguration(name)
{
    AutoConnectSettings* settings = qgcApp()->toolbox()->settingsManager()->autoConnectSettings();
    this->_username = settings->xbUsername()->rawValue().toString();
    this->_token = settings->xbToken()->rawValue().toString();
    this->_permission = settings->xbPermission()->rawValue().toString();
    this->_connectionType = settings->xbConnectionType()->rawValue().toString();
    this->_firmIP = settings->xbFirmIP()->rawValue().toString();
}

XBConfiguration::XBConfiguration(XBConfiguration *source) : LinkConfiguration(source)
{
    _copyFrom(source);

}

XBConfiguration::~XBConfiguration()
{

}

void XBConfiguration::addUsername(const QString username)
{
    this->_username = username;
}

void XBConfiguration::addToken(const QString token)
{
    _token = token;
}

void XBConfiguration::addPermission(const QString permission)
{
    _permission = permission;
}


void XBConfiguration::_copyFrom(LinkConfiguration *source)
{
    XBConfiguration* xbsource = dynamic_cast<XBConfiguration*>(source);
    if (xbsource) {
        _username = xbsource->username();
        _token  = xbsource->token();
        _permission = xbsource->permission();
        _connectionType = xbsource->connectionType();
        _firmIP = xbsource->firmIP();
    }
}

void XBConfiguration::loadSettings(QSettings &settings, const QString &root)
{
    AutoConnectSettings* acSettings = qgcApp()->toolbox()->settingsManager()->autoConnectSettings();
    settings.beginGroup(root);
    _username = settings.value("username", "").toString();
    _token = settings.value("token", "").toString();
    _permission = settings.value("permission").toString();
    _connectionType = settings.value("connection_type").toString();
    _firmIP = settings.value("firm_ip").toString();
    settings.endGroup();
}

void XBConfiguration::saveSettings(QSettings &settings, const QString &root)
{
    settings.beginGroup(root);
    settings.setValue("username", _username);
    settings.setValue("token", _token);
    settings.setValue("permission", _permission);
    settings.setValue("connection_type", _connectionType);
    settings.setValue("firm_ip", _firmIP);
    settings.endGroup();
}

void XBConfiguration::setUsername(QString username)
{
    this->_username = username;
}

void XBConfiguration::setToken(QString token)
{
    this->_token = token;
}

void XBConfiguration::setState(QString state)
{
    this->_state = state;
    emit this->stateChanged();
}

void XBConfiguration::setPermission(QString permission)
{
    this->_permission = permission;
    emit this->permissionChanged();
}

void XBConfiguration::setConnectionType(QString connectionType)
{
    this->_connectionType = connectionType;
    emit this->connectionTypeChanged();
}

void XBConfiguration::setFirmIP(QString firmIP)
{
    _firmIP = firmIP;
    emit this->firmIPChanged();
}

void XBConfiguration::updateSettings()
{

}

void XBConfiguration::copyFrom(LinkConfiguration *source)
{
    LinkConfiguration::copyFrom(source);
    _copyFrom(source);
}
