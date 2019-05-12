#ifndef XBCAMERACONTROL_H
#define XBCAMERACONTROL_H

#include <QObject>
#include "XBVideoProvider.h"
#include "XBMessageHandler.h"
#include <QDebug>

class XBCameraControl : public QObject
{
    Q_OBJECT
public:
    explicit XBCameraControl(QObject *parent = nullptr);

    Q_PROPERTY(int              type                READ type               NOTIFY infoChanged)
    Q_PROPERTY(qreal            aspectRatio         READ aspectRatio        NOTIFY infoChanged)
    Q_PROPERTY(bool             videoRunning        READ videoRunning       NOTIFY infoChanged)
    Q_PROPERTY(int              videoState          READ videoState         NOTIFY infoChanged)
    Q_PROPERTY(int              recordingState      READ recordingState     NOTIFY infoChanged)
    Q_PROPERTY(bool             fullScreen          READ fullScreen         NOTIFY infoChanged)
    Q_PROPERTY(XBVideoProvider* xbVideoProvider     READ xbVideoProvider    NOTIFY xbVideoProviderChanged)
    Q_PROPERTY(QStringList      resolutions         READ resolutions        NOTIFY resolutionsChanged)
    Q_PROPERTY(int              currentIndex        READ currentIndex       NOTIFY currentIndexChanged)
    Q_PROPERTY(int              currentRecordIndex  READ currentRecordIndex NOTIFY currentRecordIndexChanged)
    Q_PROPERTY(QString          byteRecv            READ byteRecv           NOTIFY byteRecvChanged)
    Q_PROPERTY(QString          byteSent            READ byteSent           NOTIFY byteSentChanged)
    Q_PROPERTY(QStringList      cameraTypes         READ cameraTypes        CONSTANT)
    Q_PROPERTY(int              indexCameraType     READ indexCameraType    NOTIFY indexCameraTypeChanged)
    Q_PROPERTY(QStringList      controllingMethods  READ controllingMethods NOTIFY controllingMethodsChanged)
    Q_PROPERTY(int              indexControllingMethod  READ    indexControllingMethod  NOTIFY  indexControllingMethodChanged)
    Q_PROPERTY(QStringList      pixPorts            READ pixPorts           NOTIFY pixPortsChanged)
    Q_PROPERTY(int              indexPort           READ indexPort          NOTIFY indexPortChanged)
    Q_PROPERTY(bool             isPortConfirm       READ isPortConfirm      NOTIFY  isPortConfirmChanged)
    Q_PROPERTY(bool             isBaudrateConfirm   READ isBaudrateConfirm  NOTIFY  isBaudrateConfirmChanged)
    Q_PROPERTY(QStringList      baudrates           READ baudrates          CONSTANT)
    Q_PROPERTY(int              indexBaudrate       READ indexBaudrate      NOTIFY indexBaudrateChanged)
    Q_PROPERTY(bool             isSignalConnected   READ isSignalConnected  NOTIFY signalStatusChanged)
    Q_PROPERTY(QString          formatedMessages    READ formatedMessages   NOTIFY formatedMessagesChanged)
    Q_PROPERTY(int              messageCount        READ messageCount       NOTIFY messageCountChanged)
    Q_PROPERTY(int              newMessageCount     READ newMessageCount    NOTIFY newMessageCountChanged)
    Q_PROPERTY(bool             messageTypeNone     READ messageTypeNone    NOTIFY messageTypeChanged)
    Q_PROPERTY(bool             messageTypeNormal   READ messageTypeNormal  NOTIFY messageTypeChanged)
    Q_PROPERTY(bool             messageTypeWarning  READ messageTypeWarning NOTIFY messageTypeChanged)
    Q_PROPERTY(bool             messageTypeError    READ messageTypeError   NOTIFY messageTypeChanged)
    Q_PROPERTY(QString          latestError         READ latestError        NOTIFY latestErrorChanged)
    Q_PROPERTY(QString          formatedMessage     READ formatedMessage    NOTIFY formatedMessageChanged)
    Q_PROPERTY(QStringList      recordResolutions   READ recordResolutions  NOTIFY recordResolutionsChanged)
    Q_PROPERTY(int              recordingFps        READ recordingFps       WRITE  setRecordingFps              NOTIFY recordingFpsChanged)

    Q_INVOKABLE void showPortConfirm(int);
    Q_INVOKABLE void showBaudrateConfirm(int);
    Q_INVOKABLE void resetMessages();
    Q_INVOKABLE void clearMessages();

