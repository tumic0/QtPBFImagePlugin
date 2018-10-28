#ifndef TEXTPATHITEM_H
#define TEXTPATHITEM_H

#include <QGraphicsItem>
#include <QPainterPath>
#include <QFont>
#include <QString>

class TextPathItem : public QGraphicsItem
{
public:
	TextPathItem(const QString &text, const QPainterPath &path,
	  const QFont &font, QGraphicsItem *parent = 0);

	QPainterPath shape() const {return _shape;}
	QRectF boundingRect() const {return _shape.boundingRect();}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setPen(const QPen &pen) {_pen = pen;}

private:
	QString _text;
	QPainterPath _path;
	QPainterPath _shape;
	QFont _font;
	QPen _pen;
};

#endif // TEXTPATHITEM_H
