#include <QImageIOPlugin>
#include "style.h"

class Style;

class PBFPlugin : public QImageIOPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QImageIOHandlerFactoryInterface"
	  FILE "pbfplugin.json")

public:
	PBFPlugin();
	~PBFPlugin() {}

	Capabilities capabilities(QIODevice *device, const QByteArray &format) const;
	QImageIOHandler *create(QIODevice *device,
	  const QByteArray &format = QByteArray()) const;

private:
	Style *_style;
};
