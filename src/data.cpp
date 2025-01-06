#include "data.h"

#define TYPE(tag) (tag & 0x07)
#define FIELD(tag) (tag >> 3)

struct CTX
{
	CTX(const QByteArray &ba)
	  : bp(ba.constData()), be(bp + ba.size()) {}

	const char *bp;
	const char *be;
	quint64 tag;
};

static inline qint64 zigzag64decode(quint64 value)
{
	return static_cast<qint64>((value >> 1u) ^ static_cast<quint64>(
	  -static_cast<qint64>(value & 1u)));
}

template<typename T>
static bool varint(CTX &ctx, T &val)
{
	val = 0;
	uint shift = 0;
	const char *end = qMin(ctx.be, ctx.bp + sizeof(val));

	while (ctx.bp < end) {
		val |= ((quint8)*ctx.bp & 0x7F) << shift;
		shift += 7;
		if (!((quint8)*ctx.bp++ & 0x80))
			return true;
	}

	return false;
}

static bool str(CTX &ctx, QByteArray &val)
{
	quint64 len;

	if (!varint(ctx, len))
		return false;
	if (ctx.bp + len > ctx.be)
		return false;
	val = QByteArray::fromRawData(ctx.bp, len);
	ctx.bp += len;

	return true;
}

static bool dbl(CTX &ctx, double &val)
{
	if (ctx.bp + sizeof(val) > ctx.be)
		return false;

	memcpy(&val, ctx.bp, sizeof(val));

	return true;
}

static bool flt(CTX &ctx, float &val)
{
	if (ctx.bp + sizeof(val) > ctx.be)
		return false;

	memcpy(&val, ctx.bp, sizeof(val));

	return true;
}

static bool packed(CTX &ctx, QVector<quint32> &vals)
{
	quint32 v;

	if (TYPE(ctx.tag) == 2) {
		quint64 len;
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
	} else if (TYPE(ctx.tag) == 0) {
		if (!varint(ctx, v))
			return false;
		vals.append(v);
		return true;
	} else
		return false;
}

static bool skip(CTX &ctx)
{
	quint64 len = 0;

	switch (TYPE(ctx.tag)) {
		case 0:
			return varint(ctx, len);
		case 1:
			len = 8;
			break;
		case 2:
			if (!varint(ctx, len))
				return false;
			break;
		case 5:
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
	quint64 len, num;
	double dnum;
	float fnum;

	if (!varint(ctx, len))
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
				if (!varint(ctx, num))
					return false;
				val = QVariant(static_cast<qint64>(num));
				break;
			case 5:
				if (!varint(ctx, num))
					return false;
				val = QVariant(num);
				break;
			case 6:
				if (!varint(ctx, num))
					return false;
				val = QVariant(zigzag64decode(num));
				break;
			case 7:
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
	quint64 len;
	quint8 e;
	if (!varint(ctx, len))
		return false;

	const char *ee = ctx.bp + len;
	if (ee > ctx.be)
		return false;

	while (ctx.bp < ee) {
		if (!varint(ctx, ctx.tag))
			return false;

		switch (FIELD(ctx.tag)) {
			case 1:
				if (!varint(ctx, f.id))
					return false;
				break;
			case 2:
				if (!packed(ctx, f.tags))
					return false;
				break;
			case 3:
				if (!varint(ctx, e))
					return false;
				if (e > Data::GeomType::POLYGON)
					return false;
				f.type = (Data::GeomType)e;
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
	if (ctx.tag == 0x1a) {
		quint64 len;
		if (!varint(ctx, len))
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
					if (!varint(ctx, l.extent))
						return false;
					break;
				case 15:
					if (!varint(ctx, l.version))
						return false;
					break;
				default:
					if (!skip(ctx))
						return false;
			}
		}

		return (ctx.bp == ee);
	} else
		return skip(ctx);
}

bool Data::load(const QByteArray &ba)
{
	CTX ctx(ba);

	while (ctx.bp < ctx.be) {
		if (!varint(ctx, ctx.tag))
			return false;
		_layers.append(Layer());
		if (!layer(ctx, _layers.last()))
			return false;
	}

	return (ctx.bp == ctx.be);
}
