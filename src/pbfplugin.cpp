#include <QDir>
#include <QDebug>
#include "pbfplugin.h"
#include "pbfhandler.h"
#include "style.h"


#define GLOBAL_CONFIG "/usr/share/pbf/style.json"
#define USER_CONFIG   QDir::homePath() + "/.pbf/style.json"

PBFPlugin::PBFPlugin()
{
	_style = new Style(this);

	if (!_style->load(USER_CONFIG))
		if (!_style->load(GLOBAL_CONFIG))
			qCritical() << "Map style not found";
}

QImageIOPlugin::Capabilities PBFPlugin::capabilities(QIODevice *device,
  const QByteArray &format) const
{
	if (device == 0)
		return (format == "pbf") ? Capabilities(CanRead) : Capabilities();
	else
		return (device->isReadable() && PBFHandler::canRead(device))
		  ? Capabilities(CanRead) : Capabilities();
}

QImageIOHandler *PBFPlugin::create(QIODevice *device,
  const QByteArray &format) const
{
	QImageIOHandler *handler = new PBFHandler(_style);
	handler->setDevice(device);
	handler->setFormat(format);
	return handler;
}
