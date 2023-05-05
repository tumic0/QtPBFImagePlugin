#ifndef TILE_H
#define TILE_H

#include <QImage>
#include <QPainter>
#include "text.h"

class Tile {
public:
	Tile(QImage *img, int zoom, const QPointF &scale)
	  : _zoom(zoom), _size(img->size()), _scale(scale),
	  _text(QSize(img->size().width() / scale.x(),
	  img->size().height() / scale.y())), _painter(img)
	{
		img->fill(Qt::transparent);
		_painter.scale(scale.x(), scale.y());
	}

	int zoom() const {return _zoom;}
	const QSize &size() const {return _size;}
	const QPointF &scale() const {return _scale;}

	Text &text() {return _text;}
	QPainter &painter() {return _painter;}

private:
	int _zoom;
	QSize _size;
	QPointF _scale;
	Text _text;
	QPainter _painter;
};

#endif // TILE_H
