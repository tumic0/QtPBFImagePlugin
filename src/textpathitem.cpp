#include <QFontMetrics>
#include <QPainter>
#include "textpathitem.h"


static QPointF intersection(const QLineF &line, const QRectF &rect)
{
	QPointF p;
	if (line.intersect(QLineF(rect.topLeft(), rect.topRight()), &p)
	  == QLineF::BoundedIntersection)
		return p;
	if (line.intersect(QLineF(rect.topLeft(), rect.bottomLeft()), &p)
	  == QLineF::BoundedIntersection)
		return p;
	if (line.intersect(QLineF(rect.bottomRight(), rect.bottomLeft()), &p)
	  == QLineF::BoundedIntersection)
		return p;
	if (line.intersect(QLineF(rect.bottomRight(), rect.topRight()), &p)
	  == QLineF::BoundedIntersection)
		return p;

	return rect.center();
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
	int start = 0, end = path.elementCount() - 1;

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

	if (start > 0) {
		QLineF l(path.elementAt(start-1), path.elementAt(start));
		QPointF p(intersection(l, boundingRect));
		if (p != boundingRect.center())
			lines.append(QLineF(p, path.elementAt(start)));
	}
	for (int i = start + 1; i <= end; i++)
		lines.append(QLineF(path.elementAt(i-1), path.elementAt(i)));
	if (end < path.elementCount() - 1) {
		QLineF l(path.elementAt(end), path.elementAt(end+1));
		QPointF p(intersection(l, boundingRect));
		if (p != boundingRect.center())
			lines.append(QLineF(path.elementAt(end), p));
	}

	return lines;
}

static QPainterPath textPath(const QPainterPath &path, qreal textWidth,
  qreal maxAngle, qreal charWidth, const QRectF &tileRect)
{
	QList<QLineF> lines(lineString(path, tileRect));
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
  : TextItem(text), _font(font)
{
	qreal acr = Text::avgCharRatio(text, _font);
	int textWidth = text.size() * _font.pixelSize() * acr;
	if (textWidth > path.length())
		return;

	QPainterPath tp(textPath(path, textWidth, maxAngle, _font.pixelSize() * acr,
	  tileRect));
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

	QFontMetrics fm(_font);
	int textWidth = fm.width(text());

	qreal factor = (textWidth) / qMax(_path.length(), (qreal)textWidth);
	qreal percent = (1.0 - factor) / 2.0;

	painter->setFont(_font);
	painter->setPen(_pen);

	QTransform t = painter->transform();

	for (int i = 0; i < text().size(); i++) {
		QPointF point = _path.pointAtPercent(percent);
		qreal angle = _path.angleAtPercent(percent);

		painter->translate(point);
		painter->rotate(-angle);
		painter->drawText(QPoint(0, fm.descent()), text().at(i));
		painter->setTransform(t);

		int width = fm.charWidth(text(), i);
		percent += ((qreal)width / (qreal)textWidth) * factor;
	}
}
