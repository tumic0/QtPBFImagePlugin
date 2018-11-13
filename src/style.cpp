#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include "text.h"
#include "color.h"
#include "font.h"
#include "tile.h"
#include "style.h"


static void jsonColor(const QJsonObject &json, const char *name, FunctionC &var)
{
	if (json.contains(name)) {
		const QJsonValue value = json[name];
		if (value.isString())
			var = FunctionC(Color::fromJsonString(value.toString()));
		else if (value.isObject())
			var = FunctionC(value.toObject());
	}
}

static void jsonFloat(const QJsonObject &json, const char *name, FunctionF &var)
{
	if (json.contains(name)) {
		const QJsonValue value = json[name];
		if (value.isDouble())
			var = FunctionF(value.toDouble());
		else if (value.isObject())
			var = FunctionF(value.toObject());
	}
}

static void jsonBool(const QJsonObject &json, const char *name, FunctionB &var)
{
	if (json.contains(name)) {
		QJsonValue value = json[name];
		if (value.isBool())
			var = FunctionB(value.toBool());
		else if (value.isObject())
			var = FunctionB(value.toObject());
	}
}


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

bool Style::Layer::Filter::match(const QVariantHash &tags) const
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
  : _fillOpacity(1.0), _lineOpacity(1.0), _lineWidth(1.0)
{
	// fill
	jsonFloat(json, "fill-opacity", _fillOpacity);
	jsonColor(json, "fill-color", _fillColor);
	_fillOutlineColor = _fillColor;
	jsonColor(json, "fill-outline-color", _fillOutlineColor);
	jsonBool(json, "fill-antialias",_fillAntialias);
	if (json.contains("fill-pattern")) {
		_fillColor = FunctionC(QColor());
		_fillOutlineColor = FunctionC(QColor());
	}

	// line
	jsonColor(json, "line-color", _lineColor);
	jsonFloat(json, "line-width", _lineWidth);
	jsonFloat(json, "line-opacity", _lineOpacity);
	if (json.contains("line-dasharray") && json["line-dasharray"].isArray()) {
		QJsonArray array = json["line-dasharray"].toArray();
		for (int i = 0; i < array.size(); i++)
			_lineDasharray.append(array.at(i).toDouble());
	}

	// background
	jsonColor(json, "background-color", _backgroundColor);

	// text
	jsonColor(json, "text-color", _textColor);
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

bool Style::Layer::Paint::antialias(Layer::Type type, int zoom) const
{
	switch (type) {
		case Fill:
			return _fillAntialias.value(zoom);
		case Line:
			return true;
		default:
			return false;
	}
}

Style::Layer::Layout::Layout(const QJsonObject &json)
  : _textSize(16), _textMaxWidth(10), _textMaxAngle(45), _lineCap(Qt::FlatCap),
  _lineJoin(Qt::MiterJoin), _font("Open Sans"), _capitalize(false)
{
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

	jsonFloat(json, "text-size", _textSize);
	jsonFloat(json, "text-max-width", _textMaxWidth);
	jsonFloat(json, "text-max-angle", _textMaxAngle);
	if (json.contains("text-font") && json["text-font"].isArray())
		_font = Font::fromJsonArray(json["text-font"].toArray());
	if (json.contains("text-transform") && json["text-transform"].isString())
		_capitalize = json["text-transform"].toString() == "uppercase";
}

QFont Style::Layer::Layout::font(int zoom) const
{
	QFont font(_font);
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

bool Style::Layer::match(int zoom, const QVariantHash &tags) const
{
	if (zoom >= 0) {
		if (_minZoom > 0 && zoom < _minZoom)
			return false;
		if (_maxZoom > 0 && zoom > _maxZoom)
			return false;
	}

	return _filter.match(tags);
}

void Style::Layer::setPainter(int zoom, Tile &tile) const
{
	QPen pen(_paint.pen(_type, zoom));
	QBrush brush(_paint.brush(_type, zoom));

	pen.setJoinStyle(_layout.lineJoin());
	pen.setCapStyle(_layout.lineCap());

	QPainter &p = tile.painter();
	p.setRenderHint(QPainter::Antialiasing, _paint.antialias(_type, zoom));
	p.setPen(pen);
	p.setBrush(brush);
	p.setOpacity(_paint.opacity(_type, zoom));
}

void Style::Layer::addSymbol(int zoom, const QPainterPath &path,
  const QVariantHash &tags, Tile &tile) const
{
	if (_layout.keys().isEmpty())
		return;

	QString text(_layout.field());
	for (int i = 0; i < _layout.keys().size(); i++) {
		const QString &key = _layout.keys().at(i);
		const QVariant val = tags.value(key);
		text.replace(QString("{%1}").arg(key), _layout.capitalize()
		  ? val.toString().toUpper() : val.toString());
	}

	QString tt(text.trimmed());
	if (tt.isEmpty())
		return;

	QPen pen(_paint.pen(_type, zoom));
	QFont font(_layout.font(zoom));

	if (path.elementCount() == 1 && path.elementAt(0).isMoveTo())
		tile.text().addLabel(tt, path.elementAt(0), font, pen,
		  _layout.maxTextWidth(zoom));
	else
		tile.text().addLabel(tt, path, font, pen, _layout.maxTextAngle(zoom));
}

bool Style::load(const QString &fileName)
{
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qCritical() << fileName << ": error opening file";
		return false;
	}
	QByteArray ba(file.readAll());
	file.close();

	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(ba, &error));
	if (doc.isNull()) {
		qCritical() << fileName << ":" << error.errorString();
		return false;
	}

	QJsonObject json(doc.object());
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

bool Style::match(int layer, const QVariantHash &tags)
{
	return _styles.at(layer).match(_zoom, tags);
}

void Style::setPainter(int layer, Tile &tile)
{
	const Layer &sl = _styles.at(layer);

	if (sl.isPath())
		sl.setPainter(_zoom, tile);
}

void Style::processFeature(int layer, const QPainterPath &path,
  const QVariantHash &tags, Tile &tile)
{
	const Layer &sl = _styles.at(layer);

	if (sl.isPath())
		tile.painter().drawPath(path);
	else if (sl.isSymbol())
		sl.addSymbol(_zoom, path, tags, tile);
}

void Style::drawBackground(Tile &tile)
{
	QRectF rect(QPointF(0, 0), tile.size());
	QPainterPath path;
	path.addRect(rect);

	if (_styles.isEmpty()) {
		tile.painter().setBrush(Qt::lightGray);
		tile.painter().setPen(Qt::NoPen);
		tile.painter().drawRect(rect);
	} else if (_styles.first().isBackground()) {
		_styles.first().setPainter(_zoom, tile);
		tile.painter().drawPath(path);
	}
}
