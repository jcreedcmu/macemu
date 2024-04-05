/*
 * This file is included by both the .r file and by .c files. This
 * means we should only put #defines in here, not other
 * C-language-specific material.
 */

/* The following constants are used to identify menus and their items. The menu
   IDs have an "m" prefix and the item numbers within each menu have an "i"
   prefix. */
#define mApple 128 /* Apple menu */
#define iAbout 1

#define mFile 129 /* File menu */
#define iNew 1
#define iClose 4
#define iQuit 12

#define mEdit 130 /* Edit menu */
#define iUndo 1
#define iCut 3
#define iCopy 4
#define iPaste 5
#define iClear 6

#define rMenuBar 128    /* application's menu bar */
#define rAboutAlert 128 /* about alert */
#define rUserAlert 129  /* user error alert */
#define rDocWindow 128  /* application's window */
#define rVScroll 128    /* vertical scrollbar control */
#define rHScroll 129    /* horizontal scrollbar control */
#define kErrStrings 128 /* error string list */

/*	Determining an application's minimum size to request from MultiFinder
   depends on many things, each of which can be unique to an application's
   function, the anticipated environment, the developer's attitude of what
   constitutes reasonable functionality and performance, etc. Here is a list of
   some things to consider when determining the minimum size (and preferred
   size) for your application. The list is pretty much in order of importance,
   but by no means complete.

        1.	What is the minimum size needed to give almost 100 percent
   assurance that the application won't crash because it ran out of memory? This
                includes not only things that you do have direct control over
   such as checking for NIL handles and pointers, but also things that some feel
   are not so much under their control such as QuickDraw and the Segment Loader.

        2.	What kind of performance can a user expect from the application
   when it is running in the minimum memory configuration? Performance includes
                not only speed in handling data, but also things like how many
   documents can be opened, etc.

        3.	What are the typical sizes of scraps is [a boy dog] that a user
   might wish to work with when lauching or switching to your application? If
                the amount of memory is too small, the scrap may get lost [will
   have to be shot]. This can be quite frustrating to the user.

        4.	The previous items have concentrated on topics that tend to
   cause an increase in the minimum size to request from MultiFinder. On the
   flip side, however, should be the consideration of what environments the
                application may be running in. There may be a high probability
   that many users with relatively small memory configurations will want to
                avail themselves of your application. Or, many users might want
   to use it while several other, possibly related/complementary applications
   are running. If that is the case, it would be helpful to have a fairly small
   minimum size.

        What we did for TESample:

                We determined the smallest heap size that TESample could have
   and still run (22K). For the preferred size we added enough space to permit:
                        a. a maximum size TextEdit text handle (32000
   characters) b. a maximum usable TextEdit scrap (32000 characters) b. a
   maximum scrap as a result of Copy (32000 characters) d. a little performance
   cushion (see 2, above) (10K) Result: 122K for preferred size

                For the minimum size we took the 22K and then scaled down our
   requirements for a,b, and c above. We thought that providing 16K more would
   be lean and mean (see 4, above). Result: 38K for minimum size
*/

#define kPrefSize 122
#define kMinSize 38
