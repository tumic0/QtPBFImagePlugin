#include <QFontMetrics>
#include <QPainter>
#include <QSet>
#include <QMap>
#include "text.h"
#include "textpointitem.h"
#include "textpathitem.h"


Text::~Text()
{
	qDeleteAll(_items);
}

void Text::render(QPainter *painter) const
{
	QSet<QString> set;

	for (int i = 0; i < _items.size(); i++) {
		const TextItem *ti = _items.at(i);
		if (ti->isVisible() && _sceneRect.intersects(ti->boundingRect())
		  && !set.contains(ti->text())) {
			ti->paint(painter);
			set.insert(ti->text());
		}
	}
}

void Text::addLabel(const QString &text, const QImage &icon,
  const QPainterPath &path)
{
	TextItem *ti;

	if (_alignment == Viewport) {
		QMap<qreal, int> map;
		for (int j = 0; j < path.elementCount(); j++) {
			QLineF l(path.elementAt(j), _sceneRect.center());
			map.insert(l.length(), j);
		}
		QMap<qreal, int>::const_iterator jt = map.constBegin();
		ti = new TextPointItem(text, path.elementAt(jt.value()), _font,
		  _maxWidth, _anchor, icon);
		while (true) {
			if (_sceneRect.contains(ti->boundingRect()))
				break;
			if (++jt == map.constEnd())
				break;
			static_cast<TextPointItem*>(ti)->setPos(path.elementAt(
			  jt.value()));
		}
	} else {
		switch (_placement) {
			case Line:
				ti = new TextPathItem(text, path, _font, _maxAngle, _sceneRect);
				break;
			case LineCenter:
				ti = new TextPointItem(text, path.pointAtPercent(0.5), _font,
				  _maxWidth, _anchor, icon);
				break;
			default:
				ti = new TextPointItem(text, path.elementAt(0), _font,
				  _maxWidth, _anchor, icon);
		}
	}

	// Note: empty path == point geometry (single move)
	if (!path.isEmpty() && !_sceneRect.contains(ti->boundingRect())) {
		delete ti;
		return;
	}

	ti->setPen(_pen);
	ti->setHalo(_halo);
	addItem(ti);

	QList<TextItem*> ci(collidingItems(ti));
	for (int i = 0; i < ci.size(); i++)
		ci[i]->setVisible(false);
}

QList<TextItem*> Text::collidingItems(const TextItem *item) const
{
	QList<TextItem*> list;

	if (!item->isVisible())
		return list;

	for (int i = 0; i < _items.size(); i++) {
		const TextItem *ti = _items.at(i);
		if (ti != item && ti->isVisible() && ti->collidesWithItem(item))
			list.append(const_cast<TextItem*>(ti));
	}

	return list;
}

void Text::setSymbolPlacement(SymbolPlacement placement)
{
	_placement = placement;

	if (_placement != Text::Point) {
		for (int i = 0; i < _items.size(); i++) {
			TextItem *ti = _items[i];
			if (!_sceneRect.contains(ti->boundingRect()))
				ti->setVisible(false);
		}
	}
}
