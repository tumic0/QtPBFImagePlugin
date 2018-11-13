#ifndef PBFHANDLER_H
#define PBFHANDLER_H

#include <QImageIOHandler>
#include <QImage>
#include <QVariant>
#include <QSize>

class Style;

class PBFHandler : public QImageIOHandler
{
public:
	PBFHandler(Style *style) : _style(style) {}
	~PBFHandler() {}

	bool canRead() const;
	bool read(QImage *image);

	QVariant option(ImageOption option) const;
	bool supportsOption(ImageOption option) const;
	void setOption(QImageIOHandler::ImageOption option, const QVariant &value);

	static bool canRead(QIODevice *device);

private:
	Style *_style;
	QSize _scaledSize;
};

#endif // PBFHANDLER_H
