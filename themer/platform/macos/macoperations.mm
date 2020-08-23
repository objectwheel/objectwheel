#include <macoperations.h>
#import <AppKit/AppKit.h>

namespace MacOperations {
void setDockIconVisible(bool visible)
{
    [NSApp setActivationPolicy: visible
                                ? NSApplicationActivationPolicyRegular
                                : NSApplicationActivationPolicyProhibited];
}
} // MacOperations
