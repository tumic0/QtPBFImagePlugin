#ifndef TEXTPATHITEM_H
#define TEXTPATHITEM_H

#include "textitem.h"

class TextPathItem : public TextItem
{
public:
	TextPathItem(const QString &text, const QPainterPath &path,
	  const QFont &font, int maxAngle, const QRectF &tileRect);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter) const;

private:
	QPainterPath _path;
	QPainterPath _shape;
	QRectF _boundingRect;
};

#endif // TEXTPATHITEM_H
