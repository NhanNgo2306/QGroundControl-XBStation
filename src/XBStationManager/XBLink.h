#ifndef XBLINK_H
#define XBLINK_H
#include "LinkConfiguration.h"
#include <QObject>
#include "LinkInterface.h"
#include "xbstation.h"
#include "XBCameraControl.h"
#include "LinkManager.h"
#include <QMutex>

class XBClient {
public:
    XBClient(const QString username_,const QString password_)
        : username(username_)
        , password(password_)
    {}
    XBClient(const XBClient* other)
        : username(other->username)
        , password(other->password)
    {}
    QString username;
    QString password;
};


class XBConfiguration : public LinkConfiguration
{
    Q_OBJECT
public:

    Q_PROPERTY(QString  username    READ username   WRITE setUsername   NOTIFY  usernameChanged)
    Q_PROPERTY(QString  token       READ token      WRITE setToken      NOTIFY  tokenChanged)
    Q_PROPERTY(QString  state       READ state      WRITE setState      NOTIFY  stateChanged)
    Q_PROPERTY(QString  permission  READ permission WRITE setPermission NOTIFY  permissionChanged)
    Q_PROPERTY(QString  connectionType       READ connectionType      WRITE setConnectionType NOTIFY connectionTypeChanged)
    Q_PROPERTY(QString  firmIP      READ firmIP     WRITE setFirmIP     NOTIFY  firmIPChanged)

    /*!
     * @brief Regular constructor
     *
     * @param[in] name Configuration (user friendly) name
     */
    XBConfiguration(const QString& name);

    /*!
     * @brief Copy contructor
     *
     * When manipulating data, you create a copy of the configuration, edit it
     * and then transfer its content to the original (using copyFrom() below). Use this
     * contructor to create an editable copy.
     *
     * @param[in] source Original configuration
     */
    XBConfiguration(XBConfiguration* source);

    ~XBConfiguration();


    /*!
     * @brief QML Interface
     */
//    QStringList hostList    () { return _hostList; }
    Q_INVOKABLE void addUsername (const QString username);
    Q_INVOKABLE void addToken (const QString token);
    Q_INVOKABLE void addPermission(const QString permission);

    /// From LinkConfiguration
    LinkType    type                 () { return LinkConfiguration::TypeXB; }
    void        copyFrom             (LinkConfiguration* source);
    void        loadSettings         (QSettings& settings, const QString& root);
    void        saveSettings         (QSettings& settings, const QString& root);
    bool        isAutoConnectAllowed () { return false; }
    bool        isHighLatencyAllowed () { return false; }
    QString     settingsURL          () { return "XBAuthentication.qml"; }
    QString     settingsTitle        () { return tr("Authentication"); }
    QString     username             () { return _username; }
    QString     token                () { return _token; }
    QString     state                () { return _state; }
    QString     permission           () { return _permission; }
    QString     connectionType                () { return _connectionType; }
    QString     firmIP               () { return _firmIP; }
    void        setUsername          (QString username);
    void        setToken             (QString token);
    void        setState             (QString state);
    void        setPermission        (QString permission);
    void        setConnectionType    (QString connectionType);
    void        setFirmIP            (QString firmIP);

private:
    void _copyFrom          (LinkConfiguration *source);
    XBClient* targetClient;
    QString _username;
    QString _token;
    QString _state;
    QString _permission;
    QString _connectionType;
    QString _firmIP;
signals:
    void connectionTypeChanged();
    void permissionChanged();
    void usernameChanged();
    void tokenChanged();
    void stateChanged();
    void firmIPChanged();

public:
    void updateSettings();
public slots:
};



class XBLink : public LinkInterface
{
    Q_OBJECT

    friend class UDPConfiguration;
    friend class LinkManager;

public:
    void    requestReset            () override { }
    bool    isConnected             () const override;
    QString getName                 () const override;

    qint64  getConnectionSpeed      () const override;
    qint64  getCurrentInDataRate    () const;
    qint64  getCurrentOutDataRate   () const;
    XBCameraControl* cameraControl  () { return &_xbCameraControl; }
    XBStation* xbStation            () { return _xbStation; }

    void    changePassword          ();
    void    run                     () override;
    bool    disconnect              (void);
public slots:
    void    readBytes               (QByteArray buffer);

signals:
    void    enabled                 (bool enabled);
    void    onDisconnected          (QString linkName);

private slots:
    void onDisconnectSuccess        ();
    void onDisconnectFail           ();
    void handleConnectSuccess       ();
    void handleConnectFail          (QString error);
    void handleStreamingData        (QByteArray buffer);
    void handlePermission           (int);
    void _writeBytes                (const QByteArray) override;
    void handleEnabled              (bool enabled);
private:
    XBLink(SharedLinkConfigurationPointer& config);
    ~XBLink();
    bool                _running;
    bool                _connectState;
    bool                _connect                (void) override;
    void                _disconnect             (void) override;
    void                _restartConnection      ();
    XBStation*          _xbStation;
    XBConfiguration*    _xbConfig;
    XBCameraControl     _xbCameraControl;
    int                 _permission = 0;
    QMutex              _mutex;
};

#endif // XBLINK_H
