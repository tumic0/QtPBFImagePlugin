#include <cmath>
#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "text.h"
#include "color.h"
#include "tile.h"
#include "style.h"


Style::Layer::Filter::Filter(const QJsonArray &json)
  : _type(Unknown), _not(false)
{
#define INVALID_FILTER(json) \
	{qWarning() << json << ": invalid filter"; return;}

	if (json.isEmpty())
		INVALID_FILTER(json);

	QString type = json.at(0).toString();

	if (type == "==") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = EQ;
		_kv = QPair<QString, QVariant>(json.at(1).toString(),
		  json.at(2).toVariant());
	} else if (type == "!=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = NE;
		_kv = QPair<QString, QVariant>(json.at(1).toString(),
		  json.at(2).toVariant());
	} else if (type == "<") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = LT;
		_kv = QPair<QString, QVariant>(json.at(1).toString(),
		  json.at(2).toVariant());
	} else if (type == "<=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = LE;
		_kv = QPair<QString, QVariant>(json.at(1).toString(),
		  json.at(2).toVariant());
	} else if (type == ">") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = GT;
		_kv = QPair<QString, QVariant>(json.at(1).toString(),
		  json.at(2).toVariant());
	} else if (type == ">=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = GE;
		_kv = QPair<QString, QVariant>(json.at(1).toString(),
		  json.at(2).toVariant());
	} else if (type == "all") {
		_type = All;
		for (int i = 1; i < json.size(); i++)
			_filters.append(Filter(json.at(i).toArray()));
	} else if (type == "any") {
		_type = Any;
		for (int i = 1; i < json.size(); i++)
			_filters.append(Filter(json.at(i).toArray()));
	} else if (type == "in") {
		if (json.size() < 3)
			INVALID_FILTER(json);
		_type = In;
		_kv = QPair<QString, QVariant>(json.at(1).toString(), QVariant());
		for (int i = 2; i < json.size(); i++)
			_set.insert(json.at(i).toString());
	}  else if (type == "!in") {
		if (json.size() < 3)
			INVALID_FILTER(json);
		_type = In;
		_not = true;
		_kv = QPair<QString, QVariant>(json.at(1).toString(), QVariant());
		for (int i = 2; i < json.size(); i++)
			_set.insert(json.at(i).toString());
	} else if (type == "has") {
		if (json.size() < 2)
			INVALID_FILTER(json);
		_type = Has;
		_kv = QPair<QString, QVariant>(json.at(1).toString(), QVariant());
	} else if (type == "!has") {
		if (json.size() < 2)
			INVALID_FILTER(json);
		_type = Has;
		_not = true;
		_kv = QPair<QString, QVariant>(json.at(1).toString(), QVariant());
	} else
		INVALID_FILTER(json);
}

bool Style::Layer::Filter::match(const QVariantMap &tags) const
{
	switch (_type) {
		case None:
			return true;
		case EQ:
			return tags.value(_kv.first) == _kv.second;
		case NE:
			return tags.value(_kv.first) != _kv.second;
		case GT:
			return tags.value(_kv.first) > _kv.second;
		case GE:
			return tags.value(_kv.first) >= _kv.second;
		case LT:
			return tags.value(_kv.first) < _kv.second;
		case LE:
			return tags.value(_kv.first) <= _kv.second;
		case In:
			return _set.contains(tags.value(_kv.first).toString()) ^ _not;
		case Has:
			return tags.contains(_kv.first) ^ _not;
		case All:
			for (int i = 0; i < _filters.size(); i++) {
				if (!_filters.at(i).match(tags))
					return false;
			}
			return true;
		case Any:
			for (int i = 0; i < _filters.size(); i++) {
				if (_filters.at(i).match(tags))
					return true;
			}
			return false;
		default:
			return false;
	}
}

