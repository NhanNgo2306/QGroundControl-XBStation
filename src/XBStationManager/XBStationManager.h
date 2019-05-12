#ifndef XBSTATIONMANAGER_H
#define XBSTATIONMANAGER_H

#include <QObject>
#include "QGCToolbox.h"
#include "QGCApplication.h"
#include "XBCameraControl.h"
#include "camera/camera_provider.h"
#include "XBLink.h"
#include "XBVideoProvider.h"
#include <QMutex>
#include "XBDropboxController.h"

Q_DECLARE_LOGGING_CATEGORY(XBStationManagerLog)

class XBStationManager : public QGCTool
{
    Q_OBJECT
public:
    explicit XBStationManager(QGCApplication* app, QGCToolbox* toolbox);
    ~XBStationManager();
    Q_PROPERTY(XBCameraControl*     cameraControl       READ cameraControl          NOTIFY  cameraControlChanged)
    Q_PROPERTY(bool                 isConnected         READ isConnected            NOTIFY  infoChanged)
    Q_PROPERTY(QStringList          links               READ links                  NOTIFY  linksChanged)
    Q_PROPERTY(XBVideoProvider*     xbVideoProvider     READ xbVideoProvider        CONSTANT)
    Q_PROPERTY(XBDropboxController* dropboxController   READ dropboxController      NOTIFY  dropboxControllerChanged)

    Q_INVOKABLE void openCamera();
    Q_INVOKABLE void openRecording();
    Q_INVOKABLE void closeRecording();
    Q_INVOKABLE void changeControlingMethod(int index);
    Q_INVOKABLE void changeCameraType(int type);
    Q_INVOKABLE void changeCameraResolution(int resolution);
    Q_INVOKABLE void changeRecordingResolution(int recordResolution);
    Q_INVOKABLE void closeCamera();
    Q_INVOKABLE void prepare();
    Q_INVOKABLE void changePixPort(int indexPort);
    Q_INVOKABLE void changePixBaudrate(int indexBaudrate);
    Q_INVOKABLE void changeStation(int index);
    Q_INVOKABLE void changePassword();
    Q_INVOKABLE void auth(QString email, QString password);
    Q_INVOKABLE void auth(QString email);
    Q_INVOKABLE void logOut(QString email);
    Q_INVOKABLE void enableKeyListener(bool isEnabled, QObject* obj);

    bool             isConnected        () { return _isConnected; }
    QStringList      links              () { return _links; }
    void             setVideoReceiver   (CameraProvider* cameraProvider);
    virtual          void setToolbox    (QGCToolbox *toolbox);
    void             appendLink         (QString name, XBLink* link);
    XBCameraControl* cameraControl      () { return _cameraControl; }
    XBVideoProvider* xbVideoProvider    () { return &_videoProvider; }
    XBDropboxController* dropboxController() { return _dropboxController; }

signals:
    void dropboxControllerChanged();
    void infoChanged();
    void cameraControlChanged();
    void linksChanged();
    void authSuccess(QString permission, bool isXBServer);
    void authFail(QString err);
private slots:
    void handleLinkDisconnected(QString linkName);
    void handleAuthSuccess(int permission, int limitationServer);
    void handleAuthFail(QString err);
    void handlePendingAccessToken(QString accessToken);
    void handleUploadFileToDropbox(QString fileName);
    void handleGetDopboxLocalFiles();
    void handleRemoveDropboxLocalFile(QString fileName);
private:
    bool _isConnected = false;
    int _linkCount = 0;
    QMap<QString, XBLink*> _linkList;
    QStringList _links;
    XBCameraControl* _cameraControl = nullptr;
    XBVideoProvider _videoProvider;
    int _currentLinkIndex = 0;
    XBStation* _tmpXBStation = nullptr;
    QMutex _mutex;
    QString _email;
    QString _password;
    XBDropboxController* _dropboxController = nullptr;

};

#endif // XBSTATIONMANAGER_H
