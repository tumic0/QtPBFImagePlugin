#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QPainterPath>

class TextItem
{
public:
	TextItem() : _visible(true) {}
	virtual ~TextItem() {}

	virtual QPainterPath shape() const = 0;
	virtual QRectF boundingRect() const = 0;
	virtual void paint(QPainter *painter) const = 0;

	bool isVisible() const {return _visible;}
	void setVisible(bool visible) {_visible = visible;}

	bool collidesWithItem(const TextItem *other) const
	{
		QRectF r1(boundingRect());
		QRectF r2(other->boundingRect());

		if (r1.isEmpty() || r2.isEmpty() || !r1.intersects(r2))
			return false;

		return other->shape().intersects(shape());
	}

private:
	bool _visible;
};

#endif // TEXTITEM_H