    int                 recordingFps    () { return _recordingFps; }
    qreal               aspectRatio     () { return _aspectRatio; }
    int                 type            () { return _type; }
    bool                videoRunning    () { return _videoRunning; }
    int                 videoState      () { return _videoState; }
    bool                fullScreen      () { return _fullScreen; }
    XBVideoProvider*    xbVideoProvider () { return &_xbVideoProvider; }
    QStringList         resolutions     () { return _resolutions; }
    int                 currentIndex    () { return _currentIndex; }
    int                 currentRecordIndex() { return _currentRecordingIndex; }
    QString             byteRecv        () { return _byteRecv; }
    QString             byteSent        () { return _byteSent; }
    QStringList         cameraTypes     () { return _cameraTypes; }
    int                 indexCameraType () { return _indexCameraType; }
    QStringList         controllingMethods() { return _controllingMethods; }
    int                 indexControllingMethod() { return _indexControllingMethod; }
    QStringList         pixPorts        () { return _pixPorts; }
    int                 indexPort       () { return _indexPort; }
    bool                isPortConfirm   () { return _isPortConfirm; }
    bool                isBaudrateConfirm(){ return _isBaudrateConfirm; }
    QStringList         baudrates       () { return _baudrates; }
    int                 indexBaudrate   () { return _indexBaudrate; }
    int                 recordingState  () { return _recordingState; }
    bool                isSignalConnected(){ return _isSignalConnected; }
    int                 messageCount    () { return _messageCount; }
    QString             formatedMessages();
    int                 newMessageCount () { return _currentMessageCount; }
    bool                messageTypeNone () { return _currentMessageType == MessageNone; }
    bool                messageTypeNormal() { return _currentMessageType == MessageNormal; }
    bool                messageTypeWarning() { return _currentMessageType == MessageWarning; }
    bool                messageTypeError() { return _currentMessageType == MessageError; }
    QString             latestError     () { return _latestError; }
    QString             formatedMessage () { return _formatedMessage; }
    QStringList         recordResolutions(){ return _recordResolutions; }
    void                setCurrentRecordIndex(int index) { _currentRecordingIndex = index; }

    void                setRecordingFps (int fps) { _recordingFps = fps; }

    typedef enum {
        MessageNone,
        MessageNormal,
       MessageWarning,
        MessageError
    } MessageType_t;

signals:
    void recordingFpsChanged();
    void currentRecordIndexChanged();
    void recordResolutionsChanged();
    void newMessageCountChanged();
    void signalStatusChanged();
    void infoChanged();
    void videoReceiverChanged();
    void xbVideoProviderChanged();
    void resolutionsChanged();
    void currentIndexChanged();
    void byteSentChanged();
    void byteRecvChanged();
    void indexCameraTypeChanged();
    void controllingMethodsChanged();
    void indexControllingMethodChanged();
    void pixPortsChanged();
    void indexPortChanged();
    void isPortConfirmChanged();
    void indexBaudrateChanged();
    void isBaudrateConfirmChanged();
    void formatedMessagesChanged();
    void messageCountChanged();
    void messageTypeChanged();
    void latestErrorChanged();
    void formatedMessageChanged();
    void showXBMessage();
public slots:
    void _handletextMessageReceived(XBMessage* message);
    void _handleTextMessage (int newCount);
    void handleMessage(QString type, QString text);
    void handleSignalStatus(bool isConnected);
    void handlePixPort(QString portName);
    void handleCameraStatus(int status);
    void handleRecordingStatus(int status);
    void handleResolutions(QStringList res, int currentIndex);
    void handleByteReceive(int amount);
    void handleByteSent(int amount);
    void handleCameraType(int type);
    void handleStationControllingConnected(int);
    void handleStationControllingDisconnected(int);
    void handlePixPorts(QStringList availablePorts, QString currentPortName);
    void handlePixBaudrate(QString baudrate);
    void handleResolutionChanged(int res);
    bool changeCameraType(int);
    bool changeControllingMethod(int type);
    bool changeCameraResolution(int res);
    QString changePixPort(int indexPixPort);
    QString changePixBaudrate(int indexBaudrate);
private:
    qreal _aspectRatio;
    int _type;
    bool _videoRunning = false;
    int _currentIndex = 0;
    /**
     * @brief _videoState
     * 0 : not ready
     * 2 : close
     * 1: open
     * 3: cannot open
     */
    bool _isSignalConnected         = false;
    int _videoState                 = 0;
    int _recordingState             = -1;
    int _recordingFps               = 15;
    bool _fullScreen                = false;
    QStringList _resolutions        = {""};
    XBVideoProvider _xbVideoProvider;
    QString _byteSent               = "0.0";
    QString _byteRecv               = "0.0";
    QStringList _cameraTypes        = {"RGB", "Gray"};
    int _amountSent                 = 0;
    int _amountRecv                 = 0;
    int _indexCameraType            = -1;
    int _indexControllingMethod     = -7;
    QStringList _controllingMethods = {"DGRAM", "SOCKET"};
    bool _isTheFirstConnect         = true;
    QStringList _pixPorts           = {""};
    int _indexPort                  = -1;
    int _indexBaudrate              = -1;
    bool _isPortConfirm             = false;
    bool _isBaudrateConfirm         = false;
    int _currentRecordingIndex      = 0;
    QStringList _baudrates          = {"", "1200", "2400", "4800", "9600", "19200", "38400", "57600", "111100",
                                        "115200", "256000", "460800", "500000", "921600", "1500000"};
    QStringList _recordResolutions  = {""};
    XBMessageHandler _xbMessageHandler;
    int _messageCount               = 0;
    int _currentMessageCount        = 0;
    MessageType_t   _currentMessageType;
    int             _currentErrorCount;
    int             _currentWarningCount;
    int             _currentNormalCount;
    QString         _latestError;
    QString         _formatedMessage;

};

#endif // XBCAMERACONTROL_H
