#ifndef PBF_H
#define PBF_H

class QByteArray;
class QImage;
class Style;
class QPointF;

namespace PBF
{
	bool render(const QByteArray &data, int zoom, const Style *style,
	  const QPointF &scale, QImage *render);
}

#endif // PBF_H
