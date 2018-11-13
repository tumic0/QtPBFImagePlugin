#include <QByteArray>
#include <QPainter>
#include <QDebug>
#include <QVariantHash>
#include "vector_tile.pb.h"
#include "style.h"
#include "tile.h"
#include "pbf.h"


#define MOVE_TO    1
#define LINE_TO    2
#define CLOSE_PATH 7

#define POLYGON vector_tile::Tile_GeomType::Tile_GeomType_POLYGON
#define LINESTRING vector_tile::Tile_GeomType::Tile_GeomType_LINESTRING
#define POINT vector_tile::Tile_GeomType::Tile_GeomType_POINT


static QVariant value(const vector_tile::Tile_Value &val)
{
	if (val.has_bool_value())
		return QVariant(val.bool_value());
	else if (val.has_int_value())
		return QVariant((qlonglong)val.int_value());
	else if (val.has_sint_value())
		return QVariant((qlonglong)val.sint_value());
	else if (val.has_uint_value())
		return QVariant((qulonglong)val.uint_value());
	else if (val.has_float_value())
		return QVariant(val.float_value());
	else if (val.has_double_value())
		return QVariant(val.double_value());
	else if (val.has_string_value())
		return QVariant(QString::fromStdString(val.string_value()));
	else
		return QVariant();
}

class Feature
{
public:
	Feature() : _data(0) {}
	Feature(const vector_tile::Tile_Feature *data, const QVector<QString> *keys,
	  const QVector<QVariant> *values) : _data(data)
	{
		for (int i = 0; i < data->tags_size(); i = i + 2)
			_tags.insert(keys->at(data->tags(i)), values->at(data->tags(i+1)));

		switch (data->type()) {
			case POLYGON:
				_tags.insert("$type", QVariant("Polygon"));
				break;
			case LINESTRING:
				_tags.insert("$type", QVariant("LineString"));
				break;
			case POINT:
				_tags.insert("$type", QVariant("Point"));
				break;
			default:
				break;
		}
	}

	const QVariantHash &tags() const {return _tags;}
	const vector_tile::Tile_Feature *data() const {return _data;}

private:
	QVariantHash _tags;
	const vector_tile::Tile_Feature *_data;
};

bool cmp(const Feature &f1, const Feature &f2)
{
	return f1.data()->id() < f2.data()->id();
}

class Layer
{
public:
	Layer(const vector_tile::Tile_Layer *data) : _data(data)
	{
		QVector<QString> keys;
		QVector<QVariant> values;

		for (int i = 0; i < data->keys_size(); i++)
			keys.append(QString::fromStdString(data->keys(i)));
		for (int i = 0; i < data->values_size(); i++)
			values.append(value(data->values(i)));

		_features.reserve(data->features_size());
		for (int i = 0; i < data->features_size(); i++)
			_features.append(Feature(&(data->features(i)), &keys, &values));
		qSort(_features.begin(), _features.end(), cmp);
	}

	const QVector<Feature> &features() const {return _features;}
	const vector_tile::Tile_Layer *data() const {return _data;}

private:
	const vector_tile::Tile_Layer *_data;
	QVector<Feature> _features;
};

static inline qint32 zigzag32decode(quint32 value)
{
	return static_cast<qint32>((value >> 1u) ^ static_cast<quint32>(
	  -static_cast<qint32>(value & 1u)));
}

static inline QPoint parameters(quint32 v1, quint32 v2)
{
	return QPoint(zigzag32decode(v1), zigzag32decode(v2));
}

static void processFeature(const Feature &feature, Style *style, int styleLayer,
  const QSizeF &factor, Tile &tile)
{
	if (!style->match(styleLayer, feature.tags()))
		return;

	QPoint cursor;
	QPainterPath path;

	for (int i = 0; i < feature.data()->geometry_size(); i++) {
		quint32 g = feature.data()->geometry(i);
		unsigned cmdId = g & 0x7;
		unsigned cmdCount = g >> 3;

		if (cmdId == MOVE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(feature.data()->geometry(i+1),
				  feature.data()->geometry(i+2));
				i += 2;
				cursor += offset;
				path.moveTo(QPointF(cursor.x() * factor.width(),
				  cursor.y() * factor.height()));
			}
		} else if (cmdId == LINE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(feature.data()->geometry(i+1),
				  feature.data()->geometry(i+2));
				i += 2;
				cursor += offset;
				path.lineTo(QPointF(cursor.x() * factor.width(),
				  cursor.y() * factor.height()));
			}
		} else if (cmdId == CLOSE_PATH) {
			path.closeSubpath();
			path.moveTo(cursor);
		}
	}

	style->processFeature(styleLayer, path, feature.tags(), tile);
}

static void drawLayer(const Layer &layer, Style *style, int styleLayer,
  Tile &tile)
{
	if (layer.data()->version() > 2)
		return;

	QSizeF factor(tile.size().width() / (qreal)layer.data()->extent(),
	  tile.size().height() / (qreal)layer.data()->extent());

	style->setPainter(styleLayer, tile);
	for (int i = 0; i < layer.features().size(); i++)
		processFeature(layer.features().at(i), style, styleLayer, factor, tile);
}

bool PBF::render(const QByteArray &data, int zoom, Style *style, qreal scale,
  QImage *image)
{
	vector_tile::Tile tile;
	if (!tile.ParseFromArray(data.constData(), data.size())) {
		qCritical() << "Invalid tile protocol buffer data";
		return false;
	}

	Tile t(image, scale);

	style->setZoom(zoom);
	style->drawBackground(t);

	// Prepare source layers
	QMap<QString, Layer> layers;
	for (int i = 0; i <  tile.layers_size(); i++) {
		const vector_tile::Tile_Layer &layer = tile.layers(i);
		QString name(QString::fromStdString(layer.name()));
		if (style->sourceLayers().contains(name))
			layers.insert(name, Layer(&layer));
	}

	// Process source layers in order of style layers
	for (int i = 0; i < style->sourceLayers().size(); i++) {
		QMap<QString, Layer>::const_iterator it = layers.find(
		  style->sourceLayers().at(i));
		if (it == layers.constEnd())
			continue;

		drawLayer(*it, style, i, t);
	}

	t.text().render(&t.painter());

	return true;
}
