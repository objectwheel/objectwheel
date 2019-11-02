#include <windowoperations.h>
#import <AppKit/AppKit.h>

namespace WindowOperations {

void setDockIconVisible(bool visible)
{
    [NSApp setActivationPolicy: visible
                                ? NSApplicationActivationPolicyRegular
                                : NSApplicationActivationPolicyProhibited];
}

}
