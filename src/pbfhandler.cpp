#include <QImage>
#include <QIODevice>
#include <QtEndian>
#include "gzip.h"
#include "pbf.h"
#include "pbfhandler.h"


#define TILE_SIZE 512

#define GZIP_MAGIC 0x1F8B0800

bool PBFHandler::canRead() const
{
	return canRead(device());
}

bool PBFHandler::canRead(QIODevice *device)
{
	quint32 magic;
	qint64 size = device->peek((char*)&magic, sizeof(magic));
	return (size == sizeof(magic)
	  && (qFromBigEndian(magic) & 0xFFFFFF00) == GZIP_MAGIC);
}

bool PBFHandler::read(QImage *image)
{
	QByteArray ba = Gzip::uncompress(device()->readAll());
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
