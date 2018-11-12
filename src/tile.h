#ifndef TILE_H
#define TILE_H

#include <QImage>
#include <QPainter>
#include "text.h"

class Tile {
public:
	Tile(QImage *img, qreal scale)
	  : _size(img->size()), _text(img->size(), scale), _painter(img) {}

	QSize size() const {return _size;}
	Text &text() {return _text;}
	QPainter &painter() {return _painter;}

	void render() {_text.render(&_painter);}

private:
	QSize _size;
	Text _text;
	QPainter _painter;
};

#endif // TILE_H
