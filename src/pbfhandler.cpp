#include <QImage>
#include <QIODevice>
#include <QtEndian>
#include "gzip.h"
#include "pbf.h"
#include "pbfhandler.h"


#define TILE_SIZE 256

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
	return canRead(device());
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

	bool ok;
	int zoom = format().toInt(&ok);

	QSize size = _scaledSize.isValid()
	  ? _scaledSize : QSize(TILE_SIZE, TILE_SIZE);
	qreal scale = _scaledSize.isValid()
	  ? qMax(_scaledSize.width() / TILE_SIZE, _scaledSize.height() / TILE_SIZE)
	  : 1.0;
	*image = QImage(size, QImage::Format_ARGB32);

	return PBF::render(ba, ok ? zoom : -1, _style, scale, image);
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

QByteArray PBFHandler::name() const
{
	return "pbf";
}
