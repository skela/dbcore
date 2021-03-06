//
//  DBConnectController.m
//  DropboxSDK
//
//  Created by Brian Smith on 5/4/12.
//  Copyright (c) 2012 Dropbox, Inc. All rights reserved.
//

#import "DBCConnectController.h"

#import <QuartzCore/QuartzCore.h>

#import "DBLog.h"
#import "DBCRequest.h"
#import "DBCSession+iOS.h"

#include "TargetConditionals.h"


extern id<DBNetworkRequestDelegate> dbNetworkRequestDelegate;

@interface DBCConnectController () <UIWebViewDelegate, UIAlertViewDelegate>

- (void)loadRequest;
- (BOOL)openUrl:(NSURL *)url;
- (void)dismiss;
- (void)dismissAnimated:(BOOL)animated;
- (void)cancelAnimated:(BOOL)animated;

@property (nonatomic, assign) UIViewController *rootController;
@property (nonatomic, retain) DBCSession *session;
@property (nonatomic, assign) BOOL hasLoaded;
@property (nonatomic, retain) NSURL *url;
@property (nonatomic, retain) NSURLRequest *request;
@property (nonatomic, retain) UIWebView *webView;

@end


@interface NSURL (DBCConnectController)

- (NSArray *)dbPathComponents;

@end


@implementation DBCConnectController

@synthesize rootController;
@synthesize session;
@synthesize hasLoaded;
@synthesize url;
@synthesize webView;

- (id)initWithUrl:(NSURL *)connectUrl fromController:(UIViewController *)pRootController {
    return [self initWithUrl:connectUrl fromController:pRootController session:[DBCSession sharedSession]];
}

- (id)initWithUrl:(NSURL *)connectUrl fromController:(UIViewController *)pRootController session:(DBCSession *)pSession {
    return [self initWithUrlRequest:[DBCConnectController requestForUrl:connectUrl]
                     fromController:rootController
                            session:pSession];
}

- (id)initWithUrlRequest:(NSURLRequest *)connectReq
          fromController:(UIViewController *)pRootController
                 session:(DBCSession *)pSession {
    if ((self = [super init])) {
        self.url = [connectReq URL];
        self.request = connectReq;
        self.rootController = pRootController;
        self.session = pSession;

        self.title = @"Dropbox";
        self.navigationItem.rightBarButtonItem =
            [[[UIBarButtonItem alloc]
              initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancel)]
             autorelease];

#ifdef __IPHONE_7_0 // Temporary until we can switch to XCode 5 for release.
        if (NSFoundationVersionNumber > NSFoundationVersionNumber_iOS_6_1) {
            self.edgesForExtendedLayout = UIRectEdgeNone;
        }
#endif
    }
    return self;
}

