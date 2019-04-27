#ifndef TEXT_H
#define TEXT_H

#include <QList>
#include <QRectF>
#include <QFont>
#include <QPen>

class QImage;
class QPainterPath;
class QPainter;
class TextItem;

class Text
{
public:
	enum Anchor {
		Center,
		Left,
		Right,
		Top,
		Bottom
	};

	enum SymbolPlacement {
		Point,
		Line,
		LineCenter
	};

	enum RotationAlignment {
		Map,
		Viewport,
		Auto
	};

	class Halo {
	public:
		Halo() : _width(0), _blur(0) {}
		Halo(const QColor &color, qreal width, qreal blur)
		  : _color(color), _width(width), _blur(blur) {}

		const QColor &color() const {return _color;}
		qreal width() const {return _width;}
		qreal blur() const {return _blur;}

	private:
		QColor _color;
		qreal _width;
		qreal _blur;
	};

	Text(const QSize &size) : _sceneRect(QRectF(QPointF(0, 0), size)) {}
	~Text();

	void setFont(const QFont &font) {_font = font;}
	void setPen(const QPen &pen) {_pen = pen;}
	void setAnchor(Anchor anchor) {_anchor = anchor;}
	void setMaxWidth(int width) {_maxWidth = width;}
	void setMaxAngle(int angle) {_maxAngle = angle;}
	void setSymbolPlacement(SymbolPlacement placement);
	void setRotationAlignment(RotationAlignment alignment)
	  {_alignment = alignment;}
	void setHalo(const Halo &halo) {_halo = halo;}

	void addLabel(const QString &text, const QImage &icon,
	  const QPainterPath &path);

	void render(QPainter *painter) const;

private:
	void addItem(TextItem *item) {_items.append(item);}
	QList<TextItem *> collidingItems(const TextItem *item) const;

	QRectF _sceneRect;
	QList<TextItem *> _items;

	int _maxWidth;
	int _maxAngle;
	Anchor _anchor;
	SymbolPlacement _placement;
	RotationAlignment _alignment;
	QFont _font;
	QPen _pen;
	Halo _halo;
};

#endif // TEXT_H
