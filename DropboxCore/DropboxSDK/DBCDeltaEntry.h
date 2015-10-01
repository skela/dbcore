//
//  DBDeltaEntry.h
//  DropboxSDK
//
//  Created by Brian Smith on 3/25/12.
//  Copyright (c) 2012 Dropbox. All rights reserved.
//

#import "DBCMetadata.h"

@interface DBCDeltaEntry : NSObject <NSCoding> {
    NSString *lowercasePath;
    DBCMetadata *metadata;
}

- (id)initWithArray:(NSArray *)array;

@property (nonatomic, readonly) NSString *lowercasePath;
@property (nonatomic, readonly) DBCMetadata *metadata; // nil if file has been deleted

@end
