#ifndef PBF_H
#define PBF_H

#include <QVariant>
#include <QVector>
#include <QHash>
#include <QPainterPath>
#include "vector_tile.pb.h"


typedef QHash<QByteArray, google::protobuf::uint32> KeyHash;

class PBF
{
public:
	class Layer;

	class Feature
	{
	public:
		Feature() : _data(0), _layer(0) {}
		Feature(const vector_tile::Tile_Feature *data, const Layer *layer)
		  : _data(data), _layer(layer) {}

		const QVariant *value(const QByteArray &key) const;
		vector_tile::Tile_GeomType type() const {return _data->type();}
		QPainterPath path(const QSizeF &factor) const;

		friend bool operator<(const Feature &f1, const Feature &f2);

	private:
		const vector_tile::Tile_Feature *_data;
		const Layer *_layer;
	};

	class Layer
	{
	public:

		Layer(const vector_tile::Tile_Layer *data);

		const QVector<Feature> &features() const {return _features;}
		const QVector<QVariant> &values() const {return _values;}
		const KeyHash &keys() const {return _keys;}
		const vector_tile::Tile_Layer *data() const {return _data;}

	private:
		const vector_tile::Tile_Layer *_data;
		QVector<Feature> _features;
		QVector<QVariant> _values;
		KeyHash _keys;
	};


	PBF(const vector_tile::Tile &tile);
	~PBF();

	const QHash<QByteArray, Layer*> &layers() const {return _layers;}

private:
	QHash<QByteArray, Layer*> _layers;
};

inline bool operator<(const PBF::Feature &f1, const PBF::Feature &f2)
  {return f1._data->id() < f2._data->id();}

#endif // PBF_H
