#ifndef XBDROPBOXCONTROLLER_H
#define XBDROPBOXCONTROLLER_H

#include <QObject>
#include <QFileInfoList>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QAbstractListModel>
#include <QQmlEngine>
#include <QDebug>
#include <QMutex>
//-----------------------------------------------------------------------------
class XBFileInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(int          id          READ id                             CONSTANT)
    Q_PROPERTY(QString      name        READ name                           NOTIFY nameChanged)
    Q_PROPERTY(int          size        READ size                           NOTIFY sizeChanged)
    Q_PROPERTY(QString      path        READ path                           NOTIFY pathChanged)
    Q_PROPERTY(QString      status      READ status                         NOTIFY statusChanged)

public:
    XBFileInfo();

    int         id          () const { return _id; }
    int         size        () const { return _size; }
    QString     status      () const { return _status; }
    QString     path        () const { return _path; }
    QString     name        () const { return _name; }

    void        setId       (int id_)           { _id = id_; }
    void        setSize     (int size_)         { _size = size_;    emit sizeChanged(); }
    void        setStatus   (QString stat_)     { _status = stat_;  emit statusChanged(); }
    void        setPath     (QString path)      { _path = path;     emit pathChanged(); }
    void        setName     (QString name)      { _name = name;     emit nameChanged(); }

signals:
    void        idChanged       ();
    void        sizeChanged     ();
    void        statusChanged   ();
    void        pathChanged     ();
    void        nameChanged     ();

private:
    QString     _name;
    QString     _path;
    int         _id;
    int         _size;
    QString     _status;
};


class XBFileInfoModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum QGCLogModelRoles {
        ObjectRole = Qt::UserRole + 1
    };

    XBFileInfoModel(QObject *parent = 0);

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_INVOKABLE XBFileInfo* get(int index);

    int         count           (void) const;
    void        append          (XBFileInfo* entry);
    void        clear           (void);
    XBFileInfo* findChildByName (QString name);
    XBFileInfo*operator[]       (int i);

    int         rowCount        (const QModelIndex & parent = QModelIndex()) const;
    QVariant    data            (const QModelIndex & index, int role = Qt::DisplayRole) const;

signals:
    void        countChanged    ();

protected:
    QHash<int, QByteArray> roleNames() const;
private:
    QList<XBFileInfo*> _fileInfoEntries;
    QMutex _mutex;
};

class XBDropboxController : public QObject
{
    Q_OBJECT
public:
    explicit XBDropboxController(QObject *parent = nullptr);
    Q_PROPERTY(int              accessTokenStatus           READ accessTokenStatus              NOTIFY tokenAccessStatusChanged)
    Q_PROPERTY(XBFileInfoModel* model                       READ model                          NOTIFY modelChanged)
    Q_PROPERTY(QString          accessToken                 READ accessToken                    NOTIFY accessTokenChanged)

    XBFileInfoModel*    model               () { return &_fileInfoEntriesModel; }
    void refreshAccessToken                 (QString token);
    int accessTokenStatus                   () { return _accessTokenStatus; }
    void reset                              ();
    QString accessToken                     () { return _accessToken; }
    Q_INVOKABLE void refresh                ();
    Q_INVOKABLE void upload                 (int index);
    Q_INVOKABLE void setAccessToken         (QString accessToken);
    Q_INVOKABLE void removeFile             (int index);
signals:
    void accessTokenChanged         ();
    void tokenAccessStatusChanged   ();
    void uploadFileToDropbox        (QString fileName);
    void pendingAccessToken         (QString accessToken);
    void modelChanged               ();
    void getLocalFiles              ();
    void remove                     (QString fileName);
public slots:
    void handleDropboxMessage(QByteArray data);

private:
    int _accessTokenStatus;
    QString _accessToken;
    XBFileInfoModel         _fileInfoEntriesModel;
    void handleReceivedUploadMessage(QJsonObject data);
    void handleReceivedLocalFilesMessage(QJsonObject data);
    void handleReceivedRemoveLocalFileMessage(QJsonObject messageObjet);
};

#endif // XBDROPBOXCONTROLLER_H
