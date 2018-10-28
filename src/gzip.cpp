#include <QtEndian>
#include <QDebug>
#include <zlib.h>
#include "gzip.h"


QByteArray Gzip::uncompress(const QByteArray &data)
{
	QByteArray uba;
	z_stream stream;

	quint32 *size = (quint32*)(data.constData() + data.size() - sizeof(quint32));
	uba.resize(qFromLittleEndian(*size));

	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;

	stream.next_in  = (Bytef*)data.constData();
	stream.avail_in = data.size();
	stream.next_out = (Bytef*)uba.data();
	stream.avail_out = uba.size();

	if (inflateInit2(&stream, MAX_WBITS + 16) != Z_OK)
		return uba;

	if (inflate(&stream, Z_NO_FLUSH) != Z_STREAM_END) {
		qCritical() << "Invalid gzip data";
		uba = QByteArray();
	}

	inflateEnd(&stream);

	return uba;
}
