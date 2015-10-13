//
//  DBLog.h
//  Dropbox
//
//  Created by Will Stockwell on 11/4/10.
//  Copyright 2010 Dropbox, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

#if !defined(NS_FORMAT_FUNCTION)
#define NS_FORMAT_FUNCTION(F, A)
#endif

typedef enum {
	DBCLogLevelInfo = 0,
	DBCLogLevelAnalytics,
	DBCLogLevelWarning,
	DBCLogLevelError,
	DBCLogLevelFatal
} DBCLogLevel;

typedef void DBCLogCallback(DBCLogLevel logLevel, NSString *format, va_list args);

NSString * DBLogFilePath(void);
void DBSetupLogToFile(void);

NSString* DBStringFromLogLevel(DBCLogLevel logLevel);


void DBCLogSetLevel(DBCLogLevel logLevel);
void DBCLogSetCallback(DBCLogCallback *callback);

void DBCLog(DBCLogLevel logLevel, NSString *format, ...) NS_FORMAT_FUNCTION(2,3);
void DBCLogInfo(NSString *format, ...) NS_FORMAT_FUNCTION(1,2);
void DBCLogWarning(NSString *format, ...) NS_FORMAT_FUNCTION(1,2);
void DBCLogError(NSString *format, ...) NS_FORMAT_FUNCTION(1,2);
void DBCLogFatal(NSString *format, ...) NS_FORMAT_FUNCTION(1,2);