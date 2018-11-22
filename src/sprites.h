#ifndef SPRITES_H
#define SPRITES_H

#include <QRect>
#include <QMap>
#include <QImage>

class Sprites
{
public:
	bool load(const QString &jsonFile, const QString &imageFile);

	QImage icon(const QString &name) const;

private:
	class Sprite {
	public:
		Sprite(const QJsonObject &json);
		const QRect &rect() const {return _rect;}

	private:
		QRect _rect;
	};

	QMap<QString, Sprite> _sprites;
	QString _imageFile;
};

#endif // SPRITES_H
