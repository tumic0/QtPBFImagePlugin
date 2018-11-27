#include <QFontMetrics>
#include <QPainter>
#include "text.h"
#include "textpointitem.h"
#include "textpathitem.h"


Text::~Text()
{
	for (int i = 0; i < _items.size(); i++)
		delete _items[i];
}

void Text::render(QPainter *painter) const
{
	for (int i = 0; i < _items.size(); i++) {
		const TextItem *ti = _items.at(i);
		if (ti->isVisible() && _sceneRect.intersects(ti->boundingRect()))
			ti->paint(painter);
	}
}

void Text::addLabel(const QString &text, const QPointF &pos, bool overlap,
  const QImage &icon)
{
	TextPointItem *ti = new TextPointItem(text, pos, _font, _maxWidth, _anchor,
	  icon);
	if (!overlap && !_sceneRect.contains(ti->boundingRect())) {
		delete ti;
		return;
	}
	ti->setPen(_pen);
	addItem(ti);

	QList<TextItem*> ci = collidingItems(ti);
	for (int i = 0; i < ci.size(); i++)
		ci[i]->setVisible(false);
}

void Text::addLabel(const QString &text, const QPainterPath &path)
{
	TextPathItem *ti = new TextPathItem(text, path, _font, _maxAngle,
	  _sceneRect);
	if (!_sceneRect.contains(ti->boundingRect())) {
		delete ti;
		return;
	}
	ti->setPen(_pen);

	addItem(ti);

	QList<TextItem*> ci = collidingItems(ti);
	for (int i = 0; i < ci.size(); i++)
		ci[i]->setVisible(false);
}

QList<TextItem*> Text::collidingItems(const TextItem *item) const
{
	QList<TextItem*> list;

	if (!item->isVisible())
		return list;

	for (int i = 0; i < _items.size();i ++) {
		const TextItem *ti = _items.at(i);
		if (ti != item && ti->isVisible() && ti->collidesWithItem(item))
			list.append(const_cast<TextItem*>(ti));
	}

	return list;
}
