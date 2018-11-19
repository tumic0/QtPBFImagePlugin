#ifndef TEXTPOINTITEM_H
#define TEXTPOINTITEM_H

#include <QPen>
#include <QFont>
#include <QString>
#include "textitem.h"

class TextPointItem : public TextItem
{
public:
	TextPointItem(const QString &text, const QPointF &pos, const QFont &font,
	  int maxTextWidth);

	QRectF boundingRect() const {return _boundingRect;}
	QPainterPath shape() const {return _shape;}
	void paint(QPainter *painter) const;

	void setPen(const QPen &pen) {_pen = pen;}

private:
	QPainterPath _shape;
	QRectF _boundingRect;
	QFont _font;
	QPen _pen;
};

#endif // TEXTPOINTITEM_H
