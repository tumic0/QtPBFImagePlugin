#ifndef PBF_H
#define PBF_H

#include <QImage>

class QByteArray;
class Style;

namespace PBF
{
	bool render(const QByteArray &data, int zoom, Style *style, qreal scale,
	  QImage *render);
}

#endif // PBF_H
