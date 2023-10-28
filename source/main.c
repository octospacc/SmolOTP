#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <nds.h>
#include <fat.h>

#include "rfc4226.h"
#include "rfc6238.h"
#include "utils.h"
#include "parser.h"
#include "ini.h"

#define T0 0
#define DIGITS 6
#define VALIDITY 30
#define TIME 2
#define VERSION 1.0

#define AppConfigFile "/.AppData/SmolOTP/Config.ini"
#define MaxSecretLength 32
#define MaxSecretsCount 256

extern NODE *provider_list = NULL;

typedef struct AppConfigType {
	int timezone;
	bool useDaylightSavings;
	int totpSecretsCount;
	char totpNames[MaxSecretsCount][MaxSecretLength];
	char totpSecrets[MaxSecretsCount][MaxSecretLength];
	uint32_t totpResults[MaxSecretsCount];
} AppConfigType;

char currentTotpSecret[MaxSecretLength];

uint32_t getTotp(char secret[], int timezone)
{
	size_t pos;
    size_t len = strlen(secret);
    size_t keylen;
	time_t curtime;
    uint8_t *keysec;
    uint32_t result;

	if (validate_b32key(secret, len, pos) == 1) {
		// invalid secret
		return UINT32_MAX;
	} else {
		keysec = (uint8_t *)secret;
		keylen = decode_b32key(&keysec, len);
		curtime = getTotpTime(T0, VALIDITY, timezone);
		result = TOTP(keysec, keylen, curtime, DIGITS);
		return result;
	}
}

void waitUserExit(int code)
{
	int gamepadKeys;
	while(1) {
		swiWaitForVBlank();
		scanKeys();
		gamepadKeys = keysDown();
		if (gamepadKeys & KEY_START || gamepadKeys & KEY_SELECT) {
			exit(code);
		}
	}
}

void calcOtps(AppConfigType *AppConfig)
{
	for (int i=0; i<AppConfig->totpSecretsCount; i++)
	{
		// we copy the secret in a temporary variable because the calculation corrupts it (?)
		strcpy(currentTotpSecret, AppConfig->totpSecrets[i]);
		AppConfig->totpResults[i] = getTotp(currentTotpSecret, AppConfig->timezone);
	}
}

void printOtps(AppConfigType *AppConfig)
{
	if (AppConfig->totpSecretsCount == 0)
	{
		iprintf("\x1b[5;0H No Secrets Defined in \n \"%s\"\n", AppConfigFile);
	}
	for (int i=0; i<AppConfig->totpSecretsCount; i++)
	{
		iprintf("\x1b[%d;0H %s\n", 5+(i*2), AppConfig->totpNames[i]);

		if (AppConfig->totpResults[i] == UINT32_MAX) {
			iprintf("\x1b[%d;23H INVALID\n", 5+(i*2));
		} else {
			iprintf("\x1b[%d;24H %06u\n", 5+(i*2), AppConfig->totpResults[i]);
		}
	}
}

static int HandleAppConfig(AppConfigType *AppConfig, const char* section, const char* name, const char* value)
{
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	if (MATCH("Options", "TimezoneOffset"))
	{
		AppConfig->timezone = atoi(value);
	}
	//else if (MATCH("Options", "UseDaylightSavingTime"))
	//{
	//	AppConfig->useDaylightSavings = (strcmp(strlwr(value), "true") == 0 ? true : false);
	//}
	else if (strcmp(section, "Secrets") == 0)
	{
		strcpy(AppConfig->totpNames[AppConfig->totpSecretsCount], name);
		strcpy(AppConfig->totpSecrets[AppConfig->totpSecretsCount], value);
		AppConfig->totpSecretsCount++;
	}
	return 1;
}

void RewriteAppConfig(AppConfigType *AppConfig)
{
	FILE *file = fopen(AppConfigFile, "w");
	fprintf(file,
	"\n[Options]\n"
	"# Note: For now Daylight Saving Time is not implemented, so set your UTC offset to account for that"
	"TimezoneOffset = %d\n"
	"\n[Secrets]\n",
	AppConfig->timezone);
	for (int i=0; i<AppConfig->totpSecretsCount; i++)
	{
		fprintf(file, "%s = %s\n", AppConfig->totpNames[i], AppConfig->totpSecrets[i]);
	}
	fclose(file);
}

// TODO
/*bool isinDaylightSavings(int month, int day, int weekday, int hours, int minutes)
{
	// TODO: handle daylight savings start/end dates with alternatives to the european standard
	// https://en.m.wikipedia.org/wiki/Daylight_saving_time_by_country
	//bool isSavingsMonth = (month >= 3 && month <= 10);
	//return (isSavingsMonth && isSwitchDay weekday == && );
	return false;
}*/

int main(void)
{
	int totpViewStart = 0;
	int totpViewEnd = 5;
	int gamepadKeys;
	time_t unixTime;
	struct tm *timeStruct;
	int year, month, day, weekday, hours, minutes, seconds;
	bool inDaylightSavings;
	char totpNames[MaxSecretsCount][MaxSecretLength];
	char totpSecrets[MaxSecretsCount][MaxSecretLength];
	uint32_t totpResults[MaxSecretsCount];

	AppConfigType AppConfig = {
		.timezone = 0,
		.useDaylightSavings = false,
		.totpSecretsCount = 0,
		.totpNames = totpNames,
		.totpSecrets = totpSecrets,
		.totpResults = totpResults,
	};

	consoleDemoInit();

	if (!fatInitDefault())
	{
		iprintf("fatInitDefault failure\nPress START to exit...\n");
		waitUserExit(-1);
	}

	mkdir("/.AppData");
	mkdir("/.AppData/SmolOTP");

	if (ini_parse(AppConfigFile, HandleAppConfig, &AppConfig) < 0)
	{
		RewriteAppConfig(&AppConfig);
	}

	/*if (AppConfig.useDaylightSavings && isinDaylightSavings(month, day, weekday, hours, minutes))
	{
		inDaylightSavings = true;
		AppConfig.timezone++;
	}*/

	calcOtps(&AppConfig);
	printOtps(&AppConfig);

	while(1)
	{
		unixTime = getUnixTime(AppConfig.timezone);
		timeStruct = gmtime((const time_t *)&unixTime);

		year = timeStruct->tm_year + 1900;
		month = timeStruct->tm_mon;
		day = timeStruct->tm_mday;
		//weekday = timeStruct->tm_wday;
		hours = timeStruct->tm_hour;
		minutes = timeStruct->tm_min;
		seconds = timeStruct->tm_sec;

		iprintf("\x1b[0;0H UTC Time: %04d-%02d-%02d %02d:%02d:%02d\n (Timezone Offset: %s%d)\n\n--------------------------------",
			year, month, day, hours, minutes, seconds,
			(AppConfig.timezone >= 0 ? "+" : ""), AppConfig.timezone//,
			//(AppConfig.useDaylightSavings ? "" : "DST Adjustments are Disabled.")
		);

		if (seconds % 30 == 0)
		{
			calcOtps(&AppConfig);
			printOtps(&AppConfig);
		}

		swiWaitForVBlank();
		scanKeys();
		gamepadKeys = keysDown();

		// TODO: add secrets encryption, and decryption via keypad combos
		if (gamepadKeys & KEY_START || gamepadKeys & KEY_SELECT)
		{
			exit(0);
		}
		else if (totpViewStart > 0 && gamepadKeys & KEY_UP)
		{
			// scroll list up ...
		}
		else if (totpViewEnd < AppConfig.totpSecretsCount && gamepadKeys & KEY_DOWN)
		{
			// scroll list down ...
		}
	}

	return 0;
}
