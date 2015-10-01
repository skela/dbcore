//
//  DBAccountInfo.h
//  DropboxSDK
//
//  Created by Brian Smith on 5/3/10.
//  Copyright 2010 Dropbox, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DBCQuota.h"

@interface DBCAccountInfo : NSObject <NSCoding> {
    NSString* email;
    NSString* country;
    NSString* displayName;
    DBCQuota* quota;
    NSString* userId;
    NSString* referralLink;
    NSDictionary* original;
}

- (id)initWithDictionary:(NSDictionary*)dict;

@property (nonatomic, readonly) NSString* email;
@property (nonatomic, readonly) NSString* country;
@property (nonatomic, readonly) NSString* displayName;
@property (nonatomic, readonly) DBCQuota* quota;
@property (nonatomic, readonly) NSString* userId;
@property (nonatomic, readonly) NSString* referralLink;

@end
