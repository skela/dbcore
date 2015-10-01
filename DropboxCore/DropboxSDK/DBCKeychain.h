//
//  DBKeychain.h
//  DropboxSDK
//
//  Created by Brian Smith on 4/5/12.
//  Copyright (c) 2012 Dropbox, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface DBCKeychain : NSObject

+ (NSDictionary *)credentials;

+ (void)setCredentials:(NSDictionary *)credentials;

+ (void)deleteCredentials;

@end
