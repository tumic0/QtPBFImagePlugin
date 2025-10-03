#include <QImage>
#include <QIODevice>
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include "data.h"
#include "tile.h"
#include "style.h"
#include "pbfhandler.h"

#define TILE_SIZE 512
#define MAGIC     0x1A

bool PBFHandler::canRead() const
{
	if (canRead(device())) {
		setFormat("mvt");
		return true;
	} else
		return false;
}

bool PBFHandler::canRead(QIODevice *device)
{
	quint8 magic;
	qint64 size = device->peek((char*)&magic, sizeof(magic));
	if (size != sizeof(magic))
		return false;

	return (magic == MAGIC);
}

bool PBFHandler::read(QImage *image)
{
	QByteArray ba(device()->readAll());
	Data data;
	if (!data.load(ba)) {
		qCritical() << "Invalid PBF data";
		return false;
	}

	QList<QByteArray> list(format().split(';'));
	unsigned zoom = list.size() ? list.first().toUInt() : 0;
	unsigned overzoom = (list.size() > 1) ? list.at(1).toUInt() : 0;
	unsigned style = (list.size() > 2) ? list.at(2).toUInt() : 0;

	QSize scaledSize(_scaledSize.isValid()
	  ? _scaledSize : QSize(TILE_SIZE, TILE_SIZE));
	QSize size(qMin(scaledSize.width()<<overzoom, 4096),
	  qMin(scaledSize.height()<<overzoom, 4096));
	QPointF scale((qreal)scaledSize.width() / TILE_SIZE,
	  (qreal)scaledSize.height() / TILE_SIZE);

	*image = QImage(size, QImage::Format_ARGB32_Premultiplied);
	Tile tile(image, zoom, scale);

	if (style < _styles.size())
		_styles.at(style)->render(data, tile);

	return true;
}

bool PBFHandler::supportsOption(ImageOption option) const
{
	return (option == Size || option == ScaledSize || option == Description);
}


void PBFHandler::setOption(QImageIOHandler::ImageOption option,
  const QVariant &value)
{
	if (option == ScaledSize)
		_scaledSize = value.toSize();
}

QVariant PBFHandler::option(ImageOption option) const
{
	switch (option) {
		case Size:
			return QSize(TILE_SIZE, TILE_SIZE);
		case Description:
			return description();
		default:
			return QVariant();
	};
}

QString PBFHandler::description() const
{
	QJsonArray styles;

	for (int i = 0; i < _styles.size(); i++) {
		QJsonObject style;
		style.insert("name", _styles.at(i)->name());
		style.insert("layers", QJsonArray::fromStringList(
			_styles.at(i)->layers()));
		styles.append(style);
	}

	return QJsonDocument(styles).toJson();
}
