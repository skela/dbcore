//
//  DBRestClient.h
//  DropboxSDK
//
//  Created by Brian Smith on 4/9/10.
//  Copyright 2010 Dropbox, Inc. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKIt/UIKit.h>
#import "DBCSession.h"
#import "DBCRequest.h"

@protocol DBCRestClientDelegate;
@class DBCAccountInfo;
@class DBCMetadata;

@interface DBCRestClient : NSObject {
    DBCSession* session;
    NSString* userId;
    NSString* root;
    NSMutableSet* requests;
    /* Map from path to the load request. Needs to be expanded to a general framework for cancelling
       requests. */
    NSMutableDictionary* loadRequests;
    NSMutableDictionary* imageLoadRequests;
    NSMutableDictionary* uploadRequests;
    id<DBCRestClientDelegate> delegate;
}

- (id)initWithSession:(DBCSession*)session;
- (id)initWithSession:(DBCSession *)session userId:(NSString *)userId;

/* Cancels all outstanding requests. No callback for those requests will be sent */
- (void)cancelAllRequests;


/* Loads metadata for the object at the given root/path and returns the result to the delegate as a 
   dictionary */
- (void)loadMetadata:(NSString*)path withHash:(NSString*)hash;

- (void)loadMetadata:(NSString*)path;

/* This will load the metadata of a file at a given rev */
- (void)loadMetadata:(NSString *)path atRev:(NSString *)rev;

/* Loads a list of files (represented as DBDeltaEntry objects) that have changed since the cursor was generated */
- (void)loadDelta:(NSString *)cursor;


/* Loads the file contents at the given root/path and stores the result into destinationPath */
- (void)loadFile:(NSString *)path intoPath:(NSString *)destinationPath;

/* This will load a file as it existed at a given rev */
- (void)loadFile:(NSString *)path atRev:(NSString *)rev intoPath:(NSString *)destPath;

- (void)cancelFileLoad:(NSString*)path;


- (void)loadThumbnail:(NSString *)path ofSize:(NSString *)size intoPath:(NSString *)destinationPath;
- (void)cancelThumbnailLoad:(NSString*)path size:(NSString*)size;

/* Uploads a file that will be named filename to the given path on the server. sourcePath is the
   full path of the file you want to upload. If you are modifying a file, parentRev represents the
   rev of the file before you modified it as returned from the server. If you are uploading a new
   file set parentRev to nil. */
- (void)uploadFile:(NSString *)filename toPath:(NSString *)path withParentRev:(NSString *)parentRev
    fromPath:(NSString *)sourcePath;

- (void)cancelFileUpload:(NSString *)path;

/* Avoid using this because it is very easy to overwrite conflicting changes. Provided for backwards
   compatibility reasons only */
- (void)uploadFile:(NSString*)filename toPath:(NSString*)path fromPath:(NSString *)sourcePath __attribute__((deprecated));

/* These calls allow you to upload files in chunks, which is better for file larger than a few megabytes.
   You can append bytes to the file using -[DBRestClient uploadFileChunk:offset:uploadId:] and then call
   -[DBRestClient uploadFile:toPath:withParentRev:fromUploadId:] to turn the bytes appended at that uploadId
   into an actual file in the user's Dropbox.
   Use a nil uploadId to start uploading a new file. */
- (void)uploadFileChunk:(NSString *)uploadId offset:(unsigned long long)offset fromPath:(NSString *)localPath;
- (void)uploadFile:(NSString *)filename toPath:(NSString *)parentFolder withParentRev:(NSString *)parentRev
	fromUploadId:(NSString *)uploadId;


/* Loads a list of up to 10 DBCMetadata objects representing past revisions of the file at path */
- (void)loadRevisionsForFile:(NSString *)path;

/* Same as above but with a configurable limit to number of DBCMetadata objects returned, up to 1000 */
- (void)loadRevisionsForFile:(NSString *)path limit:(NSInteger)limit;

/* Restores a file at path as it existed at the given rev and returns the metadata of the restored
   file after restoration */
- (void)restoreFile:(NSString *)path toRev:(NSString *)rev;

/* Creates a folder at the given root/path */
- (void)createFolder:(NSString*)path;

