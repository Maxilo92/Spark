#import <Cocoa/Cocoa.h>
#include "MacOSUtils.h"
#include "Log.h"

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

    std::string MacOSUtils::OpenFile(const char* filter) {
        NSOpenPanel* panel = [NSOpenPanel openPanel];
        [panel setCanChooseFiles:YES];
        [panel setCanChooseDirectories:NO];
        [panel setAllowsMultipleSelection:NO];

        if (filter) {
            NSString* filterStr = [NSString stringWithUTF8String:filter];
            NSArray* types = [filterStr componentsSeparatedByString:@";"];
            [panel setAllowedFileTypes:types];
        }

        if ([panel runModal] == NSModalResponseOK) {
            NSURL* url = [[panel URLs] firstObject];
            return std::string([[url path] UTF8String]);
        }

        return "";
    }

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
