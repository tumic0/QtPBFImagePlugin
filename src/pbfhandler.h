#ifndef PBFHANDLER_H
#define PBFHANDLER_H

#include <QImageIOHandler>
#include <QVariant>
#include <QSize>

class QImage;
class Style;

class PBFHandler : public QImageIOHandler
{
public:
	PBFHandler(const QList<Style*> &styles) : _styles(styles) {}
	~PBFHandler() {}

	bool canRead() const;
	bool read(QImage *image);

	QVariant option(ImageOption option) const;
	bool supportsOption(ImageOption option) const;
	void setOption(ImageOption option, const QVariant &value);

	static bool canRead(QIODevice *device);

private:
	QString description() const;

	const QList<Style*> _styles;
	QSize _scaledSize;
};

#endif // PBFHANDLER_H
