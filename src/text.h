#ifndef TEXT_H
#define TEXT_H

#include "textitem.h"

class Text
{
public:
	Text(const QSize &size, qreal scale)
	  : _sceneRect(QRectF(QPointF(0, 0), size)), _fontScale(scale) {}
	~Text();

	void render(QPainter *painter) const;

	void addLabel(const QString &text, const QPointF &pos,
	  const QPainter &painter, qreal maxTextWidth);
	void addLabel(const QString &text, const QPainterPath &path,
	  const QPainter &painter, qreal maxAngle);

private:
	void addItem(TextItem *item) {_items.append(item);}
	QList<TextItem *> collidingItems(const TextItem *item) const;

	QRectF _sceneRect;
	qreal _fontScale;
	QList<TextItem *> _items;
};

#endif // TEXT_H