- (void)dealloc {
	[session release];
    [url release];
    if (webView.isLoading) {
        [webView stopLoading];
    }
    webView.delegate = nil;
    [webView release];
    [super dealloc];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    self.view.backgroundColor = [UIColor colorWithRed:241.0/255 green:249.0/255 blue:255.0/255 alpha:1.0];

    UIActivityIndicatorView *activityIndicator =
        [[[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray] autorelease];
    activityIndicator.autoresizingMask =
        UIViewAutoresizingFlexibleLeftMargin | UIViewAutoresizingFlexibleRightMargin |
        UIViewAutoresizingFlexibleTopMargin | UIViewAutoresizingFlexibleBottomMargin;
    CGRect frame = activityIndicator.frame;
    frame.origin.x = floorf(self.view.bounds.size.width/2 - frame.size.width/2);
    frame.origin.y = floorf(self.view.bounds.size.height/2 - frame.size.height/2) - 20;
    activityIndicator.frame = frame;
    [activityIndicator startAnimating];
    [self.view addSubview:activityIndicator];

    self.webView = [[[UIWebView alloc] initWithFrame:self.view.frame] autorelease];
    self.webView.delegate = self;
    self.webView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.webView.scalesPageToFit = YES;
    self.webView.hidden = YES;
    self.webView.dataDetectorTypes = UIDataDetectorTypeNone;
    [self.view addSubview:self.webView];

    [self loadRequest];
}

- (void)viewDidUnload {
    [super viewDidUnload];
    if ([webView isLoading]) {
        [webView stopLoading];
    }
    webView.delegate = nil;
    [webView release];
    webView = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad ||
            [self.rootController shouldAutorotateToInterfaceOrientation:interfaceOrientation]; // Delegate to presenting view.
}


#pragma mark UIWebViewDelegate methods

- (void)webViewDidStartLoad:(UIWebView *)webView {
    [dbNetworkRequestDelegate networkRequestStarted];
}

- (void)webViewDidFinishLoad:(UIWebView *)aWebView {
    [aWebView stringByEvaluatingJavaScriptFromString:@"document.documentElement.style.webkitTouchCallout = \"none\";"]; // Disable touch-and-hold action sheet
    [aWebView stringByEvaluatingJavaScriptFromString:@"document.documentElement.style.webkitUserSelect = \"none\";"]; // Disable text selection
    webView.frame = self.view.bounds;

    CATransition* transition = [CATransition animation];
    transition.duration = 0.25;
    transition.timingFunction = [CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseIn];
    transition.type = kCATransitionFade;
    [self.view.layer addAnimation:transition forKey:nil];

    webView.hidden = NO;

    hasLoaded = YES;
    [dbNetworkRequestDelegate networkRequestStopped];
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error {
    [dbNetworkRequestDelegate networkRequestStopped];

    // ignore "Fame Load Interrupted" errors and cancels
    if (error.code == 102 && [error.domain isEqual:@"WebKitErrorDomain"]) return;
    if (error.code == NSURLErrorCancelled && [error.domain isEqual:NSURLErrorDomain]) return;

    DBCLogWarning(@"DropboxSDK: error loading DBConnectController - %@", error);

    NSString *title = @"";
    NSString *message = @"";

    if ([error.domain isEqual:NSURLErrorDomain] && error.code == NSURLErrorNotConnectedToInternet) {
        title = NSLocalizedString(@"No internet connection", @"");
        message = NSLocalizedString(@"Try again once you have an internet connection.", @"");
    } else if ([error.domain isEqual:NSURLErrorDomain] &&
               (error.code == NSURLErrorTimedOut || error.code == NSURLErrorCannotConnectToHost)) {
        title = NSLocalizedString(@"Internet connection lost", @"");
        message    = NSLocalizedString(@"Please try again.", @"");
    } else {
        title = NSLocalizedString(@"Unknown Error Occurred", @"");
        message = NSLocalizedString(@"There was an error loading Dropbox. Please try again.", @"");
    }

    UIAlertController*alert = [UIAlertController alertControllerWithTitle:title message:message preferredStyle:UIAlertControllerStyleAlert];
    
    if (self.hasLoaded)
    {
        // If it has loaded, it means it's a form submit, so users can cancel/retry on their own
        NSString *okStr = NSLocalizedString(@"OK", nil);
        
        [alert addAction:[UIAlertAction actionWithTitle:okStr style:UIAlertActionStyleCancel handler:^(UIAlertAction*action){ [self clickedAlertCancel:nil]; }]];
    }
    else
    {
        // if the page hasn't loaded, this alert gives the user a way to retry
        NSString *retryStr = NSLocalizedString(@"Retry", @"Retry loading a page that has failed to load");
        [alert addAction:[UIAlertAction actionWithTitle:retryStr style:UIAlertActionStyleDefault handler:^(UIAlertAction*action){ [self clickedAlertOK:nil]; }]];
        [alert addAction:[UIAlertAction actionWithTitle:NSLocalizedString(@"Cancel", @"") style:UIAlertActionStyleCancel handler:^(UIAlertAction*action){ [self clickedAlertCancel:nil]; }]];
    }
    
    [self showAlert:alert container:nil];
}

- (void)showAlert:(UIAlertController*)alert container:(id)container
{
    if (alert.popoverPresentationController!=nil)
    {
        if (container!=nil && [container isKindOfClass:[UIBarButtonItem class]])
        {
            alert.popoverPresentationController.barButtonItem = (UIBarButtonItem*)container;
        }
        else if (container!=nil && [container isKindOfClass:[UIView class]])
        {
            UIView *v = (UIView*)container;
            alert.popoverPresentationController.sourceView = v;
            alert.popoverPresentationController.sourceRect = v.bounds;
        }
        else
        {
            alert.popoverPresentationController.permittedArrowDirections = 0;
            alert.popoverPresentationController.sourceView = self.view;
            alert.popoverPresentationController.sourceRect = self.view.frame;
        }
    }
    
    [self presentViewController:alert animated:YES completion:nil];
}

- (BOOL)webView:(UIWebView *)aWebView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType {

    NSString *appScheme = [self.session appScheme];
    if ([[[request URL] scheme] isEqual:appScheme]) {

        BOOL success = [self openUrl:[request URL]];
		if (success && ![self.session isLinked]) {
			DBCLogError(@"DropboxSDK: credentials not saved. Make sure you call -[DBSession handleOpenUrl:] in your app delegate's application:openURL:sourceApplication:annotation: method");
		}

        [self dismiss];
        return NO;
    } else if ([[[request URL] scheme] isEqual:@"itms-apps"]) {
#if TARGET_IPHONE_SIMULATOR
        DBCLogError(@"DropboxSDK - Can't open on simulator. Run on an iOS device to test this functionality");
#else
        [[UIApplication sharedApplication] openURL:[request URL]];
        [self cancelAnimated:NO];
#endif
        return NO;
    } else if (![[[request URL] dbPathComponents] isEqual:[self.url dbPathComponents]]) {
        DBCConnectController *childController =
            [[[DBCConnectController alloc] initWithUrlRequest:request
                                              fromController:self.rootController
                                                     session:self.session] autorelease];

        NSDictionary *queryParams = [DBCSession parseURLParams:[[request URL] query]];
        NSString *title = [queryParams objectForKey:@"embed_title"];
        if (title) {
            childController.title = title;
        } else {
            childController.title = self.title;
        }
        childController.navigationItem.rightBarButtonItem = nil;

        [self.navigationController pushViewController:childController animated:YES];
        return NO;
    }
    return YES;
}


#pragma mark UIAlertView methods

- (void)clickedAlertCancel:(id)sender
{
    if ([self.navigationController.viewControllers count] > 1)
    {
        [self.navigationController popViewControllerAnimated:YES];
    }
    else
    {
        [self cancel];
    }
}

- (void)clickedAlertOK:(id)sender
{
    [self loadRequest];
}

#pragma mark private methods

+ (NSURLRequest *)requestForUrl:(NSURL *)reqUrl {
    return [[[NSURLRequest alloc] initWithURL:reqUrl
                                  cachePolicy:NSURLRequestReloadIgnoringLocalCacheData
                              timeoutInterval:20] autorelease];
}

- (void)loadRequest {
    [self.webView loadRequest:self.request];
}

- (BOOL)openUrl:(NSURL *)openUrl
{
    UIApplication *app = [UIApplication sharedApplication];
    id<UIApplicationDelegate> delegate = app.delegate;

    if ([delegate respondsToSelector:@selector(application:openURL:options:)])
    {
        [delegate application:app openURL:openUrl options:@{UIApplicationOpenURLOptionsSourceApplicationKey:@"com.getdropbox.Dropbox"}];
    }
    else if ([delegate respondsToSelector:@selector(application:openURL:sourceApplication:annotation:)])
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        [delegate application:app openURL:openUrl sourceApplication:@"com.getdropbox.Dropbox" annotation:@{}];
        #pragma GCC diagnostic pop
    }
    else if ([delegate respondsToSelector:@selector(application:handleOpenURL:)])
    {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        [delegate application:app handleOpenURL:openUrl];
        #pragma GCC diagnostic pop
    }
    else
    {
		DBCLogError(@"DropboxSDK: app delegate does not implement application:openURL:sourceApplication:annotation:");
		return NO;
	}

	return YES;
}

- (void)cancelAnimated:(BOOL)animated {
    [self dismissAnimated:animated];

    NSString *cancelUrl = [NSString stringWithFormat:@"%@://%@/cancel", [self.session appScheme], kDBCDropboxAPIVersion];
    [self openUrl:[NSURL URLWithString:cancelUrl]];
}

- (void)cancel {
    [self cancelAnimated:YES];
}

- (void)dismissAnimated:(BOOL)animated {
    if ([webView isLoading]) {
        [webView stopLoading];
    }
    [self.navigationController dismissViewControllerAnimated:animated completion:nil];
}

- (void)dismiss {
    [self dismissAnimated:YES];
}

@end


@implementation NSURL (DBConnectController)

- (NSArray *)dbPathComponents {
	return [[self path] pathComponents];
}

@end