Style::Layer::Paint::Paint(const QJsonObject &json)
  : _fillOpacity(1.0), _lineOpacity(1.0), _lineWidth(1.0), _fillAntialias(true)
{
	if (json.contains("fill-opacity") && json["fill-opacity"].isDouble())
		_fillOpacity = FunctionF(json["fill-opacity"].toDouble());
	else if (json.contains("fill-opacity") && json["fill-opacity"].isObject())
		_fillOpacity = FunctionF(json["fill-opacity"].toObject());
	if (json.contains("fill-color") && json["fill-color"].isString())
		_fillColor = FunctionC(Color::fromJsonString(
		  json["fill-color"].toString()));
	if (json.contains("fill-color") && json["fill-color"].isObject())
		_fillColor = FunctionC(json["fill-color"].toObject());
	if (json.contains("fill-outline-color")
	  && json["fill-outline-color"].isString())
		_fillOutlineColor = FunctionC(Color::fromJsonString(
		  json["fill-outline-color"].toString()));
	if (json.contains("fill-outline-color")
	  && json["fill-outline-color"].isObject())
		_fillOutlineColor = FunctionC(json["fill-outline-color"].toObject());
	if (json.contains("fill-antialias") && json["fill-antialias"].isBool())
		_fillAntialias = json["fill-antialias"].toBool();

	if (json.contains("line-color") && json["line-color"].isString())
		_lineColor = FunctionC(Color::fromJsonString(json["line-color"]
		  .toString()));
	if (json.contains("line-color") && json["line-color"].isObject())
		_lineColor = FunctionC(json["line-color"].toObject());
	if (json.contains("line-width") && json["line-width"].isObject())
		_lineWidth = FunctionF(json["line-width"].toObject());
	else if (json.contains("line-width") && json["line-width"].isDouble())
		_lineWidth = FunctionF(json["line-width"].toDouble());
	if (json.contains("line-opacity") && json["line-opacity"].isDouble())
		_lineOpacity = FunctionF(json["line-opacity"].toDouble());
	else if (json.contains("line-opacity") && json["line-opacity"].isObject())
		_lineOpacity = FunctionF(json["line-opacity"].toObject());
	if (json.contains("line-dasharray") && json["line-dasharray"].isArray()) {
		QJsonArray array = json["line-dasharray"].toArray();
		for (int i = 0; i < array.size(); i++)
			_lineDasharray.append(array.at(i).toDouble());
	}

	if (json.contains("background-color") && json["background-color"].isString())
		_backgroundColor = FunctionC(Color::fromJsonString(
		  json["background-color"].toString()));
	if (json.contains("background-color") && json["background-color"].isObject())
		_backgroundColor = FunctionC(json["background-color"].toObject());

	if (json.contains("text-color") && json["text-color"].isString())
		_textColor = FunctionC(Color::fromJsonString(json["text-color"]
		  .toString()));
	if (json.contains("text-color") && json["text-color"].isObject())
		_textColor = FunctionC(json["text-color"].toObject());
}

QPen Style::Layer::Paint::pen(Type type, int zoom) const
{
	QPen pen(Qt::NoPen);
	qreal width;
	QColor color;

	switch (type) {
		case Line:
			width = _lineWidth.value(zoom);
			color = _lineColor.value(zoom);
			if (color.isValid() && width > 0) {
				pen = QPen(color, width);
				if (!_lineDasharray.isEmpty())
					pen.setDashPattern(_lineDasharray);
			}
			break;
		case Fill:
			color = _fillOutlineColor.value(zoom);
			if (color.isValid())
				pen = QPen(color);
			else {
				color = _fillColor.value(zoom);
				if (color.isValid())
					pen = QPen(color);
			}
			break;
		case Symbol:
			color = _textColor.value(zoom);
			if (color.isValid())
				pen = QPen(color);
			break;
		default:
			break;
	}

	return pen;
}

QBrush Style::Layer::Paint::brush(Type type, int zoom) const
{
	QColor color;

	switch (type) {
		case Fill:
			color = _fillColor.value(zoom);
			return color.isValid() ? QBrush(color) : QBrush(Qt::NoBrush);
		case Background:
			color = _backgroundColor.value(zoom);
			return color.isValid() ? QBrush(color) : QBrush(Qt::NoBrush);
		default:
			return QBrush(Qt::NoBrush);
	}
}

qreal Style::Layer::Paint::opacity(Type type, int zoom) const
{
	switch (type) {
		case Fill:
			return _fillOpacity.value(zoom);
		case Line:
			return _lineOpacity.value(zoom);
		default:
			return 1.0;
	}
}

bool Style::Layer::Paint::antialias(Layer::Type type) const
{
	switch (type) {
		case Fill:
			return _fillAntialias;
		case Line:
			return true;
		default:
			return false;
	}
}

Style::Layer::Layout::Layout(const QJsonObject &json)
  : _textSize(16), _textMaxWidth(10), _textMaxAngle(45), _lineCap(Qt::FlatCap),
  _lineJoin(Qt::MiterJoin), _capitalize(false)
{
	if (!(json.contains("text-field") && json["text-field"].isString()))
		return;

	_textField = json["text-field"].toString();

	QRegExp rx("\\{[^\\}]*\\}");
	int pos = 0;
	while ((pos = rx.indexIn(_textField, pos)) != -1) {
		QString match = rx.capturedTexts().first();
		_keys.append(match.mid(1, match.size() - 2));
		pos += rx.matchedLength();
	}

	if (json.contains("text-size") && json["text-size"].isObject())
		_textSize = FunctionF(json["text-size"].toObject());
	else if (json.contains("text-size") && json["text-size"].isDouble())
		_textSize = json["text-size"].toDouble();
	if (json.contains("text-max-width") && json["text-max-width"].isObject())
		_textMaxWidth = FunctionF(json["text-max-width"].toObject());
	else if (json.contains("text-max-width") && json["text-max-width"].isDouble())
		_textMaxWidth = json["text-max-width"].toDouble();
	if (json.contains("text-max-angle") && json["text-max-angle"].isObject())
		_textMaxWidth = FunctionF(json["text-max-angle"].toObject());
	else if (json.contains("text-max-angle") && json["text-max-angle"].isDouble())
		_textMaxWidth = json["text-max-angle"].toDouble();

	if (json.contains("text-transform") && json["text-transform"].isString())
		_capitalize = json["text-transform"].toString() == "uppercase";

	if (json.contains("line-cap") && json["line-cap"].isString()) {
		if (json["line-cap"].toString() == "round")
			_lineCap = Qt::RoundCap;
		else if (json["line-cap"].toString() == "square")
			_lineCap = Qt::SquareCap;
	}
	if (json.contains("line-join") && json["line-join"].isString()) {
		if (json["line-join"].toString() == "bevel")
			_lineJoin = Qt::BevelJoin;
		else if (json["line-join"].toString() == "round")
			_lineJoin = Qt::RoundJoin;
	}
}

