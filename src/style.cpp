#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include "text.h"
#include "color.h"
#include "font.h"
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

QString Style::Layer::Template::value(int zoom, const QVariantHash &tags) const
{
	QRegExp rx = QRegExp("\\{[^\\}]*\\}");
	QString text(_field.value(zoom));
	QStringList keys;
	int pos = 0;

	while ((pos = rx.indexIn(text, pos)) != -1) {
		QString match = rx.capturedTexts().first();
		keys.append(match.mid(1, match.size() - 2));
		pos += rx.matchedLength();
	}
	for (int i = 0; i < keys.size(); i++) {
		const QString &key = keys.at(i);
		const QVariant val = tags.value(key);
		text.replace(QString("{%1}").arg(key), val.toString());
	}

	return text;
}

Style::Layer::Paint::Paint(const QJsonObject &json)
{
	// fill
	_fillOpacity = FunctionF(json["fill-opacity"], 1.0);
	_fillColor = FunctionC(json["fill-color"]);
	if (json.contains("fill-outline-color"))
		_fillOutlineColor = FunctionC(json["fill-outline-color"]);
	else
		_fillOutlineColor = _fillColor;
	_fillAntialias = FunctionB(json["fill-antialias"]);
	if (json.contains("fill-pattern")) {
		_fillPattern = FunctionS(json["fill-pattern"]);
		_fillColor = FunctionC(QColor());
		_fillOutlineColor = FunctionC(QColor());
	}

	// line
	_lineColor = FunctionC(json["line-color"]);
	_lineWidth = FunctionF(json["line-width"], 1.0);
	_lineOpacity = FunctionF(json["line-opacity"], 1.0);
	if (json.contains("line-dasharray") && json["line-dasharray"].isArray()) {
		QJsonArray array = json["line-dasharray"].toArray();
		for (int i = 0; i < array.size(); i++)
			_lineDasharray.append(array.at(i).toDouble());
	}

	// background
	_backgroundColor = FunctionC(json["background-color"]);

	// text
	_textColor = FunctionC(json["text-color"]);
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

QBrush Style::Layer::Paint::brush(Type type, int zoom, const Sprites &sprites) const
{
	QColor color;
	QBrush brush(Qt::NoBrush);
	QString pattern;

	switch (type) {
		case Fill:
			color = _fillColor.value(zoom);
			if (color.isValid())
				brush = QBrush(color);
			pattern = _fillPattern.value(zoom);
			if (!pattern.isNull())
				brush.setTextureImage(sprites.icon(pattern));
			break;
		case Background:
			color = _backgroundColor.value(zoom);
			if (color.isValid())
				brush = QBrush(color);
			pattern = _fillPattern.value(zoom);
			if (!pattern.isNull())
				brush.setTextureImage(sprites.icon(pattern));
			break;
		default:
			break;
	}

	return brush;
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
  : _lineCap(Qt::FlatCap), _lineJoin(Qt::MiterJoin), _font("Open Sans"),
  _capitalize(false), _viewportAlignment(false)
{
	// line
	_lineCap = FunctionS(json["line-cap"], "butt");
	_lineJoin = FunctionS(json["line-join"], "miter");

	// text
	_text = Template(FunctionS(json["text-field"]));

	_textSize = FunctionF(json["text-size"], 16);
	_textMaxWidth = FunctionF(json["text-max-width"], 10);
	_textMaxAngle = FunctionF(json["text-max-angle"], 45);
	if (json.contains("text-font") && json["text-font"].isArray())
		_font = Font::fromJsonArray(json["text-font"].toArray());
	if (json.contains("text-transform") && json["text-transform"].isString())
		_capitalize = json["text-transform"].toString() == "uppercase";
	if (json.contains("text-rotation-alignment")
	  && json["text-rotation-alignment"].isString())
		if (json["text-rotation-alignment"].toString() == "viewport")
			_viewportAlignment = true;

	_textAnchor = FunctionS(json["text-anchor"]);

	// icon
	_icon = Template(FunctionS(json["icon-image"]));
}

QFont Style::Layer::Layout::font(int zoom) const
{
	QFont font(_font);
	font.setPixelSize(_textSize.value(zoom));

	return font;
}

Text::Anchor Style::Layer::Layout::textAnchor(int zoom) const
{
	QString anchor(_textAnchor.value(zoom));

	if (anchor == "left")
		return Text::Left;
	else if (anchor == "right")
		return Text::Right;
	else if (anchor == "top")
		return Text::Top;
	else if (anchor == "bottom")
		return Text::Bottom;
	else
		return Text::Center;
}

Qt::PenCapStyle Style::Layer::Layout::lineCap(int zoom) const
{
	QString cap(_lineCap.value(zoom));

	if (cap == "round")
		return Qt::RoundCap;
	else if (cap == "square")
		return Qt::SquareCap;
	else
		return Qt::FlatCap;
}

Qt::PenJoinStyle Style::Layer::Layout::lineJoin(int zoom) const
{
	QString join(_lineJoin.value(zoom));

	if (join == "bevel")
		return Qt::BevelJoin;
	else if (join == "round")
		return Qt::RoundJoin;
	else
		return Qt::MiterJoin;
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

void Style::Layer::setPathPainter(Tile &tile, const Sprites &sprites) const
{
	QPen pen(_paint.pen(_type, tile.zoom()));
	QBrush brush(_paint.brush(_type, tile.zoom(), sprites));

	pen.setJoinStyle(_layout.lineJoin(tile.zoom()));
	pen.setCapStyle(_layout.lineCap(tile.zoom()));

	QPainter &p = tile.painter();
	p.setRenderHint(QPainter::Antialiasing, _paint.antialias(_type, tile.zoom()));
	p.setPen(pen);
	p.setBrush(brush);
	p.setOpacity(_paint.opacity(_type, tile.zoom()));
}

void Style::Layer::setSymbolPainter(Tile &tile) const
{
	QPen pen(_paint.pen(_type, tile.zoom()));
	QFont font(_layout.font(tile.zoom()));

	QPainter &p = tile.painter();
	p.setPen(pen);
	p.setFont(font);
}

void Style::Layer::setTextProperties(Tile &tile) const
{
	Text::Properties prop;
	prop.maxWidth = _layout.maxTextWidth(tile.zoom());
	prop.maxAngle = _layout.maxTextAngle(tile.zoom());
	prop.anchor = _layout.textAnchor(tile.zoom());

	tile.text().setProperties(prop);
}

void Style::Layer::addSymbol(Tile &tile, const QPainterPath &path,
  const QVariantHash &tags, const Sprites &sprites) const
{
	QString text = _layout.text(tile.zoom(), tags);
	QString tt(text.trimmed());
	if (tt.isEmpty())
		return;
	if (_layout.capitalize())
		tt = tt.toUpper();

	QString icon = _layout.icon(tile.zoom(), tags);

	if (_layout.viewportAlignment())
		tile.text().addLabel(tt, path.elementAt(0), tile.painter(), false,
		  sprites.icon(icon));
	else if (path.elementCount() == 1 && path.elementAt(0).isMoveTo())
		tile.text().addLabel(tt, path.elementAt(0), tile.painter(), true,
		  sprites.icon(icon));
	else
		tile.text().addLabel(tt, path, tile.painter());
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
				_layers.append(Layer(layers[i].toObject()));
	}

	for (int i = 0; i < _layers.size(); i++)
		_sourceLayers.append(_layers.at(i).sourceLayer());

	QDir styleDir = QFileInfo(fileName).absoluteDir();
	QString spritesJSON(styleDir.filePath("sprite.json"));
	if (QFileInfo::exists(spritesJSON)) {
		QString spritesImg(styleDir.filePath("sprite.png"));
		if (QFileInfo::exists(spritesImg))
			_sprites.load(spritesJSON, spritesImg);
		else
			qCritical() << spritesImg << ": no such file";
	}

	return true;
}

bool Style::match(int zoom, int layer, const QVariantHash &tags) const
{
	return _layers.at(layer).match(zoom, tags);
}

void Style::setTextProperties(Tile &tile, int layer) const
{
	const Layer &sl = _layers.at(layer);
	sl.setTextProperties(tile);
}

void Style::setPainter(Tile &tile, int layer) const
{
	const Layer &sl = _layers.at(layer);

	if (sl.isPath())
		sl.setPathPainter(tile, _sprites);
	else if (sl.isSymbol())
		sl.setSymbolPainter(tile);
}

void Style::drawFeature(Tile &tile, int layer, const QPainterPath &path,
  const QVariantHash &tags) const
{
	const Layer &sl = _layers.at(layer);

	if (sl.isPath())
		tile.painter().drawPath(path);
	else if (sl.isSymbol())
		sl.addSymbol(tile, path, tags, _sprites);
}

void Style::drawBackground(Tile &tile) const
{
	QRectF rect(QPointF(0, 0), tile.size());
	QPainterPath path;
	path.addRect(rect);

	if (_layers.isEmpty()) {
		tile.painter().setBrush(Qt::lightGray);
		tile.painter().setPen(Qt::NoPen);
		tile.painter().drawRect(rect);
	} else if (_layers.first().isBackground()) {
		_layers.first().setPathPainter(tile, _sprites);
		tile.painter().drawPath(path);
	}

	//tile.painter().setPen(Qt::red);
	//tile.painter().drawRect(rect);
}
