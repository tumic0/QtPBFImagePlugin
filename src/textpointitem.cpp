#include <QPainter>
#include <QtMath>
#include "config.h"
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)

QRectF TextPointItem::exactBoundingRect() const
{
	QFontMetrics fm(font());
	int limit = font().pixelSize() * _maxWidth;
	// Italic fonts overflow the computed bounding rect, so reduce it
	// a little bit.
	if (font().italic())
		limit -= font().pixelSize() / 2.0;

	QRect br = fm.boundingRect(QRect(0, 0, limit, 0), FLAGS, text());
	Q_ASSERT(br.isValid());
	// Expand the bounding rect back to the real content size
	if (font().italic())
		br.adjust(-font().pixelSize() / 4.0, 0, font().pixelSize() / 4.0, 0);

	return br;
}

QRectF TextPointItem::fuzzyBoundingRect() const
{
	int limit = font().pixelSize() * _maxWidth;
	qreal cw = avgCharWidth();
	qreal lh = font().pixelSize() * 1.25;
	int width = 0, lines = 0;

	QStringList l(text().split('\n'));
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


QRectF TextPointItem::computeTextRect(bool exact) const
{
#ifdef ENABLE_HIDPI
	QRectF iconRect = _icon.isNull() ? QRectF()
	  : QRectF(QPointF(0, 0), QSizeF(_icon.size()) / _icon.devicePixelRatioF());
#else // ENABLE_HIDPI
	QRectF iconRect = _icon.isNull() ? QRectF() : QRectF(QPointF(0, 0),
	  QSizeF(_icon.size()));
#endif // ENABLE_HIDPI
	QRectF textRect = exact ? exactBoundingRect() : fuzzyBoundingRect();

	switch (_anchor) {
		case Text::Center:
			textRect.moveCenter(_pos);
			break;
		case Text::Left:
			textRect.moveTopLeft(_pos - QPointF(-iconRect.width() / 2,
			  textRect.height() / 2));
			break;
		case Text::Right:
			textRect.moveTopRight(_pos - QPointF(iconRect.width() / 2,
			  textRect.height() / 2));
			break;
		case Text::Bottom:
			textRect.moveTopLeft(_pos - QPointF(textRect.width() / 2,
			  iconRect.height() / 2));
			break;
		case Text::Top:
			textRect.moveTopLeft(_pos - QPointF(textRect.width() / 2,
			  -iconRect.height() / 2));
			break;
	}

	return textRect;
}

TextPointItem::TextPointItem(const QString &text, const QPointF &pos,
  const QFont &font, int maxWidth, Text::Anchor anchor, const QImage &icon)
  : TextItem(text, font), _pos(pos), _icon(icon), _maxWidth(maxWidth),
  _anchor(anchor)
{
	_boundingRect = computeTextRect(false);

	if (!_icon.isNull()) {
#ifdef ENABLE_HIDPI
		QRectF iconRect(QPointF(0, 0), QSizeF(_icon.size())
		  / _icon.devicePixelRatioF());
#else // ENABLE_HIDPI
		QRectF iconRect(QPointF(0, 0), QSizeF(_icon.size()));
#endif // ENABLE_HIDPI
		iconRect.moveCenter(pos);
		_boundingRect |= iconRect;
	}

	_shape.addRect(_boundingRect);
}

void TextPointItem::paint(QPainter *painter) const
{
	//painter->setPen(Qt::red);
	//painter->drawRect(_boundingRect);

	QRectF textRect;
	bool hasHalo = halo().color().isValid() && halo().width() > 0;

	if (!_icon.isNull()) {
		textRect = (_anchor != Text::Center || hasHalo)
		  ? computeTextRect(true) : _boundingRect;
#ifdef ENABLE_HIDPI
		painter->drawImage(_pos - QPointF(_icon.width()
		  / _icon.devicePixelRatioF() / 2, _icon.height()
		  / _icon.devicePixelRatioF() / 2), _icon);
#else // ENABLE_HIDPI
		painter->drawImage(_pos - QPointF(_icon.width() / 2,
		  _icon.height() / 2), _icon);
#endif // ENABLE_HIDPI
	} else
		textRect = hasHalo ? computeTextRect(true) : _boundingRect;


	if (hasHalo) {
		QRect ir(textRect.toRect());
		QImage img(ir.size(), QImage::Format_ARGB32_Premultiplied);
		img.fill(Qt::transparent);
		QPainter ip(&img);
		ip.setPen(halo().color());
		ip.setFont(font());
		ip.drawText(img.rect(), FLAGS, text());

		painter->drawImage(ir.x() - 1, ir.y() - 1, img);
		painter->drawImage(ir.x() + 1, ir.y() + 1, img);
		painter->drawImage(ir.x() - 1, ir.y() + 1, img);
		painter->drawImage(ir.x() + 1, ir.y() - 1, img);
		painter->drawImage(ir.x(), ir.y() - 1, img);
		painter->drawImage(ir.x(), ir.y() + 1, img);
		painter->drawImage(ir.x() - 1, ir.y(), img);
		painter->drawImage(ir.x() + 1, ir.y(), img);

		painter->setFont(font());
		painter->setPen(pen());
		painter->drawText(ir, FLAGS, text());
	} else {
		painter->setFont(font());
		painter->setPen(pen());
		painter->drawText(textRect, FLAGS, text());
	}
}
