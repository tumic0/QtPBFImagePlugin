#include <QFontMetrics>
#include <QPainter>
#include "textpathitem.h"


#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#define INTERSECTS intersect
#else // QT 5.15
#define INTERSECTS intersects
#endif // QT 5.15

static bool intersection(const QLineF &line, const QRectF &rect,
  QPointF *p)
{
	if (line.INTERSECTS(QLineF(rect.topLeft(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.INTERSECTS(QLineF(rect.topLeft(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.INTERSECTS(QLineF(rect.bottomRight(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection)
		return true;
	if (line.INTERSECTS(QLineF(rect.bottomRight(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		return true;

	return false;
}

static bool intersection(const QLineF &line, const QRectF &rect, QPointF *p1,
  QPointF *p2)
{
	QPointF *p = p1;

	if (line.INTERSECTS(QLineF(rect.topLeft(), rect.topRight()), p)
	  == QLineF::BoundedIntersection)
		p = p2;
	if (line.INTERSECTS(QLineF(rect.topLeft(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2)
			return true;
		p = p2;
	}
	if (line.INTERSECTS(QLineF(rect.bottomRight(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2)
			return true;
		p = p2;
	}
	if (line.INTERSECTS(QLineF(rect.bottomRight(), rect.topRight()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2)
			return true;
	}

	Q_ASSERT(p != p2);

	return false;
}

static QPainterPath subpath(const QList<QLineF> &lines, int start, int end,
  qreal cut)
{
	qreal ss = 0, es = 0;
	int si = start, ei = end;

	for (int i = start; i <= end; i++) {
		qreal len = lines.at(i).length();
		if (ss + len < cut / 2) {
			ss += len;
			si++;
		} else
			break;
	}
	for (int i = end; i >= start; i--) {
		qreal len = lines.at(i).length();
		if (es + len < cut / 2) {
			es += len;
			ei--;
		} else
			break;
	}

	QLineF sl(lines.at(si).p2(), lines.at(si).p1());
	sl.setLength(sl.length() - (cut / 2 - ss));
	QLineF el(lines.at(ei));
	el.setLength(el.length() - (cut / 2 - es));

	QPainterPath p(sl.p2());
	for (int i = si; i <= ei; i++)
		p.lineTo(lines.at(i).p2());
	p.setElementPositionAt(p.elementCount() - 1, el.p2().x(), el.p2().y());

	return p;
}

static QList<QLineF> lineString(const QPainterPath &path,
  const QRectF &boundingRect)
{
	QList<QLineF> lines;
	int start = -1, end = -1;


	for (int i = 0; i < path.elementCount(); i++) {
		if (boundingRect.contains(path.elementAt(i))) {
			start = i;
			break;
		}
	}
	for (int i = path.elementCount() - 1; i >= 0; i--) {
		if (boundingRect.contains(path.elementAt(i))) {
			end = i;
			break;
		}
	}

	if (start < 0) {
		QPointF p1, p2;

		for (int i = 1; i < path.elementCount(); i++) {
			QLineF l(path.elementAt(i-1), path.elementAt(i));
			if (intersection(l, boundingRect, &p1, &p2)) {
				lines.append(QLineF(p1, p2));
				break;
			}
		}
	} else {
		QPointF p;

		if (start > 0) {
			QLineF l(path.elementAt(start-1), path.elementAt(start));
			if (intersection(l, boundingRect, &p))
				lines.append(QLineF(p, path.elementAt(start)));
		}
		for (int i = start + 1; i <= end; i++)
			lines.append(QLineF(path.elementAt(i-1), path.elementAt(i)));
		if (end < path.elementCount() - 1) {
			QLineF l(path.elementAt(end), path.elementAt(end+1));
			if (intersection(l, boundingRect, &p))
				lines.append(QLineF(path.elementAt(end), p));
		}
	}

	return lines;
}

static QPainterPath textPath(const QPainterPath &path, qreal textWidth,
  qreal maxAngle, qreal charWidth, const QRectF &tileRect)
{
	QList<QLineF> lines(lineString(path, tileRect));
	if (lines.isEmpty())
		return QPainterPath();
	qreal length = 0;
	qreal angle = lines.first().angle();
	int last = 0;

	for (int i = 0; i < lines.size(); i++) {
		qreal sl = lines.at(i).length();
		qreal a = lines.at(i).angle();

		if (!tileRect.contains(lines.at(i).p2()) || sl < charWidth
		  || qAbs(angle - a) > maxAngle) {
			if (length > textWidth)
				return subpath(lines, last, i - 1, length - textWidth);
			last = i;
			length = 0;
		} else
			length += sl;

		angle = a;
	}

	return (length > textWidth)
	  ? subpath(lines, last, lines.size() - 1, length - textWidth)
	  : QPainterPath();
}

static bool reverse(const QPainterPath &path)
{
	QLineF l(path.elementAt(0), path.elementAt(1));
	qreal angle = l.angle();
	return (angle > 90 && angle < 270) ? true : false;
}


TextPathItem::TextPathItem(const QString &text, const QPainterPath &path,
  const QFont &font, int maxAngle, const QRectF &tileRect)
  : TextItem(text, font)
{
	qreal cw = avgCharWidth();
	int textWidth = text.size() * cw;
	if (textWidth > path.length())
		return;

	QPainterPath tp(textPath(path, textWidth, maxAngle, cw, tileRect));
	if (tp.isEmpty())
		return;

	_path = reverse(tp) ? tp.toReversed() : tp;

	QPainterPathStroker s;
	s.setWidth(font.pixelSize());
	s.setCapStyle(Qt::FlatCap);
	_shape = s.createStroke(_path).simplified();
	_boundingRect = _shape.boundingRect();
}

void TextPathItem::paint(QPainter *painter) const
{
	//painter->setPen(Qt::red);
	//painter->drawPath(_shape);

	QFontMetrics fm(font());
	int textWidth = fm.boundingRect(text()).width();

	qreal factor = (textWidth) / qMax(_path.length(), (qreal)textWidth);
	qreal percent = (1.0 - factor) / 2.0;

	painter->setFont(font());

	QTransform t = painter->transform();

	if (halo().color().isValid() && halo().width() > 0) {
		painter->setPen(halo().color());
		for (int i = 0; i < text().size(); i++) {
			QPointF point = _path.pointAtPercent(percent);
			qreal angle = _path.angleAtPercent(percent);

			painter->translate(point);
			painter->rotate(-angle);
			painter->drawText(QPoint(-1, fm.descent() - 1), text().at(i));
			painter->drawText(QPoint(1, fm.descent() + 1), text().at(i));
			painter->drawText(QPoint(-1, fm.descent() + 1), text().at(i));
			painter->drawText(QPoint(1, fm.descent() -1), text().at(i));
			painter->drawText(QPoint(0, fm.descent() - 1), text().at(i));
			painter->drawText(QPoint(0, fm.descent() + 1), text().at(i));
			painter->drawText(QPoint(-1, fm.descent()), text().at(i));
			painter->drawText(QPoint(1, fm.descent()), text().at(i));
			painter->setTransform(t);

			int width = fm.horizontalAdvance(text().at(i));
			percent += ((qreal)width / (qreal)textWidth) * factor;
		}

		percent = (1.0 - factor) / 2.0;
	}

	painter->setPen(pen());
	for (int i = 0; i < text().size(); i++) {
		QPointF point = _path.pointAtPercent(percent);
		qreal angle = _path.angleAtPercent(percent);

		painter->translate(point);
		painter->rotate(-angle);
		painter->drawText(QPoint(0, fm.descent()), text().at(i));
		painter->setTransform(t);

		int width = fm.horizontalAdvance(text().at(i));
		percent += ((qreal)width / (qreal)textWidth) * factor;
	}
}
