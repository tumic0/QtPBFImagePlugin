#ifndef FONT_H
#define FONT_H

#include <QFont>

class QJsonArray;

namespace Font
{
	QFont fromJsonArray(const QJsonArray &json);
}

#endif // FONT_H
