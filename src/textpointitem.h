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

private:
	typedef QRectF (*BoundingRectFunction)(const QString &, const QFont &, int);

	static QRectF exactBoundingRect(const QString &str, const QFont &font,
	  int maxWidth);
	static QRectF fuzzyBoundingRect(const QString &str, const QFont &font,
	  int maxWidth);

	QRectF computeTextRect(BoundingRectFunction brf) const;

	QPointF _pos;
	QPainterPath _shape;
	QRectF _boundingRect;
	QImage _icon;
	int _maxWidth;
	Text::Anchor _anchor;
};

#endif // TEXTPOINTITEM_H
