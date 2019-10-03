#include <QPainter>
#include <QtMath>
#include "config.h"
#include "textpointitem.h"


#define FLAGS (Qt::AlignCenter | Qt::TextWordWrap | Qt::TextDontClip)

static QImage textImage(const QString &text, const QRectF &rect, qreal scale,
  const QColor &color, const QFont &font)
{
	QImage img(QSize(rect.size().width() * scale, rect.size().height() * scale),
	  QImage::Format_ARGB32_Premultiplied);

	img.fill(Qt::transparent);

	QFont f(font);
	f.setPixelSize(font.pixelSize() * scale);

	QPainter ip(&img);
	ip.setPen(color);
	ip.setFont(f);
	ip.drawText(img.rect(), FLAGS, text);

	return img;
}

QRectF TextPointItem::exactBoundingRect() const
{
	QFontMetrics fm(font());

	QRect br = fm.boundingRect(_textRect.toRect(), FLAGS, text());
	Q_ASSERT(br.isValid());

	// Italic fonts overflow the computed bounding rect, so expand it
	if (font().italic())
		br.adjust(-font().pixelSize() / 2.0, 0, font().pixelSize() / 2.0, 0);
	if (hasHalo())
		br.adjust(-1, -1, 1, 1);

	return br;
}

QRectF TextPointItem::fuzzyBoundingRect() const
{
	int limit = font().pixelSize() * _maxWidth;
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

	return QRectF(0, 0, width, lines > 1 ? lines * font().pixelSize() * 1.3
	  : font().pixelSize() * 1.5);
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
	QRectF textRect = exact ? exactBoundingRect() : _textRect;

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
	_textRect = fuzzyBoundingRect();
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
	QRectF textRect = (!_icon.isNull() || hasHalo())
	  ? computeTextRect(true) : _boundingRect;

	if (!_icon.isNull()) {
#ifdef ENABLE_HIDPI
		painter->drawImage(_pos - QPointF(_icon.width()
		  / _icon.devicePixelRatioF() / 2, _icon.height()
		  / _icon.devicePixelRatioF() / 2), _icon);
#else // ENABLE_HIDPI
		painter->drawImage(_pos - QPointF(_icon.width() / 2,
		  _icon.height() / 2), _icon);
#endif // ENABLE_HIDPI
	}

	if (hasHalo()) {
		const QTransform &t = painter->worldTransform();
		QPoint p((textRect.topLeft() * t.m11()).toPoint());
		QImage img(textImage(text(), textRect, t.m11(), halo().color(), font()));

		painter->save();
		painter->resetTransform();

		painter->drawImage(p.x() - 1, p.y() - 1, img);
		painter->drawImage(p.x() + 1, p.y() + 1, img);
		painter->drawImage(p.x() - 1, p.y() + 1, img);
		painter->drawImage(p.x() + 1, p.y() - 1, img);
		painter->drawImage(p.x(), p.y() - 1, img);
		painter->drawImage(p.x(), p.y() + 1, img);
		painter->drawImage(p.x() - 1, p.y(), img);
		painter->drawImage(p.x() + 1, p.y(), img);

		painter->drawImage(p.x(), p.y(), textImage(text(), textRect, t.m11(),
		  pen().color(), font()));

		painter->restore();
	} else {
		painter->setFont(font());
		painter->setPen(pen());
		painter->drawText(textRect, FLAGS, text());
	}

	//painter->setBrush(Qt::NoBrush);
	//painter->setPen(Qt::red);
	//painter->drawRect(_boundingRect);
	//painter->setPen(Qt::blue);
	//painter->drawRect(textRect);
}
