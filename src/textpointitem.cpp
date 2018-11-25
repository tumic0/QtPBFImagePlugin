#include <QPainter>
#include <QtMath>
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)

static QRectF exactBoundingRect(const QString &str, const QFont &font,
  const Text::Properties &prop)
{
	QFontMetrics fm(font);
	int limit = font.pixelSize() * prop.maxWidth;
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

static QRectF fuzzyBoundingRect(const QString &str, const QFont &font,
  const Text::Properties &prop)
{
	int limit = font.pixelSize() * prop.maxWidth;
	qreal acw = (font.capitalization() == QFont::AllUppercase) ? 0.66 : 0.55;
	qreal cw = font.pixelSize() * acw;
	if (font.bold())
		acw *= 1.1;
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


QRectF TextPointItem::computeTextRect(BoundingRectFunction brf) const
{
	QRectF iconRect = _icon.isNull() ? QRectF() : _icon.rect();
	QRectF textRect = brf(text(), _font, _properties);

	switch (_properties.anchor) {
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
  const QFont &font, const Text::Properties &prop, const QImage &icon)
  : TextItem(text), _pos(pos), _font(font), _icon(icon), _properties(prop)
{
	_boundingRect = computeTextRect(fuzzyBoundingRect);

	if (!_icon.isNull()) {
		QRectF iconRect(_icon.rect());
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

	if (!_icon.isNull()) {
		textRect = computeTextRect(exactBoundingRect);
		painter->drawImage(_pos - QPointF(_icon.width() / 2,
		  _icon.height() / 2), _icon);
	} else
		textRect = computeTextRect(fuzzyBoundingRect);

	painter->setFont(_font);
	painter->setPen(_pen);
	painter->drawText(textRect, FLAGS, text());
}