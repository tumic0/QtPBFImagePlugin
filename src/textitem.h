#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QGraphicsItem>
#include <QPen>
#include <QFont>
#include <QString>

class TextItem : public QGraphicsItem
{
public:
	TextItem(const QString &text, const QPointF &pos, const QFont &font,
	  int maxTextWidth, QGraphicsItem *parent = 0);

	QRectF boundingRect() const {return _boundingRect;}
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
	  QWidget *widget);

	void setPen(const QPen &pen) {_pen = pen;}

private:
	QString _text;
	QRectF _boundingRect;
	QFont _font;
	QPen _pen;
};

#endif // TEXTITEM_H
