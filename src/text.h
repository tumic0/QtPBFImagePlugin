#ifndef TEXT_H
#define TEXT_H

#include <QGraphicsScene>

class Text : public QGraphicsScene
{
public:
	Text(int size, QObject *parent = 0) : QGraphicsScene(parent)
	  {setSceneRect(0, 0, size, size);}

	void addLabel(const QString &text, const QPointF &pos, const QFont &font,
	  const QPen &pen, qreal maxTextWidth);
	void addLabel(const QString &text, const QPainterPath &path,
	  const QFont &font, const QPen &pen, qreal maxAngle, qreal symbolSpacing);
};

#endif // TEXT_H
