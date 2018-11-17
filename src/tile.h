#ifndef TILE_H
#define TILE_H

#include <QImage>
#include <QPainter>
#include "text.h"

class Tile {
public:
	Tile(QImage *img, const QPointF &scale)
	  : _size(img->size()), _text(QSize(img->size().width() / scale.x(),
	  img->size().height() / scale.y())), _painter(img)
	  {_painter.scale(scale.x(), scale.y());}

	QSize size() const {return _size;}
	Text &text() {return _text;}
	QPainter &painter() {return _painter;}

private:
	QSize _size;
	Text _text;
	QPainter _painter;
};

#endif // TILE_H
