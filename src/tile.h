#ifndef TILE_H
#define TILE_H

#include <QImage>
#include <QPainter>
#include "text.h"

class Tile {
public:
	Tile() : _background(256, 256, QImage::Format_ARGB32) {}

	Text &text() {return _text;}
	QImage &background() {return _background;}

	QImage &render() {
		QPainter p(&_background);
		_text.render(&p);
		return _background;
	}

private:
	Text _text;
	QImage _background;
};

#endif // TILE_H
