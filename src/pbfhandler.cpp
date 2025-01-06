#include <QImage>
#include <QIODevice>
#include <QtEndian>
#include <QDebug>
#include "gzip.h"
#include "data.h"
#include "tile.h"
#include "style.h"
#include "pbfhandler.h"


#define TILE_SIZE 512

#define GZIP_MAGIC      0x1F8B
#define GZIP_MAGIC_MASK 0xFFFF
#define PBF_MAGIC       0x1A00
#define PBF_MAGIC_MASK  0xFF00

static bool isMagic(quint16 magic, quint16 mask, quint16 value)
{
	return ((qFromBigEndian(value) & mask) == magic);
}

static bool isGZIPPBF(quint16 magic)
{
	return isMagic(GZIP_MAGIC, GZIP_MAGIC_MASK, magic);
}

static bool isPlainPBF(quint16 magic)
{
	return isMagic(PBF_MAGIC, PBF_MAGIC_MASK, magic);
}


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
	quint16 magic;
	qint64 size = device->peek((char*)&magic, sizeof(magic));
	if (size != sizeof(magic))
		return false;

	if (isPlainPBF(magic))
		return true;
	else if (isGZIPPBF(magic)) {
		QByteArray ba(Gzip::uncompress(device, sizeof(magic)));
		if (ba.size() < (int)sizeof(magic))
			return false;
		return isPlainPBF(*((quint16*)ba.constData()));
	} else
		return false;
}

bool PBFHandler::read(QImage *image)
{
	quint16 magic;
	if (device()->peek((char*)&magic, sizeof(magic)) != sizeof(magic))
		return false;

	QByteArray ba;
	if (isGZIPPBF(magic)) {
		ba = Gzip::uncompress(device());
		if (ba.isNull()) {
			qCritical() << "Invalid gzip data";
			return false;
		}
	} else if (isPlainPBF(magic))
		ba = device()->readAll();
	Data data;
	if (!data.load(ba)) {
		qCritical() << "Invalid PBF data";
		return false;
	}

	QList<QByteArray> list(format().split(';'));
	int zoom = list.size() ? list.first().toInt() : 0;
	int overzoom = (list.size() > 1) ? list.at(1).toInt() : 0;

	QSize scaledSize(_scaledSize.isValid()
	  ? _scaledSize : QSize(TILE_SIZE, TILE_SIZE));
	QSize size(scaledSize.width()<<overzoom,
	  scaledSize.height()<<overzoom);
	QPointF scale((qreal)scaledSize.width() / TILE_SIZE,
	  (qreal)scaledSize.height() / TILE_SIZE);

	*image = QImage(size, QImage::Format_ARGB32_Premultiplied);
	Tile tile(image, zoom, scale);

	_style->render(data, tile);

	return true;
}

bool PBFHandler::supportsOption(ImageOption option) const
{
	return (option == Size || option == ScaledSize);
}


void PBFHandler::setOption(QImageIOHandler::ImageOption option,
  const QVariant &value)
{
	if (option == ScaledSize)
		_scaledSize = value.toSize();
}

QVariant PBFHandler::option(ImageOption option) const
{
	return (option == Size) ? QSize(TILE_SIZE, TILE_SIZE) : QVariant();
}
