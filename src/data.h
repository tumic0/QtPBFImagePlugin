#ifndef DATA_H
#define DATA_H

#include <QByteArray>
#include <QVector>
#include <QVariant>

class Data
{
public:
	enum GeomType {
		UNKNOWN = 0,
		POINT = 1,
		LINESTRING = 2,
		POLYGON = 3
	};

	struct Feature
	{
		quint64 id;
		QVector<quint32> tags;
		GeomType type;
		QVector<quint32> geometry;
	};

	struct Layer
	{
		quint32 version;
		QByteArray name;
		QVector<Feature> features;
		QVector<QByteArray> keys;
		QVector<QVariant> values;
		quint32 extent;
	};

	bool load(const QByteArray &ba);
	const QVector<Layer> &layers() const {return _layers;}

private:
	QVector<Layer> _layers;
};

#endif // DATA_H
