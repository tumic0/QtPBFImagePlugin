#ifndef SPRITES_H
#define SPRITES_H

#include <QRect>
#include <QMap>
#include <QImage>
#include <QString>
#include <QMutex>

class QJsonObject;

class Sprites
{
public:
	Sprites() : _init(0) {}
	bool load(const QString &jsonFile, const QString &imageFile);

	bool isNull() const {return _imageFile.isNull();}
	QImage icon(const QString &name, const QColor &color = Qt::black,
	  qreal size = 1.0);

private:
	class Sprite {
	public:
		Sprite(const QJsonObject &json);

		const QRect &rect() const {return _rect;}
		qreal pixelRatio() const {return _pixelRatio;}
		bool sdf() const {return _sdf;}

	private:
		QRect _rect;
		qreal _pixelRatio;
		bool _sdf;
	};

	QImage sprite(const Sprite &sprite, const QColor &color, qreal scale);

	QMap<QString, Sprite> _sprites;
	QImage _img;
	QMutex _lock;
	QString _imageFile;
	int _init;
};

#endif // SPRITES_H
