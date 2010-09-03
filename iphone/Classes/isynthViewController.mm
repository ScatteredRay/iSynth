//
//  isynthViewController.m
//  isynth
//
//  Created by Indy on 1/31/10.
//  Copyright Apple Inc 2010. All rights reserved.
//

#include "io.h"
#include "input.h"
void beginStreamSound();
void endStreamSound();
void setupSound();

void inputDown(float down);
void inputXY(float X, float Y);

#import "isynthViewController.h"

@implementation isynthViewController


/*
// The designated initializer. Override to perform setup that is required before the view is loaded.
- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
    if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
        // Custom initialization
    }
    return self;
}
*/

/*
// Implement loadView to create a view hierarchy programmatically, without using a nib.
- (void)loadView {
}
*/



// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad {
    [super viewDidLoad];
	initInput(0, NULL);
	setupSound();
	beginStreamSound();
}


// Override to allow orientations other than the default portrait orientation.
- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
    return YES;
}

- (void)didReceiveMemoryWarning {
	// Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
	
	// Release any cached data, images, etc that aren't in use.
}

- (void)viewDidUnload {
	// Release any retained subviews of the main view.
	// e.g. self.myOutlet = nil;
	endStreamSound();
}


- (void)dealloc {
    [super dealloc];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
	UITouch* Touch = [touches anyObject];
	CGPoint point = [Touch locationInView:[Touch view]];
	CGRect bounds = [[Touch view] bounds];
	
	
	inputXY((point.x - bounds.origin.x) / bounds.size.width,
			(point.y - bounds.origin.y) / bounds.size.height);
	
	inputDown(1.0f);
	
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
	inputDown(0.0f);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	UITouch* Touch = [touches anyObject];
	CGPoint point = [Touch locationInView:[Touch view]];
	CGRect bounds = [[Touch view] bounds];
	
	
	inputXY((point.x - bounds.origin.x) / bounds.size.width,
			(point.y - bounds.origin.y) / bounds.size.height);
}

@end
