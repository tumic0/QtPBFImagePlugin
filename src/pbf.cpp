#include "pbf.h"

#define POLYGON vector_tile::Tile_GeomType_POLYGON
#define LINESTRING vector_tile::Tile_GeomType_LINESTRING
#define POINT vector_tile::Tile_GeomType_POINT

#define MOVE_TO    1
#define LINE_TO    2
#define CLOSE_PATH 7


static inline qint32 zigzag32decode(quint32 value)
{
	return static_cast<qint32>((value >> 1u) ^ static_cast<quint32>(
	  -static_cast<qint32>(value & 1u)));
}

static inline QPoint parameters(quint32 v1, quint32 v2)
{
	return QPoint(zigzag32decode(v1), zigzag32decode(v2));
}

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

const QVariant *PBF::Feature::value(const QString &key) const
{
	const KeyHash &keys(_layer->keys());
	KeyHash::const_iterator it(keys.find(key));
	if (it == keys.constEnd())
		return 0;

	const google::protobuf::RepeatedField<google::protobuf::uint32>
	  &tags(_data->tags());
	google::protobuf::uint32 index = *it;
	for (int i = 0; i < _data->tags_size(); i = i + 2)
		if (tags[i] == index)
			return &(_layer->values().at(tags[i+1]));

	return 0;
}

QPainterPath PBF::Feature::path(const QSizeF &factor) const
{
	QPoint cursor;
	QPainterPath path;

	for (int i = 0; i < _data->geometry_size(); i++) {
		quint32 g = _data->geometry(i);
		unsigned cmdId = g & 0x7;
		unsigned cmdCount = g >> 3;

		if (cmdId == MOVE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(_data->geometry(i+1),
				  _data->geometry(i+2));
				i += 2;
				cursor += offset;
				path.moveTo(QPointF(cursor.x() * factor.width(),
				  cursor.y() * factor.height()));
			}
		} else if (cmdId == LINE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(_data->geometry(i+1),
				  _data->geometry(i+2));
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

	return path;
}

PBF::Layer::Layer(const vector_tile::Tile_Layer *data) : _data(data)
{
	_keys.reserve(data->keys_size());
	for (int i = 0; i < data->keys_size(); i++)
		_keys.insert(QString::fromStdString(data->keys(i)), i);
	_values.reserve(data->values_size());
	for (int i = 0; i < data->values_size(); i++)
		_values.append(value(data->values(i)));

	_features.reserve(data->features_size());
	for (int i = 0; i < data->features_size(); i++)
		_features.append(Feature(&(data->features(i)), this));
	qSort(_features.begin(), _features.end());
}

PBF::PBF(const vector_tile::Tile &tile)
{
	for (int i = 0; i <  tile.layers_size(); i++) {
		const vector_tile::Tile_Layer &layer = tile.layers(i);
		_layers.insert(QString::fromStdString(layer.name()), new Layer(&layer));
	}
}

PBF::~PBF()
{
	for (QHash<QString, Layer*>::iterator it = _layers.begin();
	  it != _layers.end(); it++)
		delete *it;
}
