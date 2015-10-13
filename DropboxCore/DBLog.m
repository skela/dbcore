//
//  DBLog.m
//  Dropbox
//
//  Created by Will Stockwell on 11/4/10.
//  Copyright 2010 Dropbox, Inc. All rights reserved.
//

#import "DBLog.h"

static DBCLogLevel LogLevel = DBCLogLevelWarning;
static DBCLogCallback *callback = NULL;

NSString* DBCStringFromLogLevel(DBCLogLevel logLevel) {
	switch (logLevel) {
		case DBCLogLevelInfo: return @"INFO";
		case DBCLogLevelAnalytics: return @"ANALYTICS";
		case DBCLogLevelWarning: return @"WARNING";
		case DBCLogLevelError: return @"ERROR";
		case DBCLogLevelFatal: return @"FATAL";
	}
	return @"";	
}

NSString * DBCLogFilePath()
{
	static NSString *logFilePath;
	if (logFilePath == nil)
		logFilePath = [[NSHomeDirectory() stringByAppendingFormat: @"/tmp/run.log"] retain];
	return logFilePath;
}

void DBCSetupLogToFile()
{
	freopen([DBCLogFilePath() fileSystemRepresentation], "w", stderr);
}

static NSString * DBCLogFormatPrefix(DBCLogLevel logLevel) {
	return [NSString stringWithFormat: @"[%@] ", DBCStringFromLogLevel(logLevel)];
}

void DBCLogSetLevel(DBCLogLevel logLevel) {
	LogLevel = logLevel;
}

void DBCLogSetCallback(DBCLogCallback *aCallback) {
	callback = aCallback;
}

static void DBLogv(DBCLogLevel logLevel, NSString *format, va_list args) {
	if (logLevel >= LogLevel)
	{
		format = [DBCLogFormatPrefix(logLevel) stringByAppendingString: format];
		NSLogv(format, args);
		if (callback)
			callback(logLevel, format, args);
	}
}

void DBCLog(DBCLogLevel logLevel, NSString *format, ...) {
	va_list argptr;
	va_start(argptr,format);
	DBLogv(logLevel, format, argptr);
	va_end(argptr);
}

void DBCLogInfo(NSString *format, ...) {
	va_list argptr;
	va_start(argptr,format);
	DBLogv(DBCLogLevelInfo, format, argptr);
	va_end(argptr);
}

void DBCLogWarning(NSString *format, ...) {
	va_list argptr;
	va_start(argptr,format);
	DBLogv(DBCLogLevelWarning, format, argptr);
	va_end(argptr);
}

void DBCLogError(NSString *format, ...) {
	va_list argptr;
	va_start(argptr,format);
	DBLogv(DBCLogLevelError, format, argptr);
	va_end(argptr);
}

void DBCLogFatal(NSString *format, ...) {
	va_list argptr;
	va_start(argptr,format);
	DBLogv(DBCLogLevelFatal, format, argptr);
	va_end(argptr);
}

