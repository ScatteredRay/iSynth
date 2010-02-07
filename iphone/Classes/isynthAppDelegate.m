//
//  isynthAppDelegate.m
//  isynth
//
//  Created by Indy on 1/31/10.
//  Copyright Apple Inc 2010. All rights reserved.
//

#import "isynthAppDelegate.h"
#import "isynthViewController.h"

@implementation isynthAppDelegate

@synthesize window;
@synthesize viewController;


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {    
    
    // Override point for customization after app launch    
    [window addSubview:viewController.view];
    [window makeKeyAndVisible];

	return YES;
}


- (void)dealloc {
    [viewController release];
    [window release];
    [super dealloc];
}


@end
