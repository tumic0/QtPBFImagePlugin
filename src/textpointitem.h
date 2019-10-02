#ifndef TEXTPOINTITEM_H
#define TEXTPOINTITEM_H

#include <QImage>
#include "text.h"
#include "textitem.h"

class TextPointItem : public TextItem
{
public:
	TextPointItem(const QString &text, const QPointF &pos, const QFont &font,
	  int maxWidth, Text::Anchor anchor, const QImage &icon);

	QRectF boundingRect() const {return _boundingRect;}
	QPainterPath shape() const {return _shape;}
	void paint(QPainter *painter) const;

	void setPos(const QPointF &pos);

private:
	QRectF exactBoundingRect() const;
	QRectF fuzzyBoundingRect() const;
	QRectF computeTextRect(bool exact) const;
	bool hasHalo() const
	  {return halo().color().isValid() && halo().width() > 0;}

	QPointF _pos;
	QPainterPath _shape;
	QRectF _textRect, _boundingRect;
	QImage _icon;
	int _maxWidth;
	Text::Anchor _anchor;
};

#endif // TEXTPOINTITEM_H
