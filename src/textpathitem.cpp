#include <QFontMetrics>
#include <QPainter>
#include "textpathitem.h"


TextPathItem::TextPathItem(const QString &text, const QPainterPath &path,
  const QFont &font, QGraphicsItem *parent) : QGraphicsItem(parent),
  _text(text), _path(path), _font(font)
{
	QFontMetrics fm(font);
	QPainterPathStroker s;
	s.setWidth(fm.height());
	_shape = s.createStroke(path).simplified();
}

void TextPathItem::paint(QPainter *painter,
  const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	QFontMetrics fm(_font);
	int textWidth = fm.width(_text);

	qreal factor = (textWidth) / _path.length();
	qreal percent = (1.0 - factor) / 2.0;

	painter->setFont(_font);
	painter->setPen(_pen);

	for (int i = 0; i < _text.size(); i++) {
		Q_ASSERT(percent <= 1.0);

		QPointF point = _path.pointAtPercent(percent);
		qreal angle = _path.angleAtPercent(percent);

		painter->translate(point);
		painter->rotate(-angle);
		painter->drawText(QPoint(0, fm.descent()), _text.at(i));
		painter->resetTransform();

		int width = fm.charWidth(_text, i);
		percent += ((qreal)width / (qreal)textWidth) * factor;
	}
}
