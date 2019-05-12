#ifndef XBVIDEOPROVIDER_H
#define XBVIDEOPROVIDER_H

#include <QObject>
#include <QQuickImageProvider>

class XBVideoProvider : public QObject, public QQuickImageProvider
{
    Q_OBJECT
public:
    explicit XBVideoProvider(QObject *parent = nullptr);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
signals:
    void imageChanged();
public slots:
    void handleReceiveImage(QPixmap image);
private:
    QPixmap _image;
};

#endif // XBVIDEOPROVIDER_H
