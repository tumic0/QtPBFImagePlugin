#include <QPainter>
#include <QtMath>
#include <QStaticText>
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)

QRectF TextPointItem::fuzzyBoundingRect() const
{
	int fs = font().pixelSize();
	if (text().size() <= 3)
		return QRectF(0, 0, text().size() * fs, fs * 1.6);

	int limit = fs * _maxWidth;
	qreal cw = avgCharWidth();
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
					if (wl >= limit) {
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

	return QRectF(0, 0, width, lines * fs * 1.6);
}

QRectF TextPointItem::moveTextRect(const QRectF &rect) const
{
	QRectF iconRect = _icon.isNull() ? QRectF()
	  : QRectF(QPointF(0, 0), QSizeF(_icon.size()) / _icon.devicePixelRatioF());
	QRectF textRect(rect);

	switch (_anchor) {
		case Text::Center:
			textRect.moveCenter(_pos);
			break;
		case Text::Left:
			textRect.moveTopLeft(_pos - QPointF(-iconRect.width() / 2
			  - font().pixelSize()/4.0, textRect.height() / 2));
			break;
		case Text::Right:
			textRect.moveTopRight(_pos - QPointF(iconRect.width() / 2
			  + font().pixelSize()/4.0, textRect.height() / 2));
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
	_textRect = text.isEmpty() ? QRectF() : fuzzyBoundingRect();
	_boundingRect = moveTextRect(_textRect);

	if (!_icon.isNull()) {
		QRectF iconRect(QPointF(0, 0), QSizeF(_icon.size())
		  / _icon.devicePixelRatioF());
		iconRect.moveCenter(pos);
		_boundingRect |= iconRect;
	}

	_shape.addRect(_boundingRect);
}

void TextPointItem::setPos(const QPointF &pos)
{
	QPointF d(_boundingRect.left() - _pos.x(), _boundingRect.top() - _pos.y());
	_boundingRect.moveTopLeft(pos + d);
	_shape = QPainterPath();
	_shape.addRect(_boundingRect);
	_pos = pos;
}

void TextPointItem::paint(QPainter *painter) const
{
	QRectF textRect;

	painter->setFont(font());
	painter->setPen(pen());

	if (!_icon.isNull()) {
		textRect = moveTextRect(painter->boundingRect(_textRect, FLAGS, text()));
		painter->drawImage(_pos - QPointF(_icon.width()
		  / _icon.devicePixelRatioF() / 2, _icon.height()
		  / _icon.devicePixelRatioF() / 2), _icon);
	} else
		textRect = _boundingRect;

	if (hasHalo()) {
		QStaticText st(text());
		st.setTextFormat(Qt::PlainText);
		st.setTextWidth(textRect.width());
		st.setTextOption(QTextOption(Qt::AlignHCenter));
		st.setPerformanceHint(QStaticText::AggressiveCaching);

		painter->setPen(halo().color());
		painter->drawStaticText(textRect.topLeft() + QPointF(-1, -1), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(+1, +1), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(-1, +1), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(+1, -1), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(0, -1), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(0, +1), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(-1, 0), st);
		painter->drawStaticText(textRect.topLeft() + QPointF(+1, 0), st);
		painter->setPen(pen());
		painter->drawStaticText(textRect.topLeft(), st);
	} else
		painter->drawText(textRect, FLAGS, text());

	//painter->setBrush(Qt::NoBrush);
	//painter->setPen(Qt::red);
	//painter->drawRect(_boundingRect);
	//painter->setPen(Qt::blue);
	//painter->drawRect(textRect);
}
