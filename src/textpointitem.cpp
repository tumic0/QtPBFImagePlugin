﻿#include <QPainter>
#include <QtMath>
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)

static QRectF fuzzyBoundingRect(const QString &str, const QFont &font,
  int maxTextWidth)
{
	int limit = font.pixelSize() * maxTextWidth;
	qreal cw = font.pixelSize() * 0.6;
	qreal lh = font.pixelSize() * 1.25;
	int width = 0, lines = 0;

	QStringList l(str.split('\n'));
	for (int i = 0; i < l.size(); i++) {
		int lw = (int)(l.at(i).length() * cw);
		if (lw > limit) {
			l[i].replace('-', ' ');
			l[i].replace('/', ' ');
			QStringList words(l.at(i).split(' '));
			int pl = 0;
			for (int j = 0; j < words.size(); j++) {
				int wl = (int)(words.at(j).length() * cw);
				if (wl + pl < limit) {
					pl += wl + cw;
				} else {
					if (wl > limit) {
						if (pl > 0)
							lines++;
					} else
						lines++;
					width = qMax(width, qMax(pl, wl));
					pl = wl;
				}
			}
			width = qMax(width, pl);
			lines++;
		} else {
			width = qMax(width, lw);
			lines++;
		}
	}

	return QRectF(0, 0, width, lines * lh);
}

/*
static QRectF exactBoundingRect(const QString &str, const QFont &font,
  int maxTextWidth)
{
	QFontMetrics fm(font);
	int limit = font.pixelSize() * maxTextWidth;
	// Italic fonts overflow the computed bounding rect, so reduce it
	// a little bit.
	if (font.italic())
		limit -= font.pixelSize() / 2.0;

	QRect br = fm.boundingRect(QRect(0, 0, limit, 0), FLAGS, str);
	Q_ASSERT(br.isValid());
	// Expand the bounding rect back to the real content size
	if (font.italic())
		br.adjust(-font.pixelSize() / 4.0, 0, font.pixelSize() / 4.0, 0);

	return br;
}
*/

TextPointItem::TextPointItem(const QString &text, const QPointF &pos,
  const QFont &font, int maxTextWidth) :_text(text), _font(font)
{
	_boundingRect = fuzzyBoundingRect(text, font, maxTextWidth);
	//_boundingRect = exactBoundingRect(text, font, maxTextWidth);

	_boundingRect.moveCenter(pos);
	_shape.addRect(_boundingRect);
}

void TextPointItem::paint(QPainter *painter) const
{
	painter->setFont(_font);
	painter->setPen(_pen);
	painter->drawText(_boundingRect, FLAGS, _text);

	//painter->setPen(Qt::red);
	//painter->drawRect(_boundingRect);
}