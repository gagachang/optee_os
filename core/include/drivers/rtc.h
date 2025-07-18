/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright 2022 Microchip.
 */

#ifndef __DRIVERS_RTC_H
#define __DRIVERS_RTC_H

#include <kernel/panic.h>
#include <tee_api_types.h>
#include <util.h>

/* The RTC allows to set/get offset for correction */
#define RTC_CORRECTION_FEATURE	BIT(0)
/* The RTC allows to set/read an alarm */
#define RTC_ALARM_FEATURE	BIT(1)
/* The RTC can wake-up the platform through alarm */
#define RTC_WAKEUP_ALARM	BIT(2)

#define MS_PER_SEC		1000
#define MS_PER_MIN		(60 * MS_PER_SEC)
#define MS_PER_HOUR		(60 * MS_PER_MIN)
#define MS_PER_DAY		(24 * MS_PER_HOUR)

#define RTC_TIME(year, mon, mday, wday, hour, min, sec, ms)		\
	{								\
		.tm_year = (year),					\
		.tm_mon = (mon),					\
		.tm_mday = (mday),					\
		.tm_wday = (wday),					\
		.tm_hour = (hour),					\
		.tm_min = (min),					\
		.tm_sec = (sec),					\
		.tm_ms = (ms),						\
	}

/*
 * struct optee_rtc_time - Time in Gregorian calendar
 *
 * @tm_year: Absolute year
 * @tm_mon: Month: 0=January, 1=February, ..., 11=December
 * @tm_mday: Month day start from 1
 * @tm_wday: Week day: 0=Sunday, 1=Monday, ..., 6=Saturday
 * @tm_hour: Hour in range [0 23]
 * @tm_min: Minute in range [0 59]
 * @tm_sec: Second in range [0 59]
 * @tm_ms: Millisecond in range [0 999] or 0 if no applicable
 */
struct optee_rtc_time {
	uint32_t tm_year;
	uint32_t tm_mon;
	uint32_t tm_mday;
	uint32_t tm_wday;
	uint32_t tm_hour;
	uint32_t tm_min;
	uint32_t tm_sec;
	uint32_t tm_ms;
};

struct rtc {
	const struct rtc_ops *ops;
	struct optee_rtc_time range_min;
	struct optee_rtc_time range_max;
	bool is_wakeup_source;
};

/*
 * struct optee_rtc_alarm - The RTC alarm
 * @enabled:		1 if the alarm is enabled, 0 otherwise
 * @pending:		1 if the alarm is pending, 0 otherwise
 * @time:		The alarm time
 */
struct optee_rtc_alarm {
	bool enabled;
	bool pending;
	struct optee_rtc_time time;
};

/*
 * enum rtc_wait_alarm_status - Return status wait_alarm ops
 * @RTC_WAIT_ALARM_RESET:		Reset the wait for the RTC alarm
 * @RTC_WAIT_ALARM_ALARM_OCCURRED:	The RTC alarm occurred
 * @RTC_WAIT_ALARM_CANCELED:		The wait for the RTC alarm was canceled
 */
enum rtc_wait_alarm_status {
	RTC_WAIT_ALARM_RESET,
	RTC_WAIT_ALARM_ALARM_OCCURRED,
	RTC_WAIT_ALARM_CANCELED,
};

/*
 * struct rtc_ops - The RTC device operations
 *
 * @get_time:	Get the RTC time.
 * @set_time:	Set the RTC time.
 * @get_offset:	Get the RTC offset.
 * @set_offset: Set the RTC offset
 * @read_alarm:	Read the RTC alarm
 * @set_alarm:	Set the RTC alarm
 * @enable_alarm: Enable the RTC alarm
 * @wait_alarm:	Wait for the RTC alarm
 * @cancel_wait: Cancel the wait for the RTC alarm
 * @set_alarm_wakeup_status: Set the wakeup capability of the alarm
 */
