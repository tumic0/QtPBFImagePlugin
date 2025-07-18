#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>
#include "sprites.h"

static QImage sdf2img(const QImage &sdf, const QColor &color)
{
	QImage img(sdf.convertToFormat(QImage::Format_ARGB32));
	quint32 argb = color.rgba();
	uchar *bits = img.bits();
	int bpl = img.bytesPerLine();

	for (int y = 0; y < img.height(); y++) {
		for (int x = 0; x < img.width(); x++) {
			quint32 *pixel =  (quint32*)(bits + y * bpl + x * 4);
			*pixel = ((*pixel >> 24) < 192) ? 0 : argb;
		}
	}

	return img;
}

Sprites::Sprite::Sprite(const QJsonObject &json)
  : _pixelRatio(1.0), _sdf(false)
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
	if (json.contains("sdf") && json["sdf"].isBool())
		_sdf = json["sdf"].toBool();
}

bool Sprites::load(const QString &jsonFile, const QString &imageFile)
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
				_sprites.insert(it.key(), s);
			else
				qWarning() << it.key() << ": invalid sprite definition";
		} else
			qWarning() << it.key() << ": invalid sprite definition";
	}

	// Loading the sprites atlas image must be deferred until all image plugins
	// are loaded, otherwise reading the image will cause a deadlock in Qt!
	_imageFile = imageFile;

	return true;
}

QImage Sprites::sprite(const Sprite &sprite, const QColor &color, qreal scale)
{
	_lock.lock();
	if (_init <= 0) {
		if (_init < 0) {
			_lock.unlock();
			return QImage();
		}

		_img = QImage(_imageFile);
		if (_img.isNull()) {
			qWarning() << _imageFile << ": invalid sprite atlas image";
			_init = -1;
			_lock.unlock();
			return QImage();
		}

		_init = 1;
	}
	_lock.unlock();

	if (!_img.rect().contains(sprite.rect()))
		return QImage();

	QImage img(_img.copy(sprite.rect()));
	img.setDevicePixelRatio(sprite.pixelRatio());

	if (sprite.sdf()) {
		if (scale != 1.0) {
			QSize size(img.size().width() * scale, img.size().height() * scale);
			QImage simg(img.scaled(size, Qt::IgnoreAspectRatio,
			  Qt::SmoothTransformation));
			return sdf2img(simg, color);
		} else
			return sdf2img(img, color);
	} else {
		if (scale != 1.0) {
			QSize size(img.size().width() * scale, img.size().height() * scale);
			QImage simg(img.scaled(size, Qt::IgnoreAspectRatio,
			  Qt::SmoothTransformation));
			return simg;
		} else
			return img;
	}
}

QImage Sprites::icon(const QString &name, const QColor &color, qreal size)
{
	if (name.isNull())
		return QImage();

	QMap<QString, Sprite>::const_iterator it = _sprites.constFind(name);
	if (it == _sprites.constEnd())
		return QImage();

	return sprite(*it, color, size);
}
