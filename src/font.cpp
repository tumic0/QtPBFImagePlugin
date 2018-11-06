#include <QString>
#include <QFontDatabase>
#include <QJsonArray>
#include "font.h"


static const QStringList &fonts()
{
	static QStringList l(QFontDatabase().families());
	return l;
}

const QString fontFamily(const QString &str)
{
	const QStringList &fl = fonts();

	QString family(str.left(str.lastIndexOf(' ')));
	for (int j = 0; j < fl.size(); j++)
		if (fl.at(j).startsWith(family))
			return family;

	return QString();
}

static void setType(QFont &font, const QString &str)
{
	QString type(str.right(str.length() - str.lastIndexOf(' ') - 1));

	if (type == "Italic")
		font.setItalic(true);
	else if (type == "Bold")
		font.setWeight(QFont::Bold);
	else if (type == "Medium")
		font.setWeight(QFont::Medium);
}

QFont Font::fromJsonArray(const QJsonArray &json)
{
	if (json.isEmpty())
		return QFont();

	// Try exact match from the layout font list
	for (int i = 0; i < json.size(); i++) {
		if (!json.at(i).isString())
			return QFont();
		QString str(json.at(i).toString());
		QString family(fontFamily(str));
		if (!family.isNull()) {
			QFont font(family);
			setType(font, str);
			return font;
		}
	}

	// Use Qt's font matching logic on the first font in the list
	QString str(json.first().toString());
	QFont font(str.left(str.lastIndexOf(' ')));
	setType(font, str);

	return font;
}
