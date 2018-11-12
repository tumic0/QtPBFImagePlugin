#include <QFontMetrics>
#include <QPainter>
#include "textpathitem.h"


TextPathItem::TextPathItem(const QString &text, const QPainterPath &path,
  const QFont &font) : _text(text), _path(path), _font(font)
{
	QPainterPathStroker s;
	s.setWidth(font.pixelSize());
	s.setCapStyle(Qt::FlatCap);
	_shape = s.createStroke(path).simplified();
	_boundingRect = _shape.boundingRect();
}

void TextPathItem::paint(QPainter *painter) const
{
	QFontMetrics fm(_font);
	int textWidth = fm.width(_text);

	qreal factor = (textWidth) / qMax(_path.length(), (qreal)textWidth);
	qreal percent = (1.0 - factor) / 2.0;

	painter->setFont(_font);
	painter->setPen(_pen);

	for (int i = 0; i < _text.size(); i++) {
		QPointF point = _path.pointAtPercent(percent);
		qreal angle = _path.angleAtPercent(percent);

		painter->translate(point);
		painter->rotate(-angle);
		painter->drawText(QPoint(0, fm.descent()), _text.at(i));
		painter->resetTransform();

		int width = fm.charWidth(_text, i);
		percent += ((qreal)width / (qreal)textWidth) * factor;
	}

	//painter->setPen(Qt::red);
	//painter->drawPath(_shape);
}
