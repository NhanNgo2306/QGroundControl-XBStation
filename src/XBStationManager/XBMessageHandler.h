#ifndef XBMESSAGEHANDLER_H
#define XBMESSAGEHANDLER_H

#include <QObject>
#include "xbstation.h"
#include <QMutex>
#include "QGCApplication.h"
#include "UASMessageHandler.h"

class XBMessage
{
    friend class XBMessageHandler;
public:
    /**
     * @brief Get message source component ID
     */
    int getComponentID()        { return _compId; }
    /**
     * @brief Get message severity (from MAV_SEVERITY_XXX enum)
     */
    QString getText()           { return _text; }
    /**
     * @brief Get (html) formatted text (in the form: "[11:44:21.137 - COMP:50] Info: [pm] sending list")
     */
    QString getFormatedText()   { return _formatedText; }
    /**
     * @return true: This message is a of a severity which is considered an error
     */
private:
    XBMessage(QString type, QString text);
    void _setFormatedText(const QString formatedText) { _formatedText = formatedText; }
    int _compId;
    int _severity;
    QString _text;
    QString _formatedText;
};


class XBMessageHandler : public QObject
{
    Q_OBJECT

public:
    explicit XBMessageHandler(QObject *parent = nullptr);
    ~XBMessageHandler();

    /**
     * @brief Locks access to the message list
     */
    void lockAccess()   {_mutex.lock(); }
    /**
     * @brief Unlocks access to the message list
     */
    void unlockAccess() {_mutex.unlock(); }
    /**
     * @brief Access to the message list
     */
    const QVector<XBMessage*>& messages() { return _messages; }
    /**
     * @brief Clear messages
     */
    void clearMessages();
    /**
     * @brief Get error message count (Resets count once read)
     */
    int getErrorCount();
    /**
     * @brief Get error message count (never reset)
     */
    int getErrorCountTotal();
    /**
     * @brief Get warning message count (Resets count once read)
     */
    int getWarningCount();
    /**
     * @brief Get normal message count (Resets count once read)
     */
    int getNormalCount();
    /**
     * @brief Get latest error message
     */
    QString getLatestError()   { return _latestError; }

    /// Begin to show message which are errors in the toolbar
    void showErrorsInToolbar(void) { _showErrorsInToolbar = true; }

    // Override from QGCTool
//    virtual void setToolbox(QGCToolbox *toolbox);

public slots:
    /**
     * @brief Handle text message from current active UAS
     * @param uasid UAS Id
     * @param componentid Component Id
     * @param severity Message severity
     * @param text Message Text
     */
    void handleTextMessage(QString type, QString text);

signals:
    /**
     * @brief Sent out when new message arrives
     * @param message A pointer to the message. NULL if resetting (new UAS assigned)
     */
    void textMessageReceived(XBMessage* message);
    /**
     * @brief Sent out when the message count changes
     * @param count The new message count
     */
    void textMessageCountChanged(int count);

private slots:
    void _activeXBStationChanged(XBStation* vehicle);

private:
    XBStation*              _activeXBStation;
    int                     _activeComponent;
    bool                    _multiComp;
    QVector<XBMessage*>     _messages;
    QMutex                  _mutex;
    int                     _errorCount;
    int                     _errorCountTotal;
    int                     _warningCount;
    int                     _normalCount;
    QString                 _latestError;
    bool                    _showErrorsInToolbar;
    MultiVehicleManager*    _multiVehicleManager;
};


#endif // XBMESSAGEHANDLER_H
