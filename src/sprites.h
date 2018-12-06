#ifndef SPRITES_H
#define SPRITES_H

#include <QRect>
#include <QMap>
#include <QImage>
#include <QString>

class QJsonObject;

class Sprites
{
public:
	bool load(const QString &jsonFile, const QString &imageFile);

	bool isNull() const {return _imageFile.isNull();}
	QImage icon(const QString &name) const;

private:
	class Sprite {
	public:
		Sprite(const QJsonObject &json);

		const QRect &rect() const {return _rect;}
		qreal pixelRatio() const {return _pixelRatio;}

	private:
		QRect _rect;
		qreal _pixelRatio;
	};

	QMap<QString, Sprite> _sprites;
	QString _imageFile;
};

#endif // SPRITES_H
