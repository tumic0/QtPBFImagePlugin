#ifndef SPRITES_H
#define SPRITES_H

#include <QRect>
#include <QMap>
#include <QImage>

class Sprites
{
public:
	bool load(const QString &jsonFile, const QString &imageFile);
	bool load2x(const QString &jsonFile, const QString &imageFile);

	QImage icon(const QString &name, bool hidpi) const;

private:
	class Sprite {
	public:
		Sprite(const QJsonObject &json);
		const QRect &rect() const {return _rect;}

	private:
		QRect _rect;
	};

	bool load(const QString &jsonFile, QMap<QString, Sprite> &map);

	QMap<QString, Sprite> _sprites, _sprites2x;
	QString _imageFile, _image2xFile;
};

#endif // SPRITES_H
