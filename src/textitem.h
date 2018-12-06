#ifndef TEXTITEM_H
#define TEXTITEM_H

#include <QString>
#include <QPainterPath>
#include <QPen>
#include <QFont>
#include <QRectF>

class QPainter;

class TextItem
{
public:
	TextItem(const QString &text) : _text(text), _visible(true) {}
	virtual ~TextItem() {}

	const QString &text() const {return _text;}
	const QFont &font() const {return _font;}
	void setFont(const QFont &font) {_font = font;}
	const QPen &pen() const {return _pen;}
	void setPen(const QPen &pen) {_pen = pen;}

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

protected:
	static int avgCharWidth(const QString &str, const QFont &font)
	{
		qreal ratio;

		if (str.at(0).unicode() >= 0x2E80)
			ratio = 1.0;
		else {
			ratio = (font.capitalization() == QFont::AllUppercase)
			  ? 0.66 : 0.55;
			if (font.bold())
				ratio *= 1.1;
		}

		return ratio * font.pixelSize();
	}

private:
	QString _text;
	QFont _font;
	QPen _pen;
	bool _visible;
};

#endif // TEXTITEM_H
