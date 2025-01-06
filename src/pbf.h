#ifndef PBF_H
#define PBF_H

#include <QVariant>
#include <QVector>
#include <QHash>
#include <QPainterPath>
#include "data.h"


typedef QHash<QByteArray, quint32> KeyHash;

class PBF
{
public:
	class Layer;

	class Feature
	{
	public:
		Feature() : _data(0), _layer(0) {}
		Feature(const Data::Feature *data, const Layer *layer)
		  : _data(data), _layer(layer) {}

		const QVariant *value(const QByteArray &key) const;
		Data::GeomType type() const {return _data->type;}
		QPainterPath path(const QSizeF &factor) const;

		friend bool operator<(const Feature &f1, const Feature &f2);

	private:
		const Data::Feature *_data;
		const Layer *_layer;
	};

	class Layer
	{
	public:

		Layer(const Data::Layer *layer);

		const QVector<Feature> &features() const {return _features;}
		const QVector<QVariant> &values() const {return _data->values;}
		const KeyHash &keys() const {return _keys;}
		const Data::Layer *data() const {return _data;}

	private:
		const Data::Layer *_data;
		QVector<Feature> _features;
		KeyHash _keys;
	};


	PBF(const Data &data);
	~PBF();

	const QHash<QByteArray, Layer*> &layers() const {return _layers;}

private:
	QHash<QByteArray, Layer*> _layers;
};

inline bool operator<(const PBF::Feature &f1, const PBF::Feature &f2)
  {return f1._data->id < f2._data->id;}

#endif // PBF_H
