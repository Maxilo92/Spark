#import <Cocoa/Cocoa.h>
#include "MacOSUtils.h"

static bool s_ShowAboutRequested = false;

@interface SparkMenuHandler : NSObject
- (void)showAbout:(id)sender;
@end

@implementation SparkMenuHandler
- (void)showAbout:(id)sender {
    s_ShowAboutRequested = true;
}
@end

namespace Spark {

    void MacOSUtils::SetupApplicationMenu() {
        // Wir warten kurz, bis GLFW das Menü erstellt hat
        dispatch_async(dispatch_get_main_queue(), ^{
            NSApplication* app = [NSApplication sharedApplication];
            NSMenu* mainMenu = [app mainMenu];
            if (!mainMenu) return;

            // Das erste Menü ist das App-Menü ("Spark")
            NSMenuItem* appMenuItem = [mainMenu itemAtIndex:0];
            NSMenu* appMenu = [appMenuItem submenu];

            // Suche das "About Spark" Item (meistens das erste)
            for (NSMenuItem* item in [appMenu itemArray]) {
                if ([item action] == @selector(orderFrontStandardAboutPanel:)) {
                    // Unseren eigenen Handler setzen
                    static SparkMenuHandler* handler = [[SparkMenuHandler alloc] init];
                    [item setTarget:handler];
                    [item setAction:@selector(showAbout:)];
                    break;
                }
            }
        });
    }

    bool MacOSUtils::ShouldShowAbout() {
        return s_ShowAboutRequested;
    }

    void MacOSUtils::ResetAboutFlag() {
        s_ShowAboutRequested = false;
    }

}
