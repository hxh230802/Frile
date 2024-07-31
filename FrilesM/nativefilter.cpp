#import <AppKit/AppKit.h>
#import <AppKit/NSWindow.h>
#import <AppKit/NSButton.h>

bool NativeFilter::nativeEventFilter(const QByteArray &eventType, void
    *message, long *result)
{
    if (eventType == "mac_generic_NSEvent") {

        NSEvent *event = static_cast<NSEvent *>(message);
        NSWindow *win = [event window];

        // TODO: determine whether or not this is a window whose
        // events you want to intercept. I did this by checking
        // [win title] but you may want to find and use the
        // window's id instead.

        // Detect a double-click on the titlebar. If the zoom button
        // is enabled, send the full-screen message to the window
        if ([event type] == NSLeftMouseUp) {
            if ([event clickCount] > 1) {
                NSPoint pt = [event locationInWindow];
                CGRect rect = [win frame];
                // event coordinates have y going in the opposite direction from frame coordinates, very annoying
                CGFloat yInverted = rect.size.height - pt.y;
                if (yInverted <= 20) {
                    // TODO: need the proper metrics for the height of the title bar

                    NSButton *btn = [win standardWindowButton:NSWindowZoomButton];
                    if (btn.enabled) {

                        // notify qml of zoom button click

                    }

                    return true;
                }
            }
        }

        if ([event type] == NSKeyDown) {

            // detect command-M (for minimize app)
            if ([event modifierFlags] & NSCommandKeyMask) {

                // M key
                if ([event keyCode] == 46) {
                    // notify qml of miniaturize button click
                    return true;
                }

            }

            // TODO: we may be requested to handle keyboard actions for close and zoom buttons. e.g. ctrl-cmd-F is zoom, I think,
            // and Command-H is hide.

        }


        if ([event type] == NSLeftMouseDown) {

            NSPoint pt = [event locationInWindow];
            CGRect rect = [win frame];

            // event coordinates have y going in the opposite direction from frame coordinates, very annoying
            CGFloat yInverted = rect.size.height - pt.y;

            NSButton *btn = [win standardWindowButton:NSWindowMiniaturizeButton];
            CGRect rectButton = [btn frame];
            if ((yInverted >= rectButton.origin.y) && (yInverted <= (rectButton.origin.y + rectButton.size.height))) {
                if ((pt.x >= rectButton.origin.x) && (pt.x <= (rectButton.origin.x + rectButton.size.width))) {

                    // notify .qml of miniaturize button click

                    return true;
                }
            }

            btn = [win standardWindowButton:NSWindowZoomButton];
            rectButton = [btn frame];

            if (btn.enabled) {
                if ((yInverted >= rectButton.origin.y) && (yInverted <= (rectButton.origin.y + rectButton.size.height))) {
                    if ((pt.x >= rectButton.origin.x) && (pt.x <= (rectButton.origin.x + rectButton.size.width))) {

                        // notify qml of zoom button click

                        return true;
                    }
                }
            }

            btn = [win standardWindowButton:NSWindowCloseButton];
            rectButton = [btn frame];
            if ((yInverted >= rectButton.origin.y) && (yInverted <= (rectButton.origin.y + rectButton.size.height))) {
                if ((pt.x >= rectButton.origin.x) && (pt.x <= (rectButton.origin.x + rectButton.size.width))) {

                    // notify qml of close button click

                    return true;
                }
            }

        }

        return false;

    }

    return false;
}
