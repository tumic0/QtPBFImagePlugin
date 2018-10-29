#ifndef PBF_H
#define PBF_H

#include <QImage>

class QByteArray;
class Style;

namespace PBF
{
	QImage image(const QByteArray &data, int zoom, Style *style, int size);
}

#endif // PBF_H