- (void)deletePath:(NSString*)path;

- (void)copyFrom:(NSString*)fromPath toPath:(NSString *)toPath;

- (void)createCopyRef:(NSString *)path; // Used to copy between Dropboxes
- (void)copyFromRef:(NSString*)copyRef toPath:(NSString *)toPath; // Takes copy ref created by above call

- (void)moveFrom:(NSString*)fromPath toPath:(NSString *)toPath;

- (void)loadAccountInfo;

- (void)searchPath:(NSString*)path forKeyword:(NSString*)keyword;

- (void)loadSharableLinkForFile:(NSString *)path;
- (void)loadSharableLinkForFile:(NSString *)path shortUrl:(BOOL)createShortUrl;

- (void)loadStreamableURLForFile:(NSString *)path;

- (NSUInteger)requestCount;

@property (nonatomic, assign) id<DBCRestClientDelegate> delegate;

/* Creates dropbox requests that do not start immediately */
- (DBCRequest*)createLoadMetadataRequest:(NSString*)path;
- (DBCRequest*)createLoadMetadataRequest:(NSString*)path withHash:(NSString*)hash;
- (DBCRequest*)createLoadMetadataRequest:(NSString*)path atRev:(NSString*)rev;
- (DBCRequest*)createLoadMetadataRequest:(NSString*)path withParams:(NSDictionary *)params;
- (DBCRequest*)createLoadFileRequest:(NSString*)path intoPath:(NSString*)destPath;
- (DBCRequest*)createLoadFileRequest:(NSString*)path atRev:(NSString*)rev intoPath:(NSString*)destPath;

@end




/* The delegate provides allows the user to get the result of the calls made on the DBRestClient.
   Right now, the error parameter of failed calls may be nil and [error localizedDescription] does
   not contain an error message appropriate to show to the user. */
@protocol DBCRestClientDelegate <NSObject>

@optional

- (void)restClient:(DBCRestClient*)client loadedMetadata:(DBCMetadata*)metadata;
- (void)restClient:(DBCRestClient*)client metadataUnchangedAtPath:(NSString*)path;
- (void)restClient:(DBCRestClient*)client loadMetadataFailedWithError:(NSError*)error;
// [error userInfo] contains the root and path of the call that failed

- (void)restClient:(DBCRestClient*)client loadedDeltaEntries:(NSArray *)entries reset:(BOOL)shouldReset cursor:(NSString *)cursor hasMore:(BOOL)hasMore;
- (void)restClient:(DBCRestClient*)client loadDeltaFailedWithError:(NSError *)error;

- (void)restClient:(DBCRestClient*)client loadedAccountInfo:(DBCAccountInfo*)info;
- (void)restClient:(DBCRestClient*)client loadAccountInfoFailedWithError:(NSError*)error;

- (void)restClient:(DBCRestClient*)client loadedFile:(NSString*)destPath;
// Implement the following callback instead of the previous if you care about the value of the
// Content-Type HTTP header and the file metadata. Only one will be called per successful response.
- (void)restClient:(DBCRestClient*)client loadedFile:(NSString*)destPath contentType:(NSString*)contentType metadata:(DBCMetadata*)metadata;
- (void)restClient:(DBCRestClient*)client loadProgress:(CGFloat)progress forFile:(NSString*)destPath;
- (void)restClient:(DBCRestClient*)client loadFileFailedWithError:(NSError*)error;
// [error userInfo] contains the destinationPath


- (void)restClient:(DBCRestClient*)client loadedThumbnail:(NSString*)destPath metadata:(DBCMetadata*)metadata;
- (void)restClient:(DBCRestClient*)client loadThumbnailFailedWithError:(NSError*)error;

- (void)restClient:(DBCRestClient*)client uploadedFile:(NSString*)destPath from:(NSString*)srcPath
        metadata:(DBCMetadata*)metadata;
- (void)restClient:(DBCRestClient*)client uploadProgress:(CGFloat)progress
        forFile:(NSString*)destPath from:(NSString*)srcPath;
- (void)restClient:(DBCRestClient*)client uploadFileFailedWithError:(NSError*)error;
// [error userInfo] contains the sourcePath

