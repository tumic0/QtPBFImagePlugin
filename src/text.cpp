#include <QGraphicsPixmapItem>
#include <QFontMetrics>
#include <QPainter>
#include "text.h"
#include "textpathitem.h"


static QPainterPath subpath(const QPainterPath &path, int start, int end)
{
	QPainterPath p(path.elementAt(start));
	for (int i = start + 1; i <= end; i++)
		p.lineTo(path.elementAt(i));
	return p;
}

static QList<QPainterPath> segments(const QPainterPath &path, qreal segmentLimit,
  qreal pathLimit)
{
	QList<QPainterPath> list;
	int start = 0;
	qreal length = 0;

	for (int i = 1; i < path.elementCount(); i++) {
		QLineF l(path.elementAt(i-1), path.elementAt(i));
		qreal sl = l.length();
		if (sl < segmentLimit || length > pathLimit) {
			if (length > pathLimit)
				list.append(subpath(path, start, i - 1));
			start = i;
			length = 0;
		} else
			length += sl;
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

	QFontMetrics fm(font);
	int limit = fm.width('M') * maxTextWidth;
	int flags = Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip;

	QRect br = fm.boundingRect(QRect(0, 0, limit, 0), flags, text);
	if (!br.isValid())
		return;
	br.moveTo((pos - QPointF(br.width() / 2.0, br.height() / 2.0)).toPoint());
	if (!sceneRect().contains(br))
		return;
	QPixmap pm(br.size());
	pm.fill(Qt::transparent);
	QPainter p(&pm);
	p.setFont(font);
	p.setPen(pen);
	p.drawText(pm.rect(), flags, text);

	QGraphicsPixmapItem *pi = addPixmap(pm);
	pi->setPos(br.topLeft());

	QList<QGraphicsItem*> ci = collidingItems(pi);
	for (int i = 0; i < ci.size(); i++)
		ci[i]->setVisible(false);
}

void Text::addLabel(const QString &text, const QPainterPath &path,
  const QFont &font, const QPen &pen)
{
	if (path.elementCount() < 2 || !path.elementAt(0).isMoveTo())
		return;
	if (text.isEmpty())
		return;

	QFontMetrics fm(font);
	int textWidth = fm.width(text);

	if (textWidth > path.length())
		return;

	QList<QPainterPath> list(segments(path, fm.width('M'), textWidth));
	for (int i = 0; i < list.size(); i++) {
		const QPainterPath &segment = list.at(i);
		TextPathItem *pi = new TextPathItem(text, reverse(segment)
		  ? segment.toReversed() : segment, font);
		pi->setPen(pen);
		addItem(pi);

		if (!sceneRect().contains(pi->sceneBoundingRect())) {
			delete pi;
			continue;
		}

		QList<QGraphicsItem*> ci = collidingItems(pi);
		for (int j = 0; j < ci.size(); j++)
			ci[j]->setVisible(false);
	}
}
