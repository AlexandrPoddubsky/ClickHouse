#pragma once

#include <DB/Core/Field.h>
#include <Poco/Timespan.h>


namespace DB
{


/** Одна настройка какого-либо типа.
  * Хранит внутри себя значение, а также флаг - было ли значение изменено.
  * Это сделано, чтобы можно было отправлять на удалённые серверы только изменённые (или явно указанные в конфиге) значения.
  * То есть, если настройка не была указана в конфиге и не была изменена динамически, то она не отправляется на удалённый сервер,
  *  и удалённый сервер будет использовать своё значение по-умолчанию.
  */


struct SettingUInt64
{
	UInt64 value;
	bool changed = false;

	SettingUInt64(UInt64 x = 0) : value(x) {}

	operator UInt64() const { return value; }
	SettingUInt64 & operator= (UInt64 x) { set(x); return *this; }

	void set(UInt64 x)
	{
		value = x;
		changed = true;
	}

	void set(const Field & x)
	{
		set(safeGet<UInt64>(x));
	}

	void set(const String & x)
	{
		set(parse<UInt64>(x));
	}

	void set(ReadBuffer & buf)
	{
		UInt64 x = 0;
		readVarUInt(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeVarUInt(value, buf);
	}
};

typedef SettingUInt64 SettingBool;


struct SettingSeconds
{
	Poco::Timespan value;
	bool changed = false;

	SettingSeconds(UInt64 seconds = 0) : value(seconds, 0) {}

	operator Poco::Timespan() const { return value; }
	SettingSeconds & operator= (Poco::Timespan x) { set(x); return *this; }

	Poco::Timespan::TimeDiff totalSeconds() const { return value.totalSeconds(); }

	void set(Poco::Timespan x)
	{
		value = x;
		changed = true;
	}

	void set(UInt64 x)
	{
		set(Poco::Timespan(x, 0));
	}

	void set(const Field & x)
	{
		set(safeGet<UInt64>(x));
	}

	void set(const String & x)
	{
		set(parse<UInt64>(x));
	}

	void set(ReadBuffer & buf)
	{
		UInt64 x = 0;
		readVarUInt(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeVarUInt(value.totalSeconds(), buf);
	}
};


struct SettingMilliseconds
{
	Poco::Timespan value;
	bool changed = false;

	SettingMilliseconds(UInt64 milliseconds = 0) : value(milliseconds * 1000) {}

	operator Poco::Timespan() const { return value; }
	SettingMilliseconds & operator= (Poco::Timespan x) { set(x); return *this; }

	Poco::Timespan::TimeDiff totalMilliseconds() const { return value.totalMilliseconds(); }

	void set(Poco::Timespan x)
	{
		value = x;
		changed = true;
	}

	void set(UInt64 x)
	{
		set(Poco::Timespan(x * 1000));
	}

	void set(const Field & x)
	{
		set(safeGet<UInt64>(x));
	}

	void set(const String & x)
	{
		set(parse<UInt64>(x));
	}

	void set(ReadBuffer & buf)
	{
		UInt64 x = 0;
		readVarUInt(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeVarUInt(value.totalMilliseconds(), buf);
	}
};


struct SettingFloat
{
	float value;
	bool changed = false;

	SettingFloat(float x = 0) : value(x) {}

	operator float() const { return value; }
	SettingFloat & operator= (float x) { set(x); return *this; }

	void set(float x)
	{
		value = x;
		changed = true;
	}

	void set(const Field & x)
	{
		if (x.getType() == Field::Types::UInt64)
		{
			set(safeGet<UInt64>(x));
		}
		else if (x.getType() == Field::Types::Int64)
		{
			set(safeGet<Int64>(x));
		}
		else if (x.getType() == Field::Types::Float64)
		{
			set(safeGet<Float64>(x));
		}
		else
			throw Exception(std::string("Bad type of setting. Expected UInt64, Int64 or Float64, got ") + x.getTypeName(), ErrorCodes::TYPE_MISMATCH);
	}

	void set(const String & x)
	{
		set(parse<float>(x));
	}

	void set(ReadBuffer & buf)
	{
		String x;
		readBinary(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeBinary(toString(value), buf);
	}
};


enum class LoadBalancing
{
	/// среди реплик с минимальным количеством ошибок выбирается случайная
	RANDOM = 0,
	/// среди реплик с минимальным количеством ошибок выбирается реплика
	/// с минимальным количеством отличающихся символов в имени реплики и имени локального хоста
	NEAREST_HOSTNAME
};

struct SettingLoadBalancing
{
	LoadBalancing value;
	bool changed = false;

	SettingLoadBalancing(LoadBalancing x) : value(x) {}

	operator LoadBalancing() const { return value; }
	SettingLoadBalancing & operator= (LoadBalancing x) { set(x); return *this; }

	static LoadBalancing getLoadBalancing(const String & s)
	{
		if (s == "random") 				return LoadBalancing::RANDOM;
		if (s == "nearest_hostname") 	return LoadBalancing::NEAREST_HOSTNAME;

		throw Exception("Unknown load balancing mode: '" + s + "', must be one of 'random', 'nearest_hostname'", ErrorCodes::UNKNOWN_LOAD_BALANCING);
	}

	String toString() const
	{
		const char * strings[] = {"random", "nearest_hostname"};
		if (value < LoadBalancing::RANDOM || value > LoadBalancing::NEAREST_HOSTNAME)
			throw Exception("Unknown load balancing mode", ErrorCodes::UNKNOWN_OVERFLOW_MODE);
		return strings[static_cast<size_t>(value)];
	}

