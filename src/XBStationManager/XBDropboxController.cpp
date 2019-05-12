#include "XBDropboxController.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

XBDropboxController::XBDropboxController(QObject *parent) :
    QObject(parent),
    _accessTokenStatus(false)
{

}

void XBDropboxController::refreshAccessToken(QString token)
{
    _accessToken = token;
    _accessTokenStatus = 0;
    emit this->accessTokenChanged();
}

void XBDropboxController::reset()
{
    _accessTokenStatus = 0;
    emit this->tokenAccessStatusChanged();
    _fileInfoEntriesModel.clear();
}

void XBDropboxController::refresh()
{
    emit this->getLocalFiles();
}

void XBDropboxController::upload(int index)
{
    emit this->uploadFileToDropbox(_fileInfoEntriesModel.get(index)->name());
    _fileInfoEntriesModel.get(index)->setStatus("Uploading");
}

void XBDropboxController::setAccessToken(QString accessToken)
{
    emit this->pendingAccessToken(accessToken);
    _accessTokenStatus = 1;
    _accessToken = accessToken;
    emit this->tokenAccessStatusChanged();
}

void XBDropboxController::removeFile(int index)
{
    emit this->remove(_fileInfoEntriesModel.get(index)->name());
    _fileInfoEntriesModel.get(index)->setStatus("Removing");
}

void XBDropboxController::handleDropboxMessage(QByteArray data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    qDebug() << doc;
    QJsonObject obj = doc.object();
    if(obj.value("type").toString() == "auth") {
        if(obj.value("success").toBool()) {
            QJsonArray array = obj.value("data").toArray();
            this->_fileInfoEntriesModel.clear();
            for(int i = 0; i < array.size(); i++) {
                QJsonObject fileInforObj = QJsonDocument::fromJson(array[i].toString().toLocal8Bit()).object();
                XBFileInfo* fileInfor = new XBFileInfo();
                fileInfor->setId(i+1);
                fileInfor->setName(fileInforObj.value("name").toString());
                fileInfor->setPath(fileInforObj.value("path").toString());
                fileInfor->setSize(fileInforObj.value("size").toInt());
                _fileInfoEntriesModel.append(fileInfor);
            }
            _accessTokenStatus = 2;
            emit this->tokenAccessStatusChanged();
        }
    }
    else if(obj.value("type").toString() == "upload") {
        this->handleReceivedUploadMessage(obj);
    }
    else if(obj.value("type").toString() == "local-list") {
        this->handleReceivedLocalFilesMessage(obj);
    }
    else if(obj.value("type").toString() == "removed") {
        this->handleReceivedRemoveLocalFileMessage(obj);
    }

}

void XBDropboxController::handleReceivedUploadMessage(QJsonObject messageObject)
{
    XBFileInfo* fileInfo = _fileInfoEntriesModel.findChildByName(messageObject.value("name").toString());
    if(fileInfo) {
        if(messageObject.value("success").toBool()) {
            QJsonObject dataObject = messageObject.value("data").toObject();
            if(dataObject.value("is_part").toBool()) {
                fileInfo->setStatus(QString::number(dataObject.value("size").toDouble()) + "MB uploaded. Uploading...");
            }
            else {
                QJsonObject response = dataObject.value("response").toObject();
                QString errorSumary = response.value("error_summary").toString();
                if(errorSumary != "") {
                    fileInfo->setStatus("Upload Failed: "+errorSumary);
                }
                else {
                    if(response.value("name").toString() == "") {
                        fileInfo->setStatus("Upload Failed ");
                    }
                    else {
                        fileInfo->setStatus("Uploaded");
                    }
                }
            }
        }
        else {
            fileInfo->setStatus("Upload Failed: "+messageObject.value("data").toString());
        }
    }
}

void XBDropboxController::handleReceivedLocalFilesMessage(QJsonObject messageObject)
{
    if(messageObject.value("success").toBool()) {
        QJsonArray array = messageObject.value("data").toArray();
        this->_fileInfoEntriesModel.clear();
        for(int i = 0; i < array.size(); i++) {
            QJsonObject fileInforObj = QJsonDocument::fromJson(array[i].toString().toLocal8Bit()).object();
            XBFileInfo* fileInfor = new XBFileInfo();
            fileInfor->setId(i+1);
            fileInfor->setName(fileInforObj.value("name").toString());
            fileInfor->setPath(fileInforObj.value("path").toString());
            fileInfor->setSize(fileInforObj.value("size").toInt());
            _fileInfoEntriesModel.append(fileInfor);
        }
        _accessTokenStatus = 2;
        emit this->tokenAccessStatusChanged();
    }
}

void XBDropboxController::handleReceivedRemoveLocalFileMessage(QJsonObject messageObjet)
{
    XBFileInfo* fileInfo = _fileInfoEntriesModel.findChildByName(messageObjet.value("name").toString());
    if(fileInfo) {
        if(messageObjet.value("success").toBool()) {
            fileInfo->setStatus("removed");
        }
        else {
            fileInfo->setStatus("Removed Failed: "+messageObjet.value("data").toString());
        }
    }
}

XBFileInfo::XBFileInfo()
{

}

XBFileInfoModel::XBFileInfoModel(QObject *parent) : QAbstractListModel(parent)
{

}

XBFileInfo* XBFileInfoModel::get(int index)
{
    if (index < 0 || index >= _fileInfoEntries.count()) {
        return NULL;
    }
    return _fileInfoEntries[index];
}

int XBFileInfoModel::count() const
{
    return _fileInfoEntries.count();
}

void XBFileInfoModel::append(XBFileInfo *object)
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    QQmlEngine::setObjectOwnership(object, QQmlEngine::CppOwnership);
    _fileInfoEntries.append(object);
    endInsertRows();
    emit countChanged();
}

void XBFileInfoModel::clear()
{
    if(!_fileInfoEntries.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, _fileInfoEntries.count());
        while (_fileInfoEntries.count()) {
            XBFileInfo* entry = _fileInfoEntries.last();
            if(entry) entry->deleteLater();
            _fileInfoEntries.removeLast();
        }
        endRemoveRows();
        emit countChanged();
    }
}

XBFileInfo *XBFileInfoModel::findChildByName(QString name)
{
    foreach(XBFileInfo* fileInfo, _fileInfoEntries) {
        if(fileInfo->name() == name)
            return fileInfo;
    }
    return nullptr;
}

XBFileInfo* XBFileInfoModel::operator[](int index)
{
    return get(index);
}

int XBFileInfoModel::rowCount(const QModelIndex &parent) const
{
    return _fileInfoEntries.count();
}

QVariant XBFileInfoModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= _fileInfoEntries.count())
        return QVariant();
    if (role == ObjectRole)
        return QVariant::fromValue(_fileInfoEntries[index.row()]);
    return QVariant();
}

QHash<int, QByteArray> XBFileInfoModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[ObjectRole] = "xbEntry";
    return roles;
}
