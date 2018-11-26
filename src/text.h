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

	Text(const QSize &size)
	  : _sceneRect(QRectF(QPointF(0, 0), size)) {}
	~Text();

	void setFont(const QFont &font) {_font = font;}
	void setPen(const QPen &pen) {_pen = pen;}
	void setAnchor(Anchor anchor) {_anchor = anchor;}
	void setMaxWidth(int width) {_maxWidth = width;}
	void setMaxAngle(int angle) {_maxAngle = angle;}

	void addLabel(const QString &text, const QPointF &pos, bool overlap,
	  const QImage &icon);
	void addLabel(const QString &text, const QPainterPath &path);

	void render(QPainter *painter) const;

	static qreal avgCharRatio(const QString &str, const QFont &font);

private:
	void addItem(TextItem *item) {_items.append(item);}
	QList<TextItem *> collidingItems(const TextItem *item) const;

	QRectF _sceneRect;
	QList<TextItem *> _items;

	int _maxWidth;
	int _maxAngle;
	Anchor _anchor;
	QFont _font;
	QPen _pen;
};

#endif // TEXT_H
