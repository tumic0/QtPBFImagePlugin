#include <QPainter>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QDebug>
#include "text.h"
#include "font.h"
#include "tile.h"
#include "pbf.h"
#include "style.h"


static Data::GeomType geometryType(const QString &str)
{
	if (str == "Point")
		return Data::GeomType::POINT;
	else if (str == "LineString")
		return Data::GeomType::LINESTRING;
	else if (str == "Polygon")
		return Data::GeomType::POLYGON;
	else
		return Data::GeomType::UNKNOWN;
}

static QVariant variant(const QJsonValue &val)
{
	switch (val.type()) {
		case QJsonValue::String:
			return QVariant(val.toString().toUtf8());
		case QJsonValue::Double:
		case QJsonValue::Bool:
			return val.toVariant();
		default:
			qWarning() << val << ": invalid filter value";
			return QVariant();
	}
}

Style::Layer::Filter::Filter(const QJsonArray &json)
  : _type(Unknown), _not(false)
{
#define INVALID_FILTER(json) \
	{qWarning() << json << ": invalid filter"; return;}

	if (json.isEmpty())
		INVALID_FILTER(json);

	QString type(json.at(0).toString());

	if (type == "==") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		if (json.at(1).toString() == "$type") {
			_type = GeometryType;
			_kv = QPair<QByteArray, QVariant>(QByteArray(),
			  QVariant(geometryType(json.at(2).toString())));
		} else {
			_type = EQ;
			_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
			  variant(json.at(2)));
		}
	} else if (type == "!=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = NE;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == "<") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = LT;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == "<=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = LE;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == ">") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = GT;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
	} else if (type == ">=") {
		if (json.size() != 3)
			INVALID_FILTER(json);
		_type = GE;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  variant(json.at(2)));
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
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
		for (int i = 2; i < json.size(); i++)
			_set.insert(json.at(i).toString().toUtf8());
	}  else if (type == "!in") {
		if (json.size() < 3)
			INVALID_FILTER(json);
		_type = In;
		_not = true;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
		for (int i = 2; i < json.size(); i++)
			_set.insert(json.at(i).toString().toUtf8());
	} else if (type == "has") {
		if (json.size() < 2)
			INVALID_FILTER(json);
		_type = Has;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
	} else if (type == "!has") {
		if (json.size() < 2)
			INVALID_FILTER(json);
		_type = Has;
		_not = true;
		_kv = QPair<QByteArray, QVariant>(json.at(1).toString().toUtf8(),
		  QVariant());
	} else
		INVALID_FILTER(json);
}

bool Style::Layer::Filter::match(const PBF::Feature &feature) const
{
	const QVariant *v;

	switch (_type) {
		case None:
			return true;
		case EQ:
			if (!(v = feature.value(_kv.first)))
				return false;
			else
				return *v == _kv.second;
		case NE:
			if (!(v = feature.value(_kv.first)))
				return true;
			else
				return *v != _kv.second;
		case GT:
			if (!(v = feature.value(_kv.first)))
				return false;
			else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v > _kv.second;
#else // QT6
				return (QVariant::compare(*v, _kv.second)
				  == QPartialOrdering::Greater);
#endif // QT6
		case GE:
			{if (!(v = feature.value(_kv.first)))
				return false;
			else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v >= _kv.second;
#else // QT6
				QPartialOrdering res = QVariant::compare(*v, _kv.second);
				return (res == QPartialOrdering::Greater
				  || res == QPartialOrdering::Equivalent);
#endif // QT6
			}}
		case LT:
			if (!(v = feature.value(_kv.first)))
				return false;
			else
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v < _kv.second;
#else // QT6
				return (QVariant::compare(*v, _kv.second)
				  == QPartialOrdering::Less);
#endif // QT6
		case LE:
			{if (!(v = feature.value(_kv.first)))
				return false;
			else {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
				return *v <= _kv.second;
#else // QT6
				QPartialOrdering res = QVariant::compare(*v, _kv.second);
				return (res == QPartialOrdering::Less
				  || res == QPartialOrdering::Equivalent);
#endif // QT6
			}}
		case In:
			if (!(v = feature.value(_kv.first)))
				return _not;
			else
				return _set.contains((*v).toByteArray()) ^ _not;
		case Has:
			return (feature.value(_kv.first) ? true : false) ^ _not;
		case All:
			for (int i = 0; i < _filters.size(); i++)
				if (!_filters.at(i).match(feature))
					return false;
			return true;
		case Any:
			for (int i = 0; i < _filters.size(); i++)
				if (_filters.at(i).match(feature))
					return true;
			return false;
		case GeometryType:
			return feature.type() == _kv.second.toInt();
		default:
			return false;
	}
}

