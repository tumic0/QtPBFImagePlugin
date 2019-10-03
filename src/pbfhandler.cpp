#include <QImage>
#include <QIODevice>
#include <QtEndian>
#include <QDebug>
#include "gzip.h"
#include "tile.h"
#include "style.h"
#include "pbfhandler.h"


#define TILE_SIZE 512

#define GZIP_MAGIC      0x1F8B0800
#define GZIP_MAGIC_MASK 0xFFFFFF00
#define PBF_MAGIC       0x1A000000
#define PBF_MAGIC_MASK  0xFF000000

static bool isMagic(quint32 magic, quint32 mask, quint32 value)
{
	return ((qFromBigEndian(value) & mask) == magic);
}

static bool isGZIPPBF(quint32 magic)
{
	return isMagic(GZIP_MAGIC, GZIP_MAGIC_MASK, magic);
}

static bool isPlainPBF(quint32 magic)
{
	return isMagic(PBF_MAGIC, PBF_MAGIC_MASK, magic);
}


bool PBFHandler::canRead() const
{
	if (canRead(device())) {
		setFormat("pbf");
		return true;
	} else
		return false;
}

bool PBFHandler::canRead(QIODevice *device)
{
	quint32 magic;
	qint64 size = device->peek((char*)&magic, sizeof(magic));
	if (size != sizeof(magic))
		return false;

	return (isGZIPPBF(magic) || isPlainPBF(magic));
}

bool PBFHandler::read(QImage *image)
{
	quint32 magic;
	if (device()->peek((char*)&magic, sizeof(magic)) != sizeof(magic))
		return false;

	QByteArray ba;
	if (isGZIPPBF(magic))
		ba = Gzip::uncompress(device()->readAll());
	else if (isPlainPBF(magic))
		ba = device()->readAll();
	if (ba.isNull())
		return false;
	vector_tile::Tile data;
	if (!data.ParseFromArray(ba.constData(), ba.size())) {
		qCritical() << "Invalid PBF data";
		return false;
	}

	bool ok;
	int zoom = format().toInt(&ok);

	QSize size = _scaledSize.isValid()
	  ? _scaledSize : QSize(TILE_SIZE, TILE_SIZE);
	QPointF scale = _scaledSize.isValid()
	  ? QPointF((qreal)_scaledSize.width() / TILE_SIZE,
		(qreal)_scaledSize.height() / TILE_SIZE) : QPointF(1.0, 1.0);
	*image = QImage(size, QImage::Format_ARGB32_Premultiplied);
	Tile tile(image, ok ? zoom : -1, scale);

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
