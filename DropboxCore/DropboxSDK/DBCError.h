//
//  DBError.h
//  DropboxSDK
//
//  Created by Brian Smith on 7/21/10.
//  Copyright 2010 Dropbox, Inc. All rights reserved.
//

/* This file contains error codes and the dropbox error domain */

#import <Foundation/Foundation.h>

extern NSString* DBCErrorDomain;

// Error codes in the dropbox.com domain represent the HTTP status code if less than 1000
typedef enum {
    DBCErrorNone = 0,
    DBCErrorGenericError = 1000,
    DBCErrorFileNotFound,
    DBCErrorInsufficientDiskSpace,
    DBCErrorIllegalFileType, // Error sent if you try to upload a directory
    DBCErrorInvalidResponse, // Sent when the client does not get valid JSON when it's expecting it
} DBCErrorCode;
