#include <QFontMetrics>
#include <QPainter>
#include "textpathitem.h"


#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#define INTERSECTS intersect
#else // QT 5.15
#define INTERSECTS intersects
#endif // QT 5.15

static void swap(const QLineF &line, QPointF *p1, QPointF *p2)
{

	QPointF lp1(line.p1());
	QPointF lp2(line.p2());

	if ((lp1.rx() < lp2.rx() && p1->rx() > p2->rx())
	  || (lp1.ry() < lp2.ry() && p1->ry() > p2->ry())
	  || (lp1.rx() > lp2.rx() && p1->rx() < p2->rx())
	  || (lp1.ry() > lp2.ry() && p1->ry() < p2->ry())) {
		QPointF tmp(*p2);
		*p2 = *p1;
		*p1 = tmp;
	}
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
		if (p == p2) {
			swap(line, p1, p2);
			return true;
		}
		p = p2;
	}
	if (line.INTERSECTS(QLineF(rect.bottomRight(), rect.bottomLeft()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2) {
			swap(line, p1, p2);
			return true;
		}
		p = p2;
	}
	if (line.INTERSECTS(QLineF(rect.bottomRight(), rect.topRight()), p)
	  == QLineF::BoundedIntersection) {
		if (p == p2) {
			swap(line, p1, p2);
			return true;
		}
	}

	Q_ASSERT(p != p2);

	return false;
}

static bool intersection(const QLineF &line, const QRectF &rect, QPointF *p)
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

static QPainterPath subpath(const QPolygonF &path, int start, int end,
  qreal cut)
{
	qreal ss = 0, es = 0;
	int si = start, ei = end;

	for (int i = start; i < end; i++) {
		QLineF l(path.at(i), path.at(i+1));
		qreal len = l.length();
		if (ss + len < cut / 2) {
			ss += len;
			si++;
		} else
			break;
	}
	for (int i = end; i > start; i--) {
		QLineF l(path.at(i), path.at(i-1));
		qreal len = l.length();
		if (es + len < cut / 2) {
			es += len;
			ei--;
		} else
			break;
	}

	QLineF sl(path.at(si+1), path.at(si));
	sl.setLength(sl.length() - (cut / 2 - ss));
	QLineF el(path.at(ei-1), path.at(ei));
	el.setLength(el.length() - (cut / 2 - es));

	QPainterPath p(sl.p2());
	for (int i = si + 1; i < ei; i++)
		p.lineTo(path.at(i));
	p.lineTo(el.p2());

	return p;
}

static QList<QPolygonF> polyLines(const QPainterPath &path, const QRectF &rect)
{
	QList<QPolygonF> lines;
	QPolygonF line;
	bool lastIn = rect.contains(path.elementAt(0));

	for (int i = 1; i < path.elementCount(); i++) {
		if (rect.contains(path.elementAt(i))) {
			if (lastIn) {
				if (line.isEmpty())
					line.append(path.elementAt(i-1));
				line.append(path.elementAt(i));
			} else {
				QPointF p;
				QLineF l(path.elementAt(i-1), path.elementAt(i));

				if (intersection(l, rect, &p))
					line.append(p);
				line.append(path.elementAt(i));
			}

			lastIn = true;
		} else {
			QLineF l(path.elementAt(i-1), path.elementAt(i));

			if (lastIn) {
				QPointF p;
				if (line.isEmpty())
					line.append(path.elementAt(i-1));
				if (intersection(l, rect, &p))
					line.append(p);
				lines.append(line);
				line.clear();
			} else {
				QPointF p1, p2;
				if (intersection(l, rect, &p1, &p2)) {
					line.append(p1);
					line.append(p2);
					lines.append(line);
					line.clear();
				}
			}

			lastIn = false;
		}
	}

	if (!line.isEmpty())
		lines.append(line);

	return lines;
}

static QPainterPath textPath(const QPainterPath &path, qreal textWidth,
  qreal maxAngle, qreal charWidth, const QRectF &tileRect)
{
	if (path.isEmpty())
		return QPainterPath();

	QList<QPolygonF> lines(polyLines(path, tileRect));

	for (int i = 0; i < lines.size(); i++) {
		const QPolygonF &pl = lines.at(i);
		qreal angle, length = 0;
		int last = 0;

		for (int j = 1; j < pl.size(); j ++) {
			QLineF l(pl.at(j-1), pl.at(j));
			qreal sl = l.length();
			qreal a = l.angle();

			if ((sl < charWidth) || (j > 1 && qAbs(angle - a) > maxAngle)) {
				if (length > textWidth)
					return subpath(pl, last, j - 1, length - textWidth);
				last = j;
				length = 0;
			} else
				length += sl;

			angle = a;
		}

		if (length > textWidth)
			return subpath(pl, last, pl.size() - 1, length - textWidth);
	}

	return QPainterPath();
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

	//painter->setBrush(Qt::NoBrush);
	//painter->setPen(Qt::red);
	//painter->setRenderHint(QPainter::Antialiasing, false);
	//painter->drawPath(_shape);
}
