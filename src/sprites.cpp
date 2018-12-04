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


	if (json.contains("pixelRatio") && json["pixelRatio"].isDouble())
		_pixelRatio = json["pixelRatio"].toDouble();
	else
		_pixelRatio = 1.0;
}

bool Sprites::load(const QString &jsonFile, const QString &imageFile)
{
	_imageFile = imageFile;

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
				_sprites.insert(it.key(), s);
			else
				qWarning() << it.key() << ": invalid sprite definition";
		} else
			qWarning() << it.key() << ": invalid sprite definition";
	}

	return true;
}

QImage Sprites::icon(const QString &name) const
{
	const QImage *img = atlas(_imageFile);
	if (img->isNull())
		return QImage();

	QMap<QString, Sprite>::const_iterator it = _sprites.find(name);
	if (it == _sprites.constEnd())
		return QImage();

	if (!img->rect().contains(it->rect()))
		return QImage();

	QImage ret(img->copy(it->rect()));
	ret.setDevicePixelRatio(it->pixelRatio());

	return ret;
}
