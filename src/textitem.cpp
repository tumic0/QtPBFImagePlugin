#include "textitem.h"

bool TextItem::collidesWithItem(const TextItem *other) const
{
	QRectF r1(boundingRect());
	QRectF r2(other->boundingRect());

	if (r1.isEmpty() || r2.isEmpty() || !r1.intersects(r2))
		return false;

	return other->shape().intersects(shape());
}

qreal TextItem::avgCharWidth() const
{
	qreal ratio;
	ushort cp = _text.at(0).unicode();

	// CJK & East Asian scripts
	if (cp >= 0x2E80)
		ratio = 1.0;
	// Greek & Cyrilic
	else if (cp >= 0x03FF && cp <= 0x04FF) {
		ratio = (_font.capitalization() == QFont::AllUppercase) ? 0.80 : 0.73;
		if (_font.bold())
			ratio *= 1.1;
		if (_font.italic())
			ratio *= 0.9;
	// The rest (Latin scripts, Arabic, ...)
	} else {
		ratio = (_font.capitalization() == QFont::AllUppercase) ? 0.75 : 0.63;
		if (_font.bold())
			ratio *= 1.1;
		if (_font.italic())
			ratio *= 0.9;
	}

	return ratio * _font.pixelSize();
}
