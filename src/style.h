#ifndef STYLE_H
#define STYLE_H

#include <QObject>
#include <QString>
#include <QVector>
#include <QPair>
#include <QVariantHash>
#include <QStringList>
#include <QSet>
#include <QPen>
#include <QBrush>
#include <QFont>
#include "pbf.h"
#include "text.h"
#include "function.h"
#include "sprites.h"


class QPainter;
class QPainterPath;
class QJsonArray;
class QJsonObject;
class Tile;

class Style : public QObject
{
public:
	Style(QObject *parent = 0) : QObject(parent) {}

	bool load(const QString &fileName);
	void render(const PBF &data, Tile &tile);

private:
	class Layer {
	public:
		Layer() : _type(Unknown), _minZoom(0), _maxZoom(24) {}
		Layer(const QJsonObject &json);

		const QByteArray &sourceLayer() const {return _sourceLayer;}
		bool isPath() const {return (_type == Line || _type == Fill);}
		bool isBackground() const {return (_type == Background);}
		bool isSymbol() const {return (_type == Symbol);}
		bool isVisible() const {return (_layout.visible());}

		bool match(int zoom, const PBF::Feature &feature) const;
		void setPathPainter(Tile &tile, Sprites &sprites) const;
		void setTextProperties(Tile &tile) const;
		void addSymbol(Tile &tile, const QPainterPath &path,
		  const PBF::Feature &feature, Sprites &sprites) const;

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

			bool match(const PBF::Feature &feature) const;
		private:
			enum Type {
				None, Unknown,
				EQ, NE, GE, GT, LE, LT,
				All, Any,
				In, Has, GeometryType
			};

			Type _type;
			bool _not;
			QSet<QByteArray> _set;
			QPair<QByteArray, QVariant> _kv;
			QVector<Filter> _filters;
		};

		class Template {
		public:
			Template() {}
			Template(const FunctionS &str) : _field(str) {}
			QString value(int zoom, const PBF::Feature &feature) const;

		private:
			FunctionS _field;
		};

		class Layout {
		public:
			Layout() : _textSize(16), _textMaxWidth(10), _textMaxAngle(45),
			  _font("Open Sans"), _visible(true) {}
			Layout(const QJsonObject &json);

			qreal maxTextWidth(int zoom) const
			  {return _textMaxWidth.value(zoom);}
			qreal maxTextAngle(int zoom) const
			  {return _textMaxAngle.value(zoom);}
			QString text(int zoom, const PBF::Feature &feature) const
			  {return _text.value(zoom, feature).trimmed();}
			QString icon(int zoom, const PBF::Feature &feature) const
			  {return _icon.value(zoom, feature);}
			qreal iconSize(int zoom) const
			  {return _iconSize.value(zoom);}
			QFont font(int zoom) const;
			Qt::PenCapStyle lineCap(int zoom) const;
			Qt::PenJoinStyle lineJoin(int zoom) const;
			Text::Anchor textAnchor(int zoom) const;
			Text::SymbolPlacement symbolPlacement(int zoom) const;
			Text::RotationAlignment textRotationAlignment(int zoom) const;
			bool visible() const {return _visible;}

		private:
			QFont::Capitalization textTransform(int zoom) const;

			Template _text;
			Template _icon;
			FunctionF _iconSize;
			FunctionF _textSize;
			FunctionF _textMaxWidth;
			FunctionF _textMaxAngle;
			FunctionS _lineCap;
			FunctionS _lineJoin;
			FunctionS _textAnchor;
			FunctionS _textTransform;
			FunctionS _symbolPlacement;
			FunctionS _textRotationAlignment;
			QFont _font;
			bool _visible;
		};

		class Paint {
		public:
			Paint() : _fillOpacity(1.0), _lineOpacity(1.0), _lineWidth(1.0) {}
			Paint(const QJsonObject &json);

			QPen pen(Layer::Type type, int zoom) const;
			QBrush brush(Layer::Type type, int zoom, Sprites &sprites)
			  const;
			qreal opacity(Layer::Type type, int zoom) const;
			bool antialias(Layer::Type type, int zoom) const;
			Text::Halo halo(int zoom) const
			  {return Text::Halo(_textHaloColor.value(zoom),
			  _textHaloWidth.value(zoom), _textHaloBlur.value(zoom));}
			QColor iconColor(int zoom) const
			  {return _iconColor.value(zoom);}

		private:
			FunctionC _textColor;
			FunctionC _textHaloColor;
			FunctionC _lineColor;
			FunctionC _fillColor;
			FunctionC _fillOutlineColor;
			FunctionC _backgroundColor;
			FunctionC _iconColor;
			FunctionF _fillOpacity;
			FunctionF _lineOpacity;
			FunctionF _lineWidth;
			FunctionF _textHaloWidth;
			FunctionF _textHaloBlur;
			FunctionB _fillAntialias;
			QVector<qreal> _lineDasharray;
			FunctionS _fillPattern;
		};

		Type _type;
		QByteArray _sourceLayer;
		int _minZoom, _maxZoom;
		Filter _filter;
		Layout _layout;
		Paint _paint;
	};

	Sprites &sprites(const QPointF &scale);

	void drawBackground(Tile &tile);
	void setupLayer(Tile &tile, const Layer &layer);
	void drawFeature(const PBF::Feature &feature, const Layer &layer,
	  Tile &tile, const QSizeF &factor);
	void drawLayer(const PBF::Layer &pbfLayer, const Layer &styleLayer,
	  Tile &tile);

	QVector<Layer> _layers;
	Sprites _sprites, _sprites2x;
};

#endif // STYLE_H
