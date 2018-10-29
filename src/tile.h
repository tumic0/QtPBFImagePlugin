#ifndef TILE_H
#define TILE_H

#include <QImage>
#include <QPainter>
#include "text.h"

class Tile {
public:
	Tile(int size) : _background(size, size, QImage::Format_ARGB32),
	  _text(size), _painter(&_background) {}

	int size() const {return _background.width();}
	Text &text() {return _text;}
	QPainter *painter() {return &_painter;}

	QImage &render() {
		_text.render(painter());
		return _background;
	}

private:
	QImage _background;
	Text _text;
	QPainter _painter;
};

#endif // TILE_H
