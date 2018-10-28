#include <QByteArray>
#include <QPainter>
#include <QDebug>
#include <QVariantMap>
#include "vector_tile.pb.h"
#include "style.h"
#include "tile.h"
#include "pbf.h"

using namespace google::protobuf;

#define MOVE_TO    1
#define LINE_TO    2
#define CLOSE_PATH 7

#define POLYGON vector_tile::Tile_GeomType::Tile_GeomType_POLYGON
#define LINESTRING vector_tile::Tile_GeomType::Tile_GeomType_LINESTRING
#define POINT vector_tile::Tile_GeomType::Tile_GeomType_POINT

struct Layer
{
	Layer(const vector_tile::Tile_Layer *data) : data(data) {}

	const vector_tile::Tile_Layer *data;
	QVector<QString> keys;
	QVector<QVariant> values;
};

struct Feature
{
	Feature(const vector_tile::Tile_Feature *data, const QVector<QString> *keys,
	  const QVector<QVariant> *values) : data(data), keys(keys),
	  values(values) {}

	const vector_tile::Tile_Feature *data;
	const QVector<QString> *keys;
	const QVector<QVariant> *values;
};

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

static QPoint parameters(quint32 v1, quint32 v2)
{
	return QPoint((v1 >> 1) ^ (-(v1 & 1)), ((v2 >> 1) ^ (-(v2 & 1))));
}

static void feature(const Feature &feature, Style *style, int styleLayer,
  qreal factor, Tile &tile)
{
	QVariantMap tags;
	for (int i = 0; i < feature.data->tags_size(); i = i + 2)
		tags.insert(feature.keys->at(feature.data->tags(i)),
		  feature.values->at(feature.data->tags(i+1)));
	switch (feature.data->type()) {
		case POLYGON:
			tags.insert("$type", QVariant("Polygon"));
			break;
		case LINESTRING:
			tags.insert("$type", QVariant("LineString"));
			break;
		case POINT:
			tags.insert("$type", QVariant("Point"));
			break;
		default:
			break;
	}

	if (!style->match(styleLayer, tags))
		return;

	QPoint cursor;
	QPainterPath path;

	for (int i = 0; i < feature.data->geometry_size(); i++) {
		quint32 g = feature.data->geometry(i);
		unsigned cmdId = g & 0x7;
		unsigned cmdCount = g >> 3;

		if (cmdId == MOVE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(feature.data->geometry(i+1),
				  feature.data->geometry(i+2));
				i += 2;
				cursor += offset;
				path.moveTo(QPointF(cursor) / factor);
			}
		} else if (cmdId == LINE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(feature.data->geometry(i+1),
				  feature.data->geometry(i+2));
				i += 2;
				cursor += offset;
				path.lineTo(QPointF(cursor) / factor);
			}
		} else if (cmdId == CLOSE_PATH) {
			path.closeSubpath();
			path.moveTo(cursor);
		}
	}

	style->drawFeature(styleLayer, path, tags, tile);
}

static void layer(const Layer &layer, Style *style, int styleLayer, Tile &tile)
{
	qreal factor = layer.data->extent() / 256.0;

	for (int i = 0; i < layer.data->features_size(); i++)
		feature(Feature(&(layer.data->features(i)), &(layer.keys),
		  &(layer.values)), style, styleLayer, factor, tile);
}

QImage PBF::image(const QByteArray &data, int zoom, Style *style)
{
	vector_tile::Tile tile;
	if (!tile.ParseFromArray(data.constData(), data.size())) {
		qCritical() << "Invalid tile protocol buffer data";
		return QImage();
	}

	Tile t;

	style->setZoom(zoom);
	style->drawBackground(t);

	QMap<QString, Layer> layers;
	for (int i = 0; i <  tile.layers_size(); i++) {
		const vector_tile::Tile_Layer &layer = tile.layers(i);
		Layer l(&layer);

		for (int j = 0; j < layer.keys_size(); j++)
			l.keys.append(QString::fromStdString(layer.keys(j)));
		for (int j = 0; j < layer.values_size(); j++)
			l.values.append(value(layer.values(j)));

		layers.insert(QString::fromStdString(tile.layers(i).name()), l);
	}

	// Process data in order of style layers
	for (int i = 0; i < style->sourceLayers().size(); i++) {
		QMap<QString, Layer>::const_iterator it = layers.find(
		  style->sourceLayers().at(i));
		if (it == layers.constEnd())
			continue;

		layer(*it, style, i, t);
	}

	return t.render();
}