	void set(LoadBalancing x)
	{
		value = x;
		changed = true;
	}

	void set(const Field & x)
	{
		set(safeGet<const String &>(x));
	}

	void set(const String & x)
	{
		set(getLoadBalancing(x));
	}

	void set(ReadBuffer & buf)
	{
		String x;
		readBinary(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeBinary(toString(), buf);
	}
};


/// Какие строки включать в TOTALS.
enum class TotalsMode
{
	BEFORE_HAVING			= 0, /// Считать HAVING по всем прочитанным строкам;
								 ///  включая не попавшие в max_rows_to_group_by
								 ///  и не прошедшие HAVING после группировки.
	AFTER_HAVING_INCLUSIVE	= 1, /// Считать по всем строкам, кроме не прошедших HAVING;
								 ///  то есть, включать в TOTALS все строки, не прошедшие max_rows_to_group_by.
	AFTER_HAVING_EXCLUSIVE	= 2, /// Включать только строки, прошедшие и max_rows_to_group_by, и HAVING.
	AFTER_HAVING_AUTO		= 3, /// Автоматически выбирать между INCLUSIVE и EXCLUSIVE,
};

struct SettingTotalsMode
{
	TotalsMode value;
	bool changed = false;

	SettingTotalsMode(TotalsMode x) : value(x) {}

	operator TotalsMode() const { return value; }
	SettingTotalsMode & operator= (TotalsMode x) { set(x); return *this; }

	static TotalsMode getTotalsMode(const String & s)
	{
		if (s == "before_having") 			return TotalsMode::BEFORE_HAVING;
		if (s == "after_having_exclusive")	return TotalsMode::AFTER_HAVING_EXCLUSIVE;
		if (s == "after_having_inclusive")	return TotalsMode::AFTER_HAVING_INCLUSIVE;
		if (s == "after_having_auto")		return TotalsMode::AFTER_HAVING_AUTO;

		throw Exception("Unknown totals mode: '" + s + "', must be one of 'before_having', 'after_having_exclusive', 'after_having_inclusive', 'after_having_auto'", ErrorCodes::UNKNOWN_TOTALS_MODE);
	}

	String toString() const
	{
		switch (value)
		{
			case TotalsMode::BEFORE_HAVING:				return "before_having";
			case TotalsMode::AFTER_HAVING_EXCLUSIVE:	return "after_having_exclusive";
			case TotalsMode::AFTER_HAVING_INCLUSIVE:	return "after_having_inclusive";
			case TotalsMode::AFTER_HAVING_AUTO:			return "after_having_auto";

			default:
				throw Exception("Unknown TotalsMode enum value: " + toString(value), ErrorCodes::ARGUMENT_OUT_OF_BOUND);
		}
	}

	void set(TotalsMode x)
	{
		value = x;
		changed = true;
	}

	void set(const Field & x)
	{
		set(safeGet<const String &>(x));
	}

	void set(const String & x)
	{
		set(getTotalsMode(x));
	}

	void set(ReadBuffer & buf)
	{
		String x;
		readBinary(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeBinary(toString(), buf);
	}
};

/// Что делать, если ограничение превышено.
enum class OverflowMode
{
	THROW 	= 0,	/// Кинуть исключение.
	BREAK 	= 1,	/// Прервать выполнение запроса, вернуть что есть.
	ANY		= 2,	/** Только для GROUP BY: не добавлять новые строки в набор,
						* но продолжать агрегировать для ключей, успевших попасть в набор.
						*/
};

template <bool enable_mode_any>
struct SettingOverflowMode
{
	OverflowMode value;
	bool changed = false;

	SettingOverflowMode(OverflowMode x = OverflowMode::THROW) : value(x) {}

	operator OverflowMode() const { return value; }
	SettingOverflowMode & operator= (OverflowMode x) { set(x); return *this; }

	static OverflowMode getOverflowModeForGroupBy(const String & s)
	{
		if (s == "throw") 	return OverflowMode::THROW;
		if (s == "break") 	return OverflowMode::BREAK;
		if (s == "any")		return OverflowMode::ANY;

		throw Exception("Unknown overflow mode: '" + s + "', must be one of 'throw', 'break', 'any'", ErrorCodes::UNKNOWN_OVERFLOW_MODE);
	}

	static OverflowMode getOverflowMode(const String & s)
	{
		OverflowMode mode = getOverflowModeForGroupBy(s);

		if (mode == OverflowMode::ANY && !enable_mode_any)
			throw Exception("Illegal overflow mode: 'any' is only for 'group_by_overflow_mode'", ErrorCodes::ILLEGAL_OVERFLOW_MODE);

		return mode;
	}

	String toString() const
	{
		const char * strings[] = { "throw", "break", "any" };

		if (value < OverflowMode::THROW || value > OverflowMode::ANY)
			throw Exception("Unknown overflow mode", ErrorCodes::UNKNOWN_OVERFLOW_MODE);

		return strings[static_cast<size_t>(value)];
	}

	void set(OverflowMode x)
	{
		value = x;
		changed = true;
	}

	void set(const Field & x)
	{
		set(safeGet<const String &>(x));
	}

	void set(const String & x)
	{
		set(getOverflowMode(x));
	}

	void set(ReadBuffer & buf)
	{
		String x;
		readBinary(x, buf);
		set(x);
	}

	void write(WriteBuffer & buf) const
	{
		writeBinary(toString(), buf);
	}
};


}