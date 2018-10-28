#ifndef GZIP_H
#define GZIP_H

#include <QByteArray>

namespace Gzip
{
	QByteArray uncompress(const QByteArray &data);
}

#endif // GZIP_H