struct rtc_ops {
	TEE_Result (*get_time)(struct rtc *rtc, struct optee_rtc_time *tm);
	TEE_Result (*set_time)(struct rtc *rtc, struct optee_rtc_time *tm);
	TEE_Result (*get_offset)(struct rtc *rtc, long *offset);
	TEE_Result (*set_offset)(struct rtc *rtc, long offset);
	TEE_Result (*read_alarm)(struct rtc *rtc, struct optee_rtc_alarm *alrm);
	TEE_Result (*set_alarm)(struct rtc *rtc, struct optee_rtc_alarm *alrm);
	TEE_Result (*enable_alarm)(struct rtc *rtc, bool enable);
	TEE_Result (*wait_alarm)(struct rtc *rtc,
				 enum rtc_wait_alarm_status *status);
	TEE_Result (*cancel_wait)(struct rtc *rtc);
	TEE_Result (*set_alarm_wakeup_status)(struct rtc *rtc, bool status);
};

#ifdef CFG_DRIVERS_RTC
extern struct rtc *rtc_device;

/* Register a RTC device as the system RTC */
void rtc_register(struct rtc *rtc);

/**
 * rtc_is_a_leap_year() - Check if a year is a leap year
 * @year:	The year to check
 *
 * Return:	true if the year is a leap year, false otherwise
 */
bool rtc_is_a_leap_year(uint32_t year);

/**
 * rtc_get_month_days() - Get the number of days in a month
 * @month:	The month to know the number of days
 * @year:	The year of the month
 *
 * Return:	Number of days in the month
 */
uint8_t rtc_get_month_days(uint32_t month, uint32_t year);

/**
 * rtc_timecmp() - Compare two RTC time structures
 * @a:		First RTC time
 * @b:		Second RTC time
 *
 * Return a negative value if @a < @b
 * Return 0 if @a == @b
 * Return a positive value if @a > @b
 */
int rtc_timecmp(struct optee_rtc_time *a, struct optee_rtc_time *b);

/**
 * rtc_diff_calendar_ms() - Return the difference in milliseconds between
 * two times captures.
 * @ref1: First time capture
 * @ref2: Second time capture
 *
 * Return @ref1 - @ref2 in milliseconds or LLONG_MAX in case of overflow
 */
signed long long rtc_diff_calendar_ms(struct optee_rtc_time *ref1,
				      struct optee_rtc_time *ref2);

/**
 * rtc_diff_calendar_tick() - Return the difference in number of ticks between
 * two times captures.
 * @ref1: First time capture
 * @ref2: Second time capture
 * @tick_rate: Tick rate
 *
 * Return @ref1 - @ref2 in number of ticks. In case of tick computation
 * overflow, return LLONG_MAX
 */
signed long long rtc_diff_calendar_tick(struct optee_rtc_time *ref1,
					struct optee_rtc_time *ref2,
					unsigned long long tick_rate);

static inline TEE_Result rtc_get_info(uint64_t *features,
				      struct optee_rtc_time *range_min,
				      struct optee_rtc_time *range_max)
{
	if (!rtc_device)
		return TEE_ERROR_NOT_SUPPORTED;

	if (rtc_device->ops->set_offset)
		*features = RTC_CORRECTION_FEATURE;
	*range_min = rtc_device->range_min;
	*range_max = rtc_device->range_max;

	if (rtc_device->ops->set_alarm)
		*features |= RTC_ALARM_FEATURE;

	if (rtc_device->is_wakeup_source)
		*features |= RTC_WAKEUP_ALARM;

	return TEE_SUCCESS;
}

static inline TEE_Result rtc_get_time(struct optee_rtc_time *tm)
{
	if (!rtc_device)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->get_time(rtc_device, tm);
}

