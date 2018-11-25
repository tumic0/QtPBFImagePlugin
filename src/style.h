#ifndef STYLE_H
#define STYLE_H

#include <QObject>
#include <QString>
#include <QVariantHash>
#include <QStringList>
#include <QSet>
#include <QPen>
#include <QBrush>
#include <QFont>
#include "text.h"
#include "function.h"
#include "sprites.h"


class QPainter;
class QPainterPath;
class Tile;

class Style : public QObject
{
public:
	Style(QObject *parent = 0) : QObject(parent) {}

	bool load(const QString &fileName);

	const QStringList &sourceLayers() const {return _sourceLayers;}

	bool match(int zoom, int layer, const QVariantHash &tags) const;

	void drawBackground(Tile &tile) const;
	void setPainter(Tile &tile, int layer) const;
	void setTextProperties(Tile &tile, int layer) const;
	void drawFeature(Tile &tile, int layer, const QPainterPath &path,
	  const QVariantHash &tags) const;

private:
	class Layer {
	public:
		Layer() : _type(Unknown), _minZoom(-1), _maxZoom(-1) {}
		Layer(const QJsonObject &json);

		const QString &sourceLayer() const {return _sourceLayer;}
		bool isPath() const {return (_type == Line || _type == Fill);}
		bool isBackground() const {return (_type == Background);}
		bool isSymbol() const {return (_type == Symbol);}

		bool match(int zoom, const QVariantHash &tags) const;
		void setPathPainter(Tile &tile, const Sprites &sprites) const;
		void setSymbolPainter(Tile &tile) const;
		void setTextProperties(Tile &tile) const;
		void addSymbol(Tile &tile, const QPainterPath &path,
		  const QVariantHash &tags, const Sprites &sprites) const;

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

			bool match(const QVariantHash &tags) const;
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

		class Template {
		public:
			Template() {}
			Template(const FunctionS &str) : _field(str) {}
			QString value(int zoom, const QVariantHash &tags) const;

		private:
			FunctionS _field;
		};

		class Layout {
		public:
			Layout() : _textSize(16), _textMaxWidth(10), _textMaxAngle(45),
			  _font("Open Sans"), _viewportAlignment(false) {}
			Layout(const QJsonObject &json);

			qreal maxTextWidth(int zoom) const
			  {return _textMaxWidth.value(zoom);}
			qreal maxTextAngle(int zoom) const
			  {return _textMaxAngle.value(zoom);}
			QString text(int zoom, const QVariantHash &tags) const
			  {return _text.value(zoom, tags);}
			QString icon(int zoom, const QVariantHash &tags) const
			  {return _icon.value(zoom, tags);}
			QFont font(int zoom) const;
			Qt::PenCapStyle lineCap(int zoom) const;
			Qt::PenJoinStyle lineJoin(int zoom) const;
			Text::Anchor textAnchor(int zoom) const;
			bool viewportAlignment() const {return _viewportAlignment;}

		private:
			QFont::Capitalization textTransform(int zoom) const;

			Template _text;
			Template _icon;
			FunctionF _textSize;
			FunctionF _textMaxWidth;
			FunctionF _textMaxAngle;
			FunctionS _lineCap;
			FunctionS _lineJoin;
			FunctionS _textAnchor;
			FunctionS _textTransform;
			QFont _font;
			bool _viewportAlignment;
		};

		class Paint {
		public:
			Paint() : _fillOpacity(1.0), _lineOpacity(1.0), _lineWidth(1.0) {}
			Paint(const QJsonObject &json);

			QPen pen(Layer::Type type, int zoom) const;
			QBrush brush(Layer::Type type, int zoom, const Sprites &sprites) const;
			qreal opacity(Layer::Type type, int zoom) const;
			bool antialias(Layer::Type type, int zoom) const;
			QString fillPattern(int zoom) const
			  {return _fillPattern.value(zoom);}

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
			FunctionS _fillPattern;
		};

		Type _type;
		QString _sourceLayer;
		int _minZoom, _maxZoom;
		Filter _filter;
		Layout _layout;
		Paint _paint;
	};

	QVector<Layer> _layers;
	QStringList _sourceLayers;
	Sprites _sprites;
};

#endif // STYLE_H
