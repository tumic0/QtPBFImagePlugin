#ifndef PBFHANDLER_H
#define PBFHANDLER_H

#include <QImageIOHandler>
#include <QImage>
#include <QVariant>

class Style;

class PBFHandler : public QImageIOHandler
{
public:
	PBFHandler(Style *style) : _style(style) {}
	~PBFHandler() {}

	bool canRead() const;
	bool read(QImage *image);

	QByteArray name() const;
	QVariant option(ImageOption option) const;
	bool supportsOption(ImageOption option) const;

	static bool canRead(QIODevice *device);

private:
	Style *_style;
};

#endif // PBFHANDLER_H
