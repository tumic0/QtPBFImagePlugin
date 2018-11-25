#ifndef TEXT_H
#define TEXT_H

#include <QImage>

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

	struct Properties {
		int maxWidth;
		int maxAngle;
		Anchor anchor;
	};


	Text(const QSize &size)
	  : _sceneRect(QRectF(QPointF(0, 0), size)) {}
	~Text();

	void setProperties(const Properties &prop)
	  {_properties = prop;}

	void addLabel(const QString &text, const QPointF &pos,
	  const QPainter &painter, bool overlap, const QImage &icon);
	void addLabel(const QString &text, const QPainterPath &path,
	  const QPainter &painter);

	void render(QPainter *painter) const;

private:
	void addItem(TextItem *item) {_items.append(item);}
	QList<TextItem *> collidingItems(const TextItem *item) const;

	QRectF _sceneRect;
	QList<TextItem *> _items;
	Properties _properties;
};

#endif // TEXT_H
