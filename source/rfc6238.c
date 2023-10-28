/*
 *
 *  TOTP: Time-Based One-Time Password Algorithm
 *  Copyright (c) 2017, fmount <fmount9@autistici.org>
 *
 *  This software is distributed under MIT License
 *
 *  Compute the hmac using openssl library.
 *  SHA-1 engine is used by default, but you can pass another one,
 *
 *  e.g EVP_md5(), EVP_sha224, EVP_sha512, etc
 *
 */

#include "rfc6238.h"

time_t getUnixTime(int timezoneOffset)
{
	// note: subtract, not add, the TZ offset from the system time to get the UTC time based on TZ
	// eg. if localTime = 12:00, then utcTime = localTime - (+02) = 10:00
	return (time(NULL) - (timezoneOffset * 60 * 60));
}

time_t getTotpTime(time_t t0, int timestep, int timezoneOffset)
{
    return (floor((getUnixTime(timezoneOffset) - t0) / timestep));
}

uint32_t TOTP(uint8_t *key, size_t kl, uint64_t time, int digits)
{
    uint32_t totp;
    totp = HOTP(key, kl, time, digits);
    return totp;
}
