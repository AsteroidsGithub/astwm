/*
    ASTWM - A Simple Compositing Window Manager

    This is a simple window manager that uses Xlib to create a window manager
    that can move and resize windows. It also has a simple compositing feature
    that allows for transparency.

    This project is made by Jonthan Voss and based upon the tinywm project
*/
#include <X11/Xlib.h>
#include <stdio.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(void)
{
    Display *dpy;
    XWindowAttributes attr;

    /* we use this to save the pointer's state at the beginning of the
     * move/resize.
     */
    XButtonEvent start;

    XEvent ev;

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
    XSelectInput(dpy, DefaultRootWindow(dpy), SubstructureNotifyMask);

    start.subwindow = None;
    for (;;)
    {

        XNextEvent(dpy, &ev);

        // // return an array of the current windows order by z index
        // Window root, parent, *children;
        // unsigned int nchildren;
        // XQueryTree(dpy, DefaultRootWindow(dpy), &root, &parent, &children, &nchildren);

        // for (int i = 0; i < nchildren; i++)
        // {
        //     XWindowAttributes tempAttr;
        //     XGetWindowAttributes(dpy, children[i], &tempAttr);

        //     // Draw a red border around the window
        //     XSetWindowBorder(dpy, children[i], 0xff0000);
        //     XSetWindowBorderWidth(dpy, children[i], 10);

        //     // Print the window depth

        //     printf("Window %d: %d\n", i, tempAttr.depth);
        // }

        if (ev.type == CreateNotify)
        {
            XSetWindowBorder(dpy, ev.xcreatewindow.window, 0xff0000);
            XSetWindowBorderWidth(dpy, ev.xcreatewindow.window, 2);
        }

        else if (ev.type == KeyPress && ev.xkey.subwindow != None)
            XRaiseWindow(dpy, ev.xkey.subwindow);
        else if (ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {

            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        /* we only get motion events when a button is being pressed,
         * but we still have to check that the drag started on a window */
        else if (ev.type == MotionNotify && start.subwindow != None)
        {

            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;

            XMoveResizeWindow(dpy, start.subwindow,
                              attr.x + (start.button == 1 ? xdiff : 0),
                              attr.y + (start.button == 1 ? ydiff : 0),
                              MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                              MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
        }

        else if (ev.type == ButtonRelease)
        {
            start.subwindow = None;
        }
    }
}
