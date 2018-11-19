#ifndef TEXTPATHITEM_H
#define TEXTPATHITEM_H

#include <QFont>
#include <QString>
#include "textitem.h"

class TextPathItem : public TextItem
{
public:
	TextPathItem(const QString &text, const QPainterPath &path,
	  const QFont &font);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter) const;

	void setPen(const QPen &pen) {_pen = pen;}

private:
	QPainterPath _path;
	QPainterPath _shape;
	QRectF _boundingRect;
	QFont _font;
	QPen _pen;
};

#endif // TEXTPATHITEM_H
