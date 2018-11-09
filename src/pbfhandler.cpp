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

	if ((qFromBigEndian(magic) & GZIP_MAGIC_MASK) == GZIP_MAGIC)
		return true;
	if ((qFromBigEndian(magic) & PBF_MAGIC_MASK) == PBF_MAGIC)
		return true;

	return false;
}

bool PBFHandler::read(QImage *image)
{
	quint32 magic;
	qint64 size = device()->peek((char*)&magic, sizeof(magic));
	if (size != sizeof(magic))
		return false;

	QByteArray ba;

	if ((qFromBigEndian(magic) & GZIP_MAGIC_MASK) == GZIP_MAGIC)
		ba = Gzip::uncompress(device()->readAll());
	else if ((qFromBigEndian(magic) & PBF_MAGIC_MASK) == PBF_MAGIC)
		ba = device()->readAll();
	if (ba.isNull())
		return false;

	bool ok;
	int zoom = format().toInt(&ok);
	*image = PBF::image(ba, ok ? zoom : -1, _style, TILE_SIZE);

	return !image->isNull();
}

bool PBFHandler::supportsOption(ImageOption option) const
{
	return (option == Size);
}

QVariant PBFHandler::option(ImageOption option) const
{
	return (option == Size) ? QSize(TILE_SIZE, TILE_SIZE) : QVariant();
}

QByteArray PBFHandler::name() const
{
	return "pbf";
}
