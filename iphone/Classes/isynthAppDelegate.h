//
//  isynthAppDelegate.h
//  isynth
//
//  Created by Indy on 1/31/10.
//  Copyright Apple Inc 2010. All rights reserved.
//

#import <UIKit/UIKit.h>

@class isynthViewController;

@interface isynthAppDelegate : NSObject <UIApplicationDelegate> {
    UIWindow *window;
    isynthViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet isynthViewController *viewController;

@end

