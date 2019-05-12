#include "XBStationManager.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "xbstation.h"
#include "camera/camera_provider.h"
#include <QDateTime>

QGC_LOGGING_CATEGORY(XBStationManagerLog, "XBStationManagerLog")

XBStationManager::XBStationManager(QGCApplication* app, QGCToolbox* toolbox) : QGCTool(app, toolbox)
{
    _cameraControl = new XBCameraControl();
    _dropboxController = new XBDropboxController();
    this->connect(_dropboxController, SIGNAL(pendingAccessToken(QString)), this, SLOT(handlePendingAccessToken(QString)));
    this->connect(_dropboxController, SIGNAL(uploadFileToDropbox(QString)), this, SLOT(handleUploadFileToDropbox(QString)));
    this->connect(_dropboxController, SIGNAL(getLocalFiles()), this, SLOT(handleGetDopboxLocalFiles()));
    this->connect(_dropboxController, SIGNAL(remove(QString)), this, SLOT(handleRemoveDropboxLocalFile(QString)));
    _tmpXBStation = new XBStation();
}

XBStationManager::~XBStationManager()
{
    if(this->cameraControl() != nullptr)
        delete this->cameraControl();
    deleteLater();
}

void XBStationManager::openCamera()
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->openCamera();
}

void XBStationManager::openRecording()
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->openRecording(_cameraControl->currentRecordIndex(), _cameraControl->recordingFps());
}

void XBStationManager::closeRecording()
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->closeRecording();
}

void XBStationManager::changeControlingMethod(int index)
{
    if(_cameraControl->changeControllingMethod(index)) {
        if(_cameraControl->controllingMethods().at(index) == "SOCKET") {
            emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changeControllingMethod(XBStation::TYPE_CONTROLLING_SOCKET);
        }
        else if(_cameraControl->controllingMethods().at(index) == "DGRAM") {
            emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changeControllingMethod(XBStation::TYPE_CONTROLLING_DGRAM);
        }
    }
}

void XBStationManager::changeCameraType(int type)
{
    if(_cameraControl->changeCameraType(type)) {
        if(type ==0) emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changeCameraType(XBStation::ID_CAM_TYPE_RGB);
        else if(type == 1) emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changeCameraType(XBStation::ID_CAM_TYPE_GRAYSCALE);
    }
}

void XBStationManager::changeCameraResolution(int resolution)
{
    if(_cameraControl->changeCameraResolution(resolution) && resolution > 0) {
        resolution = (resolution - 1 >= 0) ? resolution - 1 : 0 ;
        emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changeCameraResolution(resolution);
    }
}

void XBStationManager::changeRecordingResolution(int recordResolution)
{
    if(recordResolution > 0) {
        _cameraControl->setCurrentRecordIndex(recordResolution - 1);
    }
}

void XBStationManager::closeCamera()
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->closeCamera();
}

void XBStationManager::prepare()
{
    if(_cameraControl->resolutions().size() == 0 || _cameraControl->indexCameraType() == -1) {
        emit _linkList.value(_links[_currentLinkIndex])->xbStation()->getCameraResolutions();
    }
    if(_cameraControl->pixPorts().size() == 0 || _cameraControl->indexBaudrate() == -1) {
        emit _linkList.value(_links[_currentLinkIndex])->xbStation()->getPixPorts();
    }

}

void XBStationManager::changePixPort(int indexPort)
{
    QString pixPort = _cameraControl->changePixPort(indexPort);
    if(pixPort != "") {
        emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changePixPort(pixPort);
    }
}

void XBStationManager::changePixBaudrate(int indexBaudrate)
{
    if(indexBaudrate > 0) {
        QString pixBaudrate = _cameraControl->changePixBaudrate(indexBaudrate);
        if(pixBaudrate != "") {
            emit _linkList.value(_links[_currentLinkIndex])->xbStation()->changePixBaudrate(pixBaudrate);
        }
    }
}

void XBStationManager::changeStation(int index)
{
    if(_links.length() > 0 ){
        _dropboxController->reset();
        for(int i = 0; i < _links.length(); i++) {
            this->disconnect(_linkList[_links[index]], SIGNAL(onDropboxMessage(QByteArray)), _dropboxController, SLOT(handleDropboxMessage(QByteArray)));
            emit _linkList[_links[i]]->enabled(false);
        }
        emit _linkList[_links[index]]->enabled(true);
        this->connect(_linkList[_links[index]], SIGNAL(onDropboxMessage(QByteArray)), _dropboxController, SLOT(handleDropboxMessage(QByteArray)));
        _dropboxController->reset();
        _dropboxController->refreshAccessToken(_linkList[_links[index]]->xbStation()->getDropboxAccessToken());
        _cameraControl = _linkList[_links[index]]->cameraControl();
        emit cameraControlChanged();

    }
}

