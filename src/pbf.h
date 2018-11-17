#ifndef PBF_H
#define PBF_H

#include <QImage>

class QByteArray;
class Style;

namespace PBF
{
	bool render(const QByteArray &data, int zoom, Style *style,
	  const QPointF &scale, QImage *render);
}

#endif // PBF_H
