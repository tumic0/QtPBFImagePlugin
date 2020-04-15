#ifndef GZIP_H
#define GZIP_H

#include <QByteArray>

class QIODevice;

namespace Gzip
{
	QByteArray uncompress(QIODevice *device, int limit = 0);
}

#endif // GZIP_H
