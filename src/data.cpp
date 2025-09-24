#include "data.h"

#define TYPE(tag) (tag & 0x07)
#define FIELD(tag) (tag >> 3)

#define VARINT 0
#define I64    1
#define LEN    2
#define I32    5

struct CTX
{
	CTX(const QByteArray &ba)
	  : bp(ba.constData()), be(bp + ba.size()), tag(0) {}

	const char *bp;
	const char *be;
	quint32 tag;
};

static inline qint64 zigzag64decode(quint64 value)
{
	return static_cast<qint64>((value >> 1u) ^ static_cast<quint64>(
	  -static_cast<qint64>(value & 1u)));
}

template<typename T>
static bool varint(CTX &ctx, T &val)
{
	unsigned int shift = 0;
	val = 0;

	while (ctx.bp < ctx.be) {
		val |= static_cast<T>((quint8)*ctx.bp & 0x7F) << shift;
		shift += 7;
		if (!((quint8)*ctx.bp++ & 0x80))
			return true;
	}

	return false;
}

static bool length(CTX &ctx, quint32 &val)
{
	if (TYPE(ctx.tag) != LEN)
		return false;

	if (!varint(ctx, val))
		return false;

	return true;
}

static bool str(CTX &ctx, QByteArray &val)
{
	quint32 len;

	if (!length(ctx, len))
		return false;
	if (ctx.bp + len > ctx.be)
		return false;

/* In Qt5 the (later) conversion to QString is broken when the QByteArray is
   not nul terminated so we have to use the "deep copy" constructor that
   nul-terminates the byte array when it is created. */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	val = QByteArray(ctx.bp, len);
#else
	val = QByteArray::fromRawData(ctx.bp, len);
#endif
	ctx.bp += len;

	return true;
}

static bool dbl(CTX &ctx, double &val)
{
	if (TYPE(ctx.tag) != I64)
		return false;
	if (ctx.bp + sizeof(val) > ctx.be)
		return false;

	memcpy(&val, ctx.bp, sizeof(val));
	ctx.bp += sizeof(val);

	return true;
}

static bool flt(CTX &ctx, float &val)
{
	if (TYPE(ctx.tag) != I32)
		return false;
	if (ctx.bp + sizeof(val) > ctx.be)
		return false;

	memcpy(&val, ctx.bp, sizeof(val));
	ctx.bp += sizeof(val);

	return true;
}

static bool packed(CTX &ctx, QVector<quint32> &vals)
{
	quint32 v;

	if (TYPE(ctx.tag) == LEN) {
		quint32 len;
		if (!varint(ctx, len))
			return false;
		const char *ee = ctx.bp + len;
		if (ee > ctx.be)
			return false;
		while (ctx.bp < ee) {
			if (!varint(ctx, v))
				return false;
			vals.append(v);
		}
		return (ctx.bp == ee);
	} else if (TYPE(ctx.tag) == VARINT) {
		if (!varint(ctx, v))
			return false;
		vals.append(v);
		return true;
	} else
		return false;
}

static bool skip(CTX &ctx)
{
	quint32 len = 0;

	switch (TYPE(ctx.tag)) {
		case VARINT:
			return varint(ctx, len);
		case I64:
			len = 8;
			break;
		case LEN:
			if (!varint(ctx, len))
				return false;
			break;
		case I32:
			len = 4;
			break;
		default:
			return false;
	}

	if (ctx.bp + len > ctx.be)
		return false;
	ctx.bp += len;

	return true;
}

static bool value(CTX &ctx, QVariant &val)
{
	QByteArray ba;
	quint64 num;
	double dnum;
	float fnum;
	quint32 len;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (FIELD(ctx.tag)) {
			case 1:
				if (!str(ctx, ba))
					return false;
				val = QVariant(ba);
				break;
			case 2:
				if (!flt(ctx, fnum))
					return false;
				val = QVariant(fnum);
				break;
			case 3:
				if (!dbl(ctx, dnum))
					return false;
				val = QVariant(dnum);
				break;
			case 4:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(static_cast<qint64>(num));
				break;
			case 5:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(num);
				break;
			case 6:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(zigzag64decode(num));
				break;
			case 7:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, num))
					return false;
				val = QVariant(num ? true : false);
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ee);
}

static bool feature(CTX &ctx, Data::Feature &f)
{
	quint32 len;
	quint32 e;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (FIELD(ctx.tag)) {
			case 1:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, f.id))
					return false;
				break;
			case 2:
				if (!packed(ctx, f.tags))
					return false;
				break;
			case 3:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, e))
					return false;
				if (e > Data::GeomType::POLYGON)
					return false;
				f.type = static_cast<Data::GeomType>(e);
				break;
			case 4:
				if (!packed(ctx, f.geometry))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ee);
}

static bool layer(CTX &ctx, Data::Layer &l)
{
	quint32 len;

	if (!length(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (FIELD(ctx.tag)) {
			case 1:
				if (!str(ctx, l.name))
					return false;
				break;
			case 2:
				l.features.append(Data::Feature());
				if (!feature(ctx, l.features.last()))
					return false;
				break;
			case 3:
				l.keys.append(QByteArray());
				if (!str(ctx, l.keys.last()))
					return false;
				break;
			case 4:
				l.values.append(QVariant());
				if (!value(ctx, l.values.last()))
					return false;
				break;
			case 5:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, l.extent))
					return false;
				break;
			case 15:
				if (TYPE(ctx.tag) != VARINT)
					return false;
				if (!varint(ctx, l.version))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ee);
}

bool Data::load(const QByteArray &ba)
{
	CTX ctx(ba);

	while (ctx.bp < ctx.be) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (FIELD(ctx.tag)) {
			case 3:
				_layers.append(Layer());
				if (!layer(ctx, _layers.last()))
					return false;
				break;
			default:
				if (!skip(ctx))
					return false;
		}
	}

	return (ctx.bp == ctx.be);
}
