#include <QIODevice>
#include <zlib.h>
#include "gzip.h"

#define CHUNK 16384

QByteArray Gzip::uncompress(QIODevice *device, int limit)
{
	int ret = Z_STREAM_END;
	z_stream strm;
	unsigned char in[CHUNK];
	unsigned char out[CHUNK];
	qint64 rs;
	QByteArray uba;


	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	if (inflateInit2(&strm, MAX_WBITS + 16) != Z_OK)
		return QByteArray();

	do {
		rs = device->read((char*)in, CHUNK);
		if (rs < 0) {
			(void)inflateEnd(&strm);
			return QByteArray();
		} else if (rs == 0)
			break;
		else
			strm.avail_in = (uInt)rs;
		strm.next_in = in;

		do {
			strm.avail_out = CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			Q_ASSERT(ret != Z_STREAM_ERROR);
			switch (ret) {
				case Z_NEED_DICT:
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return QByteArray();
			}
			uba.append((char*)out, CHUNK - strm.avail_out);
			if (limit && uba.size() >= limit) {
				(void)inflateEnd(&strm);
				return uba;
			}
		} while (!strm.avail_out);
	} while (ret != Z_STREAM_END);

	(void)inflateEnd(&strm);
	return (ret == Z_STREAM_END) ? uba : QByteArray();
}
