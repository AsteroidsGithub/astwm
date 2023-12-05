/*
    ASTWM - A Simple Compositing Window Manager

    This is a simple window manager that uses Xlib to create a window manager
    that can move and resize windows. It also has a simple compositing feature
    that allows for transparency.

    This project is made by Jonthan Voss and based upon the tinywm project
*/
#include <X11/Xlib.h>
#include <X11/Xatom.h>

#include <string.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_WINDOWS 100

typedef struct
{
    Window child;
    Window decoration;
} WindowPair;

int main(void)
{
    Display *dpy;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;

    WindowPair windowPairs[MAX_WINDOWS];
    int windowCount = 0;

    /* return failure status if we can't connect */
    if (!(dpy = XOpenDisplay(0x0)))
        return 1;

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    // on a new window create event we want to draw a red border around the window
    // and print the window depth
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureNotifyMask | StructureNotifyMask);

    start.subwindow = None;
    for (;;)
    {
        XNextEvent(dpy, &ev);

        if (ev.type == CreateNotify && ev.xcreatewindow.window != None)
        {
            Atom type;
            int format;
            unsigned long nitems, bytes_after;
            unsigned char *prop_return = NULL;

            // Check if the window has the IS_DECORATION_WINDOW property
            XGetWindowProperty(dpy, ev.xcreatewindow.window, XInternAtom(dpy, "IS_DECORATION_WINDOW", False), 0, 1, False, AnyPropertyType, &type, &format, &nitems, &bytes_after, &prop_return);

            // If the window is not a decoration window
            if (prop_return == NULL || strcmp((char *)prop_return, "1") != 0)
            {
                XWindowAttributes child_attr;
                XGetWindowAttributes(dpy, ev.xcreatewindow.window, &child_attr);

                Window decorationWindow = XCreateSimpleWindow(
                    dpy,
                    DefaultRootWindow(dpy),
                    child_attr.x,
                    child_attr.y,
                    child_attr.width + 10,
                    child_attr.height + 30,
                    1,
                    0xff0000,
                    0xffffff);

                // Add an entry to the array
                windowPairs[windowCount].child = ev.xcreatewindow.window;
                windowPairs[windowCount].decoration = decorationWindow;
                windowCount++;

                printf("Window count: %d\n", windowCount);

                XChangeProperty(dpy, decorationWindow, XInternAtom(dpy, "IS_DECORATION_WINDOW", False), XA_STRING, 8, PropModeReplace, (unsigned char *)"1", 1);

                XReparentWindow(dpy, ev.xcreatewindow.window, decorationWindow, 4, 10);

                XMapWindow(dpy, decorationWindow);
            }
        }

        if (ev.type == DestroyNotify)
        {
            for (int i = 0; i < windowCount; i++)
            {
                if (windowPairs[i].child == ev.xdestroywindow.window)
                {
                    XDestroyWindow(dpy, windowPairs[i].decoration);
                    // windowPairs[i] = windowPairs[windowCount - 1];
                    windowCount--;
                    break;
                }
            }
        }

        if (ev.type == ReparentNotify)
        {
            if (ev.xreparent.parent == DefaultRootWindow(dpy))
            {
                for (int i = 0; i < windowCount; i++)
                {
                    if (windowPairs[i].child == ev.xreparent.window)
                    {
                        XDestroyWindow(dpy, windowPairs[i].decoration);
                        windowPairs[i] = windowPairs[windowCount - 1];
                        windowCount--;
                        break;
                    }
                }
            }
        }

        if (ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {

            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        /* we only get motion events when a button is being pressed,
         * but we still have to check that the drag started on a window */
        else if (ev.type == MotionNotify && start.subwindow != None)
        {
            WindowPair windowPair;
            for (int i = 0; i < windowCount; i++)
            {
                if (windowPairs[i].child == start.subwindow || windowPairs[i].decoration == start.subwindow)
                {
                    windowPair = windowPairs[i];
                    break;
                }
            }

            XRaiseWindow(dpy, windowPair.decoration);
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;

            XMoveResizeWindow(dpy, windowPair.decoration,
                              attr.x + (start.button == 1 ? xdiff : 0),
                              attr.y + (start.button == 1 ? ydiff : 0),
                              MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                              MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));

            // Resize the child window
            XResizeWindow(dpy, windowPair.child,
                          MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                          MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
        }

        else if (ev.type == ButtonRelease)
        {
            start.subwindow = None;
        }
    }
}
