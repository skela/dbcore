//
//  DBSession.h
//  DropboxSDK
//
//  Created by Brian Smith on 4/8/10.
//  Copyright 2010 Dropbox, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

extern NSString *kDBCSDKVersion;

extern NSString *kDBCDropboxAPIHost;
extern NSString *kDBCDropboxAPIContentHost;
extern NSString *kDBCDropboxWebHost;
extern NSString *kDBCDropboxAPIVersion;

extern NSString *kDBCRootDropbox;
extern NSString *kDBCRootAppFolder;

extern NSString *kDBCProtocolHTTPS;

@protocol DBSessionDelegate;


/*  Creating and setting the shared DBSession should be done before any other Dropbox objects are
    used, perferrably in the UIApplication delegate. */
@class MPOAuthCredentialConcreteStore;
@interface DBCSession : NSObject {
    NSDictionary *baseCredentials;
    NSMutableDictionary *credentialStores;
    MPOAuthCredentialConcreteStore *anonymousStore;
    NSString *root;
    id<DBSessionDelegate> delegate;
}

+ (DBCSession*)sharedSession;
+ (void)setSharedSession:(DBCSession *)session;

- (id)initWithAppKey:(NSString *)key appSecret:(NSString *)secret root:(NSString *)root;
- (BOOL)isLinked; // Session must be linked before creating any DBRestClient objects

- (void)unlinkAll;
- (void)unlinkUserId:(NSString *)userId;

- (MPOAuthCredentialConcreteStore *)credentialStoreForUserId:(NSString *)userId;
- (void)updateAccessToken:(NSString *)token accessTokenSecret:(NSString *)secret forUserId:(NSString *)userId;

@property (nonatomic, readonly) NSString *root;
@property (nonatomic, readonly) NSArray *userIds;
@property (nonatomic, assign) id<DBSessionDelegate> delegate;

@end


@protocol DBSessionDelegate

- (void)sessionDidReceiveAuthorizationFailure:(DBCSession *)session userId:(NSString *)userId;

@end