QFont Style::Layer::Layout::font(int zoom) const
{
	QFont font;
	font.setPixelSize(_textSize.value(zoom));

	return font;
}

Style::Layer::Layer(const QJsonObject &json)
  : _type(Unknown), _minZoom(-1), _maxZoom(-1)
{
	// type
	QString type = json["type"].toString();
	if (type == "fill")
		_type = Fill;
	else if (type == "line")
		_type = Line;
	else if (type == "background")
		_type = Background;
	else if (type == "symbol")
		_type = Symbol;

	// source-layer
	_sourceLayer = json["source-layer"].toString();

	// zooms
	if (json.contains("minzoom") && json["minzoom"].isDouble())
		_minZoom = json["minzoom"].toInt();
	if (json.contains("maxzoom") && json["maxzoom"].isDouble())
		_maxZoom = json["maxzoom"].toInt();

	// filter
	if (json.contains("filter") && json["filter"].isArray())
		_filter = Filter(json["filter"].toArray());

	// layout
	if (json.contains("layout") && json["layout"].isObject())
		_layout = Layout(json["layout"].toObject());

	// paint
	if (json.contains("paint") && json["paint"].isObject())
		_paint = Paint(json["paint"].toObject());
}

bool Style::Layer::match(int zoom, const QVariantMap &tags) const
{
	if (zoom >= 0) {
		if (_minZoom > 0 && zoom < _minZoom)
			return false;
		if (_maxZoom > 0 && zoom > _maxZoom)
			return false;
	}

	return _filter.match(tags);
}

void Style::Layer::drawPath(int zoom, const QPainterPath &path,
  Tile &tile) const
{
	QPen pen(_paint.pen(_type, zoom));
	if (_type == Line && pen.style() == Qt::NoPen)
		return;
	QBrush brush(_paint.brush(_type, zoom));
	if (_type == Fill && brush.style() == Qt::NoBrush)
		return;

	pen.setJoinStyle(_layout.lineJoin());
	pen.setCapStyle(_layout.lineCap());

	QPainter &p = tile.painter();
	p.setRenderHint(QPainter::Antialiasing, _paint.antialias(_type));
	p.setPen(pen);
	p.setBrush(brush);
	p.setOpacity(_paint.opacity(_type, zoom));
	p.drawPath(path);
}

void Style::Layer::drawSymbol(int zoom, const QPainterPath &path,
  const QVariantMap &tags, Tile &tile) const
{
	QString text(_layout.field());
	for (int i = 0; i < _layout.keys().size(); i++) {
		const QString &key = _layout.keys().at(i);
		const QVariant val = tags.value(key);
		text.replace(QString("{%1}").arg(key), _layout.capitalize()
		  ? val.toString().toUpper() : val.toString());
	}

	QPen pen(_paint.pen(_type, zoom));
	QFont font(_layout.font(zoom));

	if (path.elementCount() == 1 && path.elementAt(0).isMoveTo())
		tile.text().addLabel(text.trimmed(), path.elementAt(0), font, pen,
		  _layout.maxTextWidth(zoom));
	else
		tile.text().addLabel(text.trimmed(), path, font, pen,
		  _layout.maxTextAngle(zoom));
}

bool Style::load(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QByteArray ba(file.readAll());
	file.close();

	QJsonObject json(QJsonDocument::fromJson(ba).object());
	if (json.contains("layers") && json["layers"].isArray()) {
		QJsonArray layers = json["layers"].toArray();
		for (int i = 0; i < layers.size(); i++)
			if (layers[i].isObject())
				_styles.append(Layer(layers[i].toObject()));
	}

	for (int i = 0; i < _styles.size(); i++)
		_sourceLayers.append(_styles.at(i).sourceLayer());

	return true;
}

bool Style::match(int layer, const QVariantMap &tags)
{
	return _styles.at(layer).match(_zoom, tags);
}

void Style::drawFeature(int layer, const QPainterPath &path,
  const QVariantMap &tags, Tile &tile)
{
	const Layer &sl = _styles.at(layer);

	if (sl.isPath())
		sl.drawPath(_zoom, path, tile);
	else if (sl.isSymbol())
		sl.drawSymbol(_zoom, path, tags, tile);
}

void Style::drawBackground(Tile &tile)
{
	QRectF rect(0, 0, tile.size(), tile.size());
	QPainterPath path;
	path.addRect(rect);

	if (_styles.isEmpty()) {
		tile.painter().setBrush(Qt::lightGray);
		tile.painter().setPen(Qt::NoPen);
		tile.painter().drawRect(rect);
	} else if (_styles.first().isBackground())
		_styles.first().drawPath(_zoom, path, tile);
}
