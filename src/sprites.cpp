#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include "sprites.h"


/*
	Loading the sprites atlas image must be deferred until all image plugins
	are loaded, otherwise reading the image will cause a deadlock!
*/
static const QImage *atlas(const QString &fileName)
{
	static QImage *img = new QImage(fileName);
	return img;
}

static const QImage *atlas2x(const QString &fileName)
{
	static QImage *img = new QImage(fileName);
	return img;
}


Sprites::Sprite::Sprite(const QJsonObject &json)
{
	int x, y, width, height;

	if (json.contains("x") && json["x"].isDouble())
		x = json["x"].toInt();
	else
		return;
	if (json.contains("y") && json["y"].isDouble())
		y = json["y"].toInt();
	else
		return;
	if (json.contains("width") && json["width"].isDouble())
		width = json["width"].toInt();
	else
		return;
	if (json.contains("height") && json["height"].isDouble())
		height = json["height"].toInt();
	else
		return;

	_rect = QRect(x, y, width, height);
}

bool Sprites::load(const QString &jsonFile, const QString &imageFile)
{
	_imageFile = imageFile;
	return load(jsonFile, _sprites);
}

bool Sprites::load2x(const QString &jsonFile, const QString &imageFile)
{
	_image2xFile = imageFile;
	return load(jsonFile, _sprites2x);
}

bool Sprites::load(const QString &jsonFile, QMap<QString, Sprite> &map)
{
	QFile file(jsonFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qCritical() << jsonFile << ": error opening file";
		return false;
	}
	QByteArray ba(file.readAll());

	QJsonParseError error;
	QJsonDocument doc(QJsonDocument::fromJson(ba, &error));
	if (doc.isNull()) {
		qCritical() << jsonFile << ":" << error.errorString();
		return false;
	}

	QJsonObject json(doc.object());
	for (QJsonObject::const_iterator it = json.constBegin();
	  it != json.constEnd(); it++) {
		QJsonValue val(*it);
		if (val.isObject()) {
			Sprite s(val.toObject());
			if (s.rect().isValid())
				map.insert(it.key(), s);
			else
				qWarning() << it.key() << ": invalid sprite definition";
		} else
			qWarning() << it.key() << ": invalid sprite definition";
	}

	return true;
}

QImage Sprites::icon(const QString &name, bool hidpi) const
{
	qreal ratio;
	const QImage *img;
	const QMap<QString, Sprite> *map;

	if (hidpi && !_image2xFile.isNull()) {
		img = atlas2x(_image2xFile);
		map = &_sprites2x;
		ratio = 2;
	} else if (!_imageFile.isNull()) {
		img = atlas(_imageFile);
		map = &_sprites;
		ratio = 1;
	} else
		return QImage();

	if (img->isNull())
		return QImage();


	QMap<QString, Sprite>::const_iterator it = map->find(name);
	if (it == map->constEnd())
		return QImage();

	if (!img->rect().contains(it->rect()))
		return QImage();

	QImage ret(img->copy(it->rect()));
	ret.setDevicePixelRatio(ratio);

	return ret;
}
