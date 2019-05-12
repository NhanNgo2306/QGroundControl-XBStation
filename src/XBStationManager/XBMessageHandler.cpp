#include "XBMessageHandler.h"

XBMessageHandler::XBMessageHandler(QObject *parent) : QObject(parent)
{

}

XBMessageHandler::~XBMessageHandler()
{
    clearMessages();
}

void XBMessageHandler::clearMessages()
{
    _mutex.lock();
    while(_messages.count()) {
        delete _messages.last();
        _messages.pop_back();
    }
    _errorCount   = 0;
    _warningCount = 0;
    _normalCount  = 0;
    _mutex.unlock();
    emit textMessageCountChanged(0);
}

int XBMessageHandler::getErrorCount()
{
    return _errorCount;
}

int XBMessageHandler::getErrorCountTotal()
{
    return _errorCountTotal;
}

int XBMessageHandler::getWarningCount()
{
    return _warningCount;
}

int XBMessageHandler::getNormalCount()
{
    return _normalCount;
}

void XBMessageHandler::_activeXBStationChanged(XBStation *xbStation)
{
    if (_activeXBStation != nullptr) {
        disconnect(_activeXBStation, &XBStation::onTextMessageReceived, this, &XBMessageHandler::handleTextMessage);
        _activeXBStation = NULL;
        clearMessages();
    }

    //     And now if there's an autopilot to follow, set up the UI.
    if (xbStation) {
        // Connect to the new UAS.
        clearMessages();
        _activeXBStation = xbStation;
        connect(_activeXBStation, &XBStation::onTextMessageReceived, this, &XBMessageHandler::handleTextMessage);
    }
}

void XBMessageHandler::handleTextMessage(QString type, QString text)
{
    _mutex.lock();
    QString style = "";
    QString severityText("");
    if(type == XBStation::ID_MSG_DANGER) {
        style = QString("<#E>");
        _errorCount++;
        _errorCountTotal++;
        severityText = QString(tr(" ALERT:"));
    }
    else if (type == XBStation::ID_MSG_WARNING) {
        style = QString("<#I>");
        _warningCount++;
        severityText = QString(tr(" Warning:"));
    }
    else if (type == XBStation::ID_MSG_SUCCESS) {
        style = QString("<#S>");
        _normalCount++;
        severityText = QString(tr(" Success:"));
    }
    else if (type == XBStation::ID_MSG_INFO) {
        style = QString("<#N>");
        _normalCount++;
        severityText = QString(tr(" Info:"));
    }

    // Finally preppend the properly-styled text with a timestamp.
    QString dateString = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    XBMessage* message = new XBMessage(type, text);
    QString compString("");

    message->_setFormatedText(QString("<font style=\"%1\">[%2%3]%4 %5</font><br/>").arg(style).arg(dateString).arg(compString).arg(severityText).arg(text));

    _mutex.unlock();

    _messages.append(message);
    int count = _messages.count();

    emit textMessageReceived(message);
    emit textMessageCountChanged(count);

}

XBMessage::XBMessage(QString type, QString text)
{

}
