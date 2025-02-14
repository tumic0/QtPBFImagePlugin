#include "pbf.h"

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

const QVariant *PBF::Feature::value(const QByteArray &key) const
{
	const KeyHash &keys(_layer->keys());
	KeyHash::const_iterator it(keys.find(key));
	if (it == keys.constEnd())
		return 0;

	quint32 index = *it;
	for (int i = 0; i < _data->tags.size(); i = i + 2)
		if (_data->tags.at(i) == index)
			return &(_layer->values().at(_data->tags.at(i+1)));

	return 0;
}

QPainterPath PBF::Feature::path(const QSizeF &factor) const
{
	QPoint cursor;
	QPainterPath path;

	for (int i = 0; i < _data->geometry.size(); i++) {
		quint32 g = _data->geometry.at(i);
		unsigned cmdId = g & 0x7;
		unsigned cmdCount = g >> 3;

		if (cmdId == MOVE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(_data->geometry.at(i+1),
				  _data->geometry.at(i+2));
				i += 2;
				cursor += offset;
				path.moveTo(QPointF(cursor.x() * factor.width(),
				  cursor.y() * factor.height()));
			}
		} else if (cmdId == LINE_TO) {
			for (unsigned j = 0; j < cmdCount; j++) {
				QPoint offset = parameters(_data->geometry.at(i+1),
				  _data->geometry.at(i+2));
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

PBF::Layer::Layer(const Data::Layer *layer) : _data(layer)
{
	_keys.reserve(layer->keys.size());
	for (int i = 0; i < layer->keys.size(); i++)
		_keys.insert(layer->keys.at(i), i);

	_features.reserve(layer->features.size());
	for (int i = 0; i < layer->features.size(); i++)
		_features.append(Feature(&(layer->features.at(i)), this));
	std::sort(_features.begin(), _features.end());
}

PBF::PBF(const Data &data)
{
	for (int i = 0; i <  data.layers().size(); i++) {
		const Data::Layer &layer = data.layers().at(i);
		_layers.insert(layer.name, new Layer(&layer));
	}
}

PBF::~PBF()
{
	qDeleteAll(_layers);
}
