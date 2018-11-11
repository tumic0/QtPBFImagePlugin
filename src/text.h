#ifndef TEXT_H
#define TEXT_H

#include "textitem.h"

class Text
{
public:
	Text(int size) : _sceneRect(QRectF(0, 0, size, size)) {}
	~Text();

	void render(QPainter *painter);

	void addLabel(const QString &text, const QPointF &pos, const QFont &font,
	  const QPen &pen, qreal maxTextWidth);
	void addLabel(const QString &text, const QPainterPath &path,
	  const QFont &font, const QPen &pen, qreal maxAngle);

private:
	void addItem(TextItem *item) {_items.append(item);}
	QList<TextItem *> collidingItems(const TextItem *item) const;

	QRectF _sceneRect;
	QList<TextItem *> _items;
};

#endif // TEXT_H