void XBStationManager::changePassword()
{
    _linkList.value(_links[_currentLinkIndex])->changePassword();
}

void XBStationManager::auth(QString email, QString password)
{
    this->connect(_tmpXBStation, SIGNAL(onAuthSuccess(int, int)), this, SLOT(handleAuthSuccess(int, int)));
    this->connect(_tmpXBStation, SIGNAL(onAuthFailed(QString)), this, SLOT(handleAuthFail(QString)));
    _email = email;
    _password = password;
    emit _tmpXBStation->auth(email, password);
}

void XBStationManager::auth(QString email)
{
    this->connect(_tmpXBStation, SIGNAL(onAuthSuccess(int, int)), this, SLOT(handleAuthSuccess(int, int)));
    this->connect(_tmpXBStation, SIGNAL(onAuthFailed(QString)), this, SLOT(handleAuthFail(QString)));
    emit _tmpXBStation->auth(email);
}

void XBStationManager::logOut(QString email)
{
    emit _tmpXBStation->logOut(email);
}

void XBStationManager::enableKeyListener(bool isEnabled, QObject *obj)
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->enableKeyListener(isEnabled, obj);
}

void XBStationManager::setVideoReceiver(CameraProvider *cameraProvider)
{
    emit cameraControlChanged();
}

void XBStationManager::setToolbox(QGCToolbox *toolbox)
{
    QGCTool::setToolbox(toolbox);
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

}

void XBStationManager::appendLink(QString name, XBLink *link)
{
    if(!_links.contains(name)) {
        _links.append(name);
        _linkList.insert(name, link);
        this->connect(link, SIGNAL(onDisconnected(QString)), this, SLOT(handleLinkDisconnected(QString)));
        emit linksChanged();
        if(_links.size() == 1) {
            _dropboxController->reset();
            this->connect(link->xbStation()->getImageProvider(), SIGNAL(onImage(QPixmap)), &_videoProvider, SLOT(handleReceiveImage(QPixmap)));
            this->connect(link->xbStation(), SIGNAL(onDropboxMessage(QByteArray)), _dropboxController, SLOT(handleDropboxMessage(QByteArray)));
            _dropboxController->reset();
            _dropboxController->refreshAccessToken(link->xbStation()->getDropboxAccessToken());
            _cameraControl = link->cameraControl();
            emit link->enabled(true);
            emit cameraControlChanged();
        }
    }
}

void XBStationManager::handleLinkDisconnected(QString linkName)
{
    if(_links.contains(linkName)) {
        _links.removeAll(linkName);
    }
    if(_linkList.contains(linkName)) {
        _linkList.remove(linkName);
    }
    emit linksChanged();
}

void XBStationManager::handleAuthSuccess(int permission, int limitationXBServer)
{
    this->disconnect(_tmpXBStation, SIGNAL(onAuthSuccess(int, int)), this, SLOT(handleAuthSuccess(int, int)));
    this->disconnect(_tmpXBStation, SIGNAL(onAuthFailed(QString)), this, SLOT(handleAuthFail(QString)));
    QDateTime dataTime;
    dataTime.setTime_t(permission);

    emit this->authSuccess(dataTime.toString(Qt::SystemLocaleShortDate), (limitationXBServer - QDateTime::currentSecsSinceEpoch() > 0));
}

void XBStationManager::handleAuthFail(QString error)
{
    this->disconnect(_tmpXBStation, SIGNAL(onAuthSuccess(int, int)), this, SLOT(handleAuthSuccess(int, int)));
    this->disconnect(_tmpXBStation, SIGNAL(onAuthFailed(QString)), this, SLOT(handleAuthFail(QString)));
    emit this->authFail(error);
}

void XBStationManager::handlePendingAccessToken(QString accessToken)
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->setDropboxToken(accessToken);
}

void XBStationManager::handleUploadFileToDropbox(QString fileName)
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->uploadDropboxFile(fileName);
}

void XBStationManager::handleGetDopboxLocalFiles()
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->getDropboxLocalFiles();
}

void XBStationManager::handleRemoveDropboxLocalFile(QString fileName)
{
    emit _linkList.value(_links[_currentLinkIndex])->xbStation()->removeDropboxLocalFile(fileName);
}
