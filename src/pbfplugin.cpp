#include <QStandardPaths>
#include <QDir>
#include "pbfplugin.h"
#include "pbfhandler.h"
#include "style.h"


PBFPlugin::PBFPlugin()
{
	QString styleDir(QStandardPaths::locate(QStandardPaths::AppDataLocation,
	  "style", QStandardPaths::LocateDirectory));
	if (!styleDir.isEmpty())
		loadStyles(styleDir);

	if (_styles.isEmpty()) {
		Q_INIT_RESOURCE(pbfplugin);
		loadStyles(":/style");
	}
}

PBFPlugin::~PBFPlugin()
{
	qDeleteAll(_styles);
}

QImageIOPlugin::Capabilities PBFPlugin::capabilities(QIODevice *device,
  const QByteArray &format) const
{
	if (device == 0)
		return (format == "mvt") ? Capabilities(CanRead) : Capabilities();
	else
		return (device->isReadable() && PBFHandler::canRead(device))
		  ? Capabilities(CanRead) : Capabilities();
}

QImageIOHandler *PBFPlugin::create(QIODevice *device,
  const QByteArray &format) const
{
	QImageIOHandler *handler = new PBFHandler(_styles);
	handler->setDevice(device);
	handler->setFormat(format);
	return handler;
}

void PBFPlugin::loadStyles(const QString &path)
{
	QDir dir(path);
	QFileInfoList styles(dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot));

	for (int i = 0; i < styles.size(); i++) {
		QDir d(styles.at(i).absoluteFilePath());
		Style *style = new Style(d.filePath("style.json"));
		if (style->isValid())
			_styles.append(style);
		else
			delete style;
	}
}
