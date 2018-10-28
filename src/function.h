#ifndef FUNCTION_H
#define FUNCTION_H

#include <QList>
#include <QPointF>
#include <QColor>

class QJsonObject;

class FunctionF {
public:
	FunctionF(qreal deflt = 0) : _default(deflt), _base(1.0) {}
	FunctionF(const QJsonObject &json, qreal dflt = 0);

	qreal value(qreal x) const;

private:
	QList<QPointF> _stops;
	qreal _default;
	qreal _base;
};

class FunctionC {
public:
	FunctionC(const QColor &deflt = QColor())
	  : _default(deflt), _base(1.0) {}
	FunctionC(const QJsonObject &json, const QColor &dflt = QColor());

	QColor value(qreal x) const;

private:
	QList<QPair<qreal, QColor> > _stops;
	QColor _default;
	qreal _base;
};

#endif // FUNCTION_H