static inline TEE_Result rtc_set_time(struct optee_rtc_time *tm)
{
	if (!rtc_device || !rtc_device->ops->set_time)
		return TEE_ERROR_NOT_SUPPORTED;

	if (tm->tm_mon >= 12 ||
	    tm->tm_mday > rtc_get_month_days(tm->tm_mon, tm->tm_year) ||
	    tm->tm_wday >= 7 || tm->tm_hour >= 24 || tm->tm_min >= 60 ||
	    tm->tm_sec >= 60 || tm->tm_ms >= 1000 ||
	    rtc_timecmp(tm, &rtc_device->range_min) < 0 ||
	    rtc_timecmp(tm, &rtc_device->range_max) > 0)
		return TEE_ERROR_BAD_PARAMETERS;

	return rtc_device->ops->set_time(rtc_device, tm);
}

static inline TEE_Result rtc_get_offset(long *offset)
{
	if (!rtc_device || !rtc_device->ops->get_offset)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->get_offset(rtc_device, offset);
}

static inline TEE_Result rtc_set_offset(long offset)
{
	if (!rtc_device || !rtc_device->ops->set_offset)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->set_offset(rtc_device, offset);
}

static inline TEE_Result rtc_read_alarm(struct optee_rtc_alarm *alarm)
{
	if (!rtc_device || !rtc_device->ops->read_alarm)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->read_alarm(rtc_device, alarm);
}

static inline TEE_Result rtc_set_alarm(struct optee_rtc_alarm *alarm)
{
	if (!rtc_device || !rtc_device->ops->set_alarm)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->set_alarm(rtc_device, alarm);
}

static inline TEE_Result rtc_enable_alarm(bool enable)
{
	if (!rtc_device || !rtc_device->ops->enable_alarm)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->enable_alarm(rtc_device, enable);
}

static inline TEE_Result rtc_wait_alarm(enum rtc_wait_alarm_status *status)
{
	if (!rtc_device || !rtc_device->ops->wait_alarm)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->wait_alarm(rtc_device, status);
}

static inline TEE_Result rtc_cancel_wait_alarm(void)
{
	if (!rtc_device || !rtc_device->ops->cancel_wait)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->cancel_wait(rtc_device);
}

static inline TEE_Result rtc_set_alarm_wakeup_status(bool status)
{
	if (!rtc_device || !rtc_device->ops->set_alarm_wakeup_status)
		return TEE_ERROR_NOT_SUPPORTED;

	return rtc_device->ops->set_alarm_wakeup_status(rtc_device, status);
}
#else

static inline void rtc_register(struct rtc *rtc __unused) {}

static inline bool __noreturn rtc_is_a_leap_year(uint32_t year __unused)
{
	panic();
}

static inline uint8_t __noreturn rtc_get_month_days(uint32_t month __unused,
						    uint32_t year __unused)
{
	panic();
}

static inline int __noreturn rtc_timecmp(struct optee_rtc_time *a __unused,
					 struct optee_rtc_time *b __unused)
{
	panic();
}

static inline TEE_Result rtc_get_info(uint64_t *features __unused,
				      struct optee_rtc_time *range_min __unused,
				      struct optee_rtc_time *range_max __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_get_time(struct optee_rtc_time *tm __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_set_time(struct optee_rtc_time *tm __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_get_offset(long *offset __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_set_offset(long offset __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline signed long long
rtc_diff_calendar_ms(struct optee_rtc_time *ref1 __unused,
		     struct optee_rtc_time *ref2 __unused)
{
	return LLONG_MAX;
}

static inline signed long long
rtc_diff_calendar_tick(struct optee_rtc_time *ref1 __unused,
		       struct optee_rtc_time *ref2 __unused,
		       unsigned long long tick_rate __unused)
{
	return LLONG_MAX;
}

static inline TEE_Result rtc_read_alarm(struct optee_rtc_alarm *alarm __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_set_alarm(struct optee_rtc_alarm *alarm __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_enable_alarm(bool enable __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result
rtc_wait_alarm(enum rtc_wait_alarm_status *status __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_cancel_wait_alarm(void)
{
	return TEE_ERROR_NOT_SUPPORTED;
}

static inline TEE_Result rtc_set_alarm_wakeup_status(bool status __unused)
{
	return TEE_ERROR_NOT_SUPPORTED;
}
#endif
#endif /* __DRIVERS_RTC_H */