- (void)restClient:(DBCRestClient *)client uploadedFileChunk:(NSString *)uploadId newOffset:(unsigned long long)offset
	fromFile:(NSString *)localPath expires:(NSDate *)expiresDate;
- (void)restClient:(DBCRestClient *)client uploadFileChunkFailedWithError:(NSError *)error;
- (void)restClient:(DBCRestClient *)client uploadFileChunkProgress:(CGFloat)progress
	forFile:(NSString *)uploadId offset:(unsigned long long)offset fromPath:(NSString *)localPath;

- (void)restClient:(DBCRestClient *)client uploadedFile:(NSString *)destPath fromUploadId:(NSString *)uploadId
    metadata:(DBCMetadata *)metadata;
- (void)restClient:(DBCRestClient *)client uploadFromUploadIdFailedWithError:(NSError *)error;

// Deprecated upload callback
- (void)restClient:(DBCRestClient*)client uploadedFile:(NSString*)destPath from:(NSString*)srcPath;

// Deprecated download callbacks
- (void)restClient:(DBCRestClient*)client loadedFile:(NSString*)destPath contentType:(NSString*)contentType;
- (void)restClient:(DBCRestClient*)client loadedThumbnail:(NSString*)destPath;

- (void)restClient:(DBCRestClient*)client loadedRevisions:(NSArray *)revisions forFile:(NSString *)path;
- (void)restClient:(DBCRestClient*)client loadRevisionsFailedWithError:(NSError *)error;

- (void)restClient:(DBCRestClient*)client restoredFile:(DBCMetadata *)fileMetadata;
- (void)restClient:(DBCRestClient*)client restoreFileFailedWithError:(NSError *)error;

- (void)restClient:(DBCRestClient*)client createdFolder:(DBCMetadata*)folder;
// Folder is the metadata for the newly created folder
- (void)restClient:(DBCRestClient*)client createFolderFailedWithError:(NSError*)error;
// [error userInfo] contains the root and path

- (void)restClient:(DBCRestClient*)client deletedPath:(NSString *)path;
- (void)restClient:(DBCRestClient*)client deletePathFailedWithError:(NSError*)error;
// [error userInfo] contains the root and path

- (void)restClient:(DBCRestClient*)client copiedPath:(NSString *)fromPath to:(DBCMetadata *)to;
- (void)restClient:(DBCRestClient*)client copyPathFailedWithError:(NSError*)error;
// [error userInfo] contains the root and path

- (void)restClient:(DBCRestClient*)client createdCopyRef:(NSString *)copyRef forPath:(NSString *)path;
- (void)restClient:(DBCRestClient*)client createCopyRefFailedWithError:(NSError *)error;

// Deprecated copy ref callback
- (void)restClient:(DBCRestClient*)client createdCopyRef:(NSString *)copyRef;

- (void)restClient:(DBCRestClient*)client copiedRef:(NSString *)copyRef to:(DBCMetadata *)to;
- (void)restClient:(DBCRestClient*)client copyFromRefFailedWithError:(NSError*)error;

- (void)restClient:(DBCRestClient*)client movedPath:(NSString *)from_path to:(DBCMetadata *)result;
- (void)restClient:(DBCRestClient*)client movePathFailedWithError:(NSError*)error;
// [error userInfo] contains the root and path

- (void)restClient:(DBCRestClient*)restClient loadedSearchResults:(NSArray*)results
forPath:(NSString*)path keyword:(NSString*)keyword;
// results is a list of DBCMetadata * objects
- (void)restClient:(DBCRestClient*)restClient searchFailedWithError:(NSError*)error;

- (void)restClient:(DBCRestClient*)restClient loadedSharableLink:(NSString*)link
forFile:(NSString*)path;
- (void)restClient:(DBCRestClient*)restClient loadSharableLinkFailedWithError:(NSError*)error;

- (void)restClient:(DBCRestClient*)restClient loadedStreamableURL:(NSURL*)url forFile:(NSString*)path;
- (void)restClient:(DBCRestClient*)restClient loadStreamableURLFailedWithError:(NSError*)error;

- (void)restClient:(DBCRestClient*)restClient requestCompleted:(DBCRequest*)request;

@end


