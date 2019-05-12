#include "XBVideoProvider.h"
#include <QDebug>

XBVideoProvider::XBVideoProvider(QObject *parent) : QObject(parent), QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    this->blockSignals(false);
}

QPixmap XBVideoProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return _image;
}

void XBVideoProvider::handleReceiveImage(QPixmap image)
{
    _image = image;
    emit this->imageChanged();
}
