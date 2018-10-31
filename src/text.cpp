#include <QGraphicsPixmapItem>
#include <QFontMetrics>
#include <QPainter>
#include "text.h"
#include "textitem.h"
#include "textpathitem.h"


static QPainterPath subpath(const QPainterPath &path, int start, int end)
{
	QPainterPath p(path.elementAt(start));
	for (int i = start + 1; i <= end; i++)
		p.lineTo(path.elementAt(i));
	return p;
}

static QList<QPainterPath> segments(const QPainterPath &path, qreal pathLimit,
  qreal maxAngle, qreal charWidth, const QRectF &tileRect)
{
	QList<QPainterPath> list;
	int start = 0;
	qreal length = 0;
	qreal angle = -1;

	for (int i = 1; i < path.elementCount(); i++) {
		QLineF l(path.elementAt(i-1), path.elementAt(i));
		qreal a = l.angle();
		qreal sl = l.length();
		if (angle < 0)
			angle = a;
		if (!tileRect.contains(path.elementAt(i))
		  || sl < charWidth || qAbs(angle - a) > maxAngle
		  || length > pathLimit) {
			if (length > pathLimit)
				list.append(subpath(path, start, i - 1));
			start = i;
			length = 0;
		} else
			length += sl;
		angle = a;
	}

	if (length > pathLimit)
		list.append(subpath(path, start, path.elementCount() - 1));

	return list;
}

static bool reverse(const QPainterPath &path)
{
	QLineF l(path.elementAt(0), path.elementAt(1));
	qreal angle = l.angle();
	return (angle > 90 && angle < 270) ? true : false;
}

void Text::addLabel(const QString &text, const QPointF &pos, const QFont &font,
  const QPen &pen, qreal maxTextWidth)
{
	if (text.isEmpty())
		return;
	if (!sceneRect().contains(pos))
		return;

	TextItem *ti = new TextItem(text, pos, font, maxTextWidth);
	addItem(ti);
	if (!sceneRect().contains(ti->sceneBoundingRect())) {
		delete ti;
		return;
	}

	ti->setPen(pen);

	QList<QGraphicsItem*> ci = collidingItems(ti);
	for (int i = 0; i < ci.size(); i++)
		ci[i]->setVisible(false);
}

void Text::addLabel(const QString &text, const QPainterPath &path,
  const QFont &font, const QPen &pen, qreal maxAngle)
{
	if (path.elementCount() < 2 || !path.elementAt(0).isMoveTo())
		return;
	if (text.isEmpty())
		return;

	QFontMetrics fm(font);
	int textWidth = fm.width(text);

	if (textWidth > path.length())
		return;

	QList<QPainterPath> list(segments(path, textWidth, maxAngle,
	  fm.averageCharWidth(), sceneRect()));
	for (int i = 0; i < list.size(); i++) {
		const QPainterPath &segment = list.at(i);
		TextPathItem *pi = new TextPathItem(text, reverse(segment)
		  ? segment.toReversed() : segment, font);
		addItem(pi);
		if (!sceneRect().contains(pi->sceneBoundingRect())) {
			delete pi;
			continue;
		}

		pi->setPen(pen);

		QList<QGraphicsItem*> ci = collidingItems(pi);
		for (int j = 0; j < ci.size(); j++)
			ci[j]->setVisible(false);
	}
}
