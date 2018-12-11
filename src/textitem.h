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
	TextItem(const QString &text, const QFont &font)
	  : _text(text), _font(font), _visible(true) {}
	virtual ~TextItem() {}

	const QString &text() const {return _text;}
	const QFont &font() const {return _font;}
	const QPen &pen() const {return _pen;}
	void setPen(const QPen &pen) {_pen = pen;}

	virtual QPainterPath shape() const = 0;
	virtual QRectF boundingRect() const = 0;
	virtual void paint(QPainter *painter) const = 0;

	bool isVisible() const {return _visible;}
	void setVisible(bool visible) {_visible = visible;}

	bool collidesWithItem(const TextItem *other) const;

protected:
	int avgCharWidth() const;

private:
	QString _text;
	QFont _font;
	QPen _pen;
	bool _visible;
};

#endif // TEXTITEM_H
