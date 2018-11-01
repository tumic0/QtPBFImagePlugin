#include <QStandardPaths>
#include "pbfplugin.h"
#include "pbfhandler.h"
#include "style.h"


PBFPlugin::PBFPlugin()
{
	_style = new Style(this);

	QString style(QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  "style/style.json"));

	if (style.isEmpty() || !_style->load(style)) {
		Q_INIT_RESOURCE(pbfplugin);
		_style->load(":/style/style.json");
	}
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
