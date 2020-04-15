#ifndef GZIP_H
#define GZIP_H

#include <QByteArray>

class QIODevice;

namespace Gzip
{
	QByteArray uncompress(QIODevice *device, qint64 limit = 0);
}

#endif // GZIP_H
