#ifndef TEXT_H
#define TEXT_H

#include <QGraphicsScene>

class Text : public QGraphicsScene
{
public:
	Text(QObject *parent = 0) : QGraphicsScene(parent)
	  {setSceneRect(0, 0, 256, 256);}

	void addLabel(const QString &text, const QPointF &pos, const QFont &font,
	  const QPen &pen, qreal maxTextWidth);
	void addLabel(const QString &text, const QPainterPath &path,
	  const QFont &font, const QPen &pen);
};

#endif // TEXT_H
