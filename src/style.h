#ifndef STYLE_H
#define STYLE_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QStringList>
#include <QSet>
#include <QPen>
#include <QBrush>
#include <QFont>
#include "function.h"


class QPainter;
class QPainterPath;
class Tile;

class Style : public QObject
{
public:
	Style(QObject *parent = 0) : QObject(parent) {}

	bool load(const QString &fileName);

	void setZoom(int zoom) {_zoom = zoom;}

	const QStringList &sourceLayers() const {return _sourceLayers;}
	bool match(int layer, const QVariantMap &tags);

	void drawBackground(Tile &tile);
	void drawFeature(int layer, const QPainterPath &path,
	  const QVariantMap &tags, Tile &tile);

private:
	class Layer {
	public:
		Layer(const QJsonObject &json);

		const QString &sourceLayer() const {return _sourceLayer;}
		bool isPath() const {return (_type == Line || _type == Fill);}
		bool isBackground() const {return (_type == Background);}
		bool isSymbol() const {return (_type == Symbol);}

		bool match(int zoom, const QVariantMap &tags) const;
		void drawPath(int zoom, const QPainterPath &path, Tile &tile) const;
		void drawSymbol(int zoom, const QPainterPath &path,
		  const QVariantMap &tags, Tile &tile) const;

	private:
		enum Type {
			Unknown,
			Fill,
			Line,
			Background,
			Symbol
		};

		class Filter {
		public:
			Filter() : _type(None) {}
			Filter(const QJsonArray &json);

			bool match(const QVariantMap &tags) const;
		private:
			enum Type {
				None, Unknown,
				EQ, NE, GE, GT, LE, LT,
				All, Any,
				In, Has
			};

			Type _type;
			bool _not;
			QSet<QString> _set;
			QPair<QString, QVariant> _kv;
			QVector<Filter> _filters;
		};

		class Layout {
		public:
			Layout() : _textSize(16), _textMaxWidth(10), _textMaxAngle(45),
			  _lineCap(Qt::FlatCap), _lineJoin(Qt::MiterJoin),
			  _capitalize(false) {}
			Layout(const QJsonObject &json);

			bool capitalize() const {return _capitalize;}
			qreal maxTextWidth(int zoom) const
			  {return _textMaxWidth.value(zoom);}
			qreal maxTextAngle(int zoom) const
			  {return _textMaxAngle.value(zoom);}
			const QString &field() const {return _textField;}
			const QStringList &keys() const {return _keys;}
			QFont font(int zoom) const;
			Qt::PenCapStyle lineCap() const {return _lineCap;}
			Qt::PenJoinStyle lineJoin() const {return _lineJoin;}

		private:
			QStringList _keys;
			QString _textField;
			FunctionF _textSize;
			FunctionF _textMaxWidth;
			FunctionF _textMaxAngle;
			Qt::PenCapStyle _lineCap;
			Qt::PenJoinStyle _lineJoin;
			QFont _font;
			bool _capitalize;
		};

		class Paint {
		public:
			Paint() : _fillOpacity(1.0), _lineOpacity(1.0), _lineWidth(1.0) {}
			Paint(const QJsonObject &json);

			QPen pen(Layer::Type type, int zoom) const;
			QBrush brush(Layer::Type type, int zoom) const;
			qreal opacity(Layer::Type type, int zoom) const;
			bool antialias(Layer::Type type, int zoom) const;

		private:
			FunctionC _textColor;
			FunctionC _lineColor;
			FunctionC _fillColor;
			FunctionC _fillOutlineColor;
			FunctionC _backgroundColor;
			FunctionF _fillOpacity;
			FunctionF _lineOpacity;
			FunctionF _lineWidth;
			FunctionB _fillAntialias;
			QVector<qreal> _lineDasharray;
		};

		Type _type;
		QString _sourceLayer;
		int _minZoom, _maxZoom;
		Filter _filter;
		Layout _layout;
		Paint _paint;
	};

	int _zoom;
	QList<Layer> _styles;
	QStringList _sourceLayers;
};

#endif // STYLE_H