QString Style::Layer::Template::value(int zoom, const PBF::Feature &feature) const
{
	static QRegularExpression rx("\\{[^\\}]*\\}");
	QString text(_field.value(zoom));
	QRegularExpressionMatchIterator it = rx.globalMatch(text);
	QStringList keys;

	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QString val = match.captured(0);
		keys.append(val.mid(1, val.size() - 2));
	}
	for (int i = 0; i < keys.size(); i++) {
		const QString &key = keys.at(i);
		const QVariant *val = feature.value(key.toUtf8());
		text.replace(QString("{%1}").arg(key), val ? val->toString() : "");
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
	_textHaloColor = FunctionC(json["text-halo-color"], QColor());
	_textHaloWidth = FunctionF(json["text-halo-width"]);
	_textHaloBlur = FunctionF(json["text-halo-blur"]);

	// icon
	_iconColor = FunctionC(json["icon-color"]);
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

QBrush Style::Layer::Paint::brush(Type type, int zoom, Sprites &sprites)
  const
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
  : _lineCap(Qt::FlatCap), _lineJoin(Qt::MiterJoin), _font("Open Sans")
{
	// line
	_lineCap = FunctionS(json["line-cap"], "butt");
	_lineJoin = FunctionS(json["line-join"], "miter");

	// text
	_text = Template(FunctionS(json["text-field"]));

	_textSize = FunctionF(json["text-size"], 16);
	_textMaxWidth = FunctionF(json["text-max-width"], 10);
	_textMaxAngle = FunctionF(json["text-max-angle"], 45);
	_textTransform = FunctionS(json["text-transform"], "none");
	_textRotationAlignment = FunctionS(json["text-rotation-alignment"]);
	_textAnchor = FunctionS(json["text-anchor"]);

	if (json.contains("text-font") && json["text-font"].isArray())
		_font = Font::fromJsonArray(json["text-font"].toArray());

	// icon
	_icon = Template(FunctionS(json["icon-image"]));
	_iconSize = FunctionF(json["icon-size"]);

	// symbol
	_symbolPlacement = FunctionS(json["symbol-placement"]);

	// visibility
	if (json.contains("visibility") && json["visibility"].isString())
		_visible = !(json["visibility"].toString() == "none");
	else
		_visible = true;
}

QFont Style::Layer::Layout::font(int zoom) const
{
	QFont font(_font);
	font.setPixelSize(_textSize.value(zoom));
	font.setCapitalization(textTransform(zoom));

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

QFont::Capitalization Style::Layer::Layout::textTransform(int zoom) const
{
	QString transform(_textTransform.value(zoom));

	if (transform == "uppercase")
		return QFont::AllUppercase;
	else if (transform == "lowercase")
		return QFont::AllLowercase;
	else
		return QFont::MixedCase;
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

Text::SymbolPlacement Style::Layer::Layout::symbolPlacement(int zoom) const
{
	QString placement(_symbolPlacement.value(zoom));

	if (placement == "line")
		return Text::Line;
	else if (placement == "line-center")
		return Text::LineCenter;
	else
		return Text::Point;
}

Text::RotationAlignment Style::Layer::Layout::textRotationAlignment(int zoom)
  const
{
	QString alignment(_textRotationAlignment.value(zoom));

	if (alignment == "map")
		return Text::Map;
	else if (alignment == "viewport")
		return Text::Viewport;
	else
		return Text::Auto;
}

Style::Layer::Layer(const QJsonObject &json)
  : _type(Unknown), _minZoom(0), _maxZoom(24)
{
	// type
	QString type(json["type"].toString());

	if (type == "fill")
		_type = Fill;
	else if (type == "line")
		_type = Line;
	else if (type == "background")
		_type = Background;
	else if (type == "symbol")
		_type = Symbol;

	// source-layer
	_sourceLayer = json["source-layer"].toString().toUtf8();

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

bool Style::Layer::match(int zoom, const PBF::Feature &feature) const
{
	if (zoom >= 0 && (zoom < _minZoom || zoom > _maxZoom))
		return false;

	return _filter.match(feature);
}

void Style::Layer::setPathPainter(Tile &tile, Sprites &sprites) const
{
	QPainter &p = tile.painter();
	int zoom = tile.zoom();

	QPen pen(_paint.pen(_type, zoom));
	pen.setJoinStyle(_layout.lineJoin(zoom));
	pen.setCapStyle(_layout.lineCap(zoom));

	QBrush brush(_paint.brush(_type, zoom, sprites));

	p.setRenderHint(QPainter::Antialiasing, _paint.antialias(_type, zoom));
	p.setPen(pen);
	p.setBrush(brush);
	p.setOpacity(_paint.opacity(_type, zoom));
}

void Style::Layer::setTextProperties(Tile &tile) const
{
	Text &t = tile.text();
	int zoom = tile.zoom();

	t.setMaxWidth(_layout.maxTextWidth(zoom));
	t.setMaxAngle(_layout.maxTextAngle(zoom));
	t.setAnchor(_layout.textAnchor(zoom));
	t.setPen(_paint.pen(_type, zoom));
	t.setFont(_layout.font(zoom));
	t.setSymbolPlacement(_layout.symbolPlacement(zoom));
	t.setRotationAlignment(_layout.textRotationAlignment(zoom));
	t.setHalo(_paint.halo(zoom));
}

void Style::Layer::addSymbol(Tile &tile, const QPainterPath &path,
  const PBF::Feature &feature, Sprites &sprites) const
{
	QString text(_layout.text(tile.zoom(), feature));
	QString icon(_layout.icon(tile.zoom(), feature));
	QColor color(_paint.iconColor(tile.zoom()));
	qreal size(_layout.iconSize(tile.zoom()));
	QImage img(sprites.icon(icon, color, size));

	if (text.isEmpty() && img.isNull())
		return;

	tile.text().addLabel(text, img, path);
}

static bool loadSprites(const QDir &styleDir, const QString &json,
  const QString &img, Sprites &sprites)
{
	QString spritesJSON(styleDir.filePath(json));

	if (QFileInfo::exists(spritesJSON)) {
		QString spritesImg(styleDir.filePath(img));
		if (QFileInfo::exists(spritesImg))
			return sprites.load(spritesJSON, spritesImg);
		else {
			qWarning() << spritesImg << ": no such file";
			return false;
		}
	}

	return true;
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

	QDir styleDir(QFileInfo(fileName).absoluteDir());
	loadSprites(styleDir, "sprite.json", "sprite.png", _sprites);
	loadSprites(styleDir, "sprite@2x.json", "sprite@2x.png", _sprites2x);

	return true;
}

Sprites &Style::sprites(const QPointF &scale)
{
	return (scale.x() > 1.0 || scale.y() > 1.0)
	  && !_sprites2x.isNull() ? _sprites2x : _sprites;
}

void Style::setupLayer(Tile &tile, const Layer &layer)
{
	if (layer.isSymbol())
		layer.setTextProperties(tile);
	else if (layer.isPath())
		layer.setPathPainter(tile, sprites(tile.scale()));
}

void Style::drawBackground(Tile &tile)
{
	QRectF rect(QPointF(0, 0), QSizeF(tile.size().width() / tile.scale().x(),
	  tile.size().height() / tile.scale().y()));
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
}

void Style::drawFeature(const PBF::Feature &feature, const Layer &layer,
  Tile &tile, const QSizeF &factor)
{
	if (!layer.match(tile.zoom(), feature))
		return;

	QPainterPath path(feature.path(factor));
	if (!path.elementCount())
		return;

	if (layer.isPath())
		tile.painter().drawPath(path);
	else if (layer.isSymbol())
		layer.addSymbol(tile, path, feature, sprites(tile.scale()));
}

void Style::drawLayer(const PBF::Layer &pbfLayer, const Layer &styleLayer,
  Tile &tile)
{
	if (pbfLayer.data()->version > 2)
		return;

	if (!styleLayer.isVisible())
		return;

	QSizeF factor(tile.size().width() / tile.scale().x() /
	  (qreal)pbfLayer.data()->extent, tile.size().height() / tile.scale().y()
	  / (qreal)pbfLayer.data()->extent);

	tile.painter().save();
	setupLayer(tile, styleLayer);
	for (int i = 0; i < pbfLayer.features().size(); i++)
		drawFeature(pbfLayer.features().at(i), styleLayer, tile, factor);
	tile.painter().restore();
}

void Style::render(const PBF &data, Tile &tile)
{
	drawBackground(tile);

	for (int i = 0; i < _layers.size(); i++) {
		QHash<QByteArray, PBF::Layer*>::const_iterator it = data.layers().find(
		  _layers.at(i).sourceLayer());
		if (it == data.layers().constEnd())
			continue;

		drawLayer(**it, _layers.at(i), tile);
	}

	tile.text().render(&tile.painter());

	//QRectF rect(QPointF(0, 0), QSizeF(tile.size().width() / tile.scale().x(),
	//  tile.size().height() / tile.scale().y()));
	//tile.painter().setPen(Qt::red);
	//tile.painter().setBrush(Qt::NoBrush);
	//tile.painter().setRenderHint(QPainter::Antialiasing, false);
	//tile.painter().drawRect(rect);
}
