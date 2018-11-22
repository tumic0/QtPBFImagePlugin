#ifndef TEXTPOINTITEM_H
#define TEXTPOINTITEM_H

#include <QPen>
#include <QFont>
#include <QString>
#include "textitem.h"
#include "text.h"

class TextPointItem : public TextItem
{
public:
	TextPointItem(const QString &text, const QPointF &pos, const QFont &font,
	  const Text::Properties &prop, const QImage &icon);

	QRectF boundingRect() const {return _boundingRect;}
	QPainterPath shape() const {return _shape;}
	void paint(QPainter *painter) const;

	void setPen(const QPen &pen) {_pen = pen;}

private:
	typedef QRectF (*BoundingRectFunction)(const QString &, const QFont &, int);
	QRectF computeTextRect(BoundingRectFunction brf) const;

	QPointF _pos;
	QPainterPath _shape;
	QRectF _boundingRect;
	QFont _font;
	QPen _pen;
	QImage _icon;
	Text::Properties _properties;
};

#endif // TEXTPOINTITEM_H
