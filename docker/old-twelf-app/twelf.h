#define kDelChar 8

// give ourselves a little bit of headroom before we actually hit 32kb.
#define kMaxTELength 32000

#define MARGIN 15

/*	kControlInvisible is used to 'turn off' controls (i.e., cause the
   control not to be redrawn as a result of some Control Manager call such as
   SetCtlValue) by being put into the contrlVis field of the record.
   kControlVisible is used the same way to 'turn on' the control. */
#define kControlInvisible 0
#define kControlVisible 0xFF

/*	kScrollbarAdjust and kScrollbarWidth are used in calculating
        values for control positioning and sizing. */
#define kScrollbarWidth 16
#define kScrollbarAdjust (kScrollbarWidth - 1)

#define rVScroll 128       /* vertical scrollbar control */
#define rExecButton 129    /* execute button */
#define rDebugCheckbox 130 /* debug checkbox */

/*	kScrollTweak compensates for off-by-one requirements of the scrollbars
 to have borders coincide with the growbox. */
#define kScrollTweak 2

#define kMainWidth 600
#define kMainHeight 500

#define kInputWidth 300
#define kInputHeight 150

#define kInputOffX 30
#define kInputOffY 30

#define kChicago 0
#define kGeneva 1
#define kMonaco 4

#define kAboutBox 1234  // A hack to prevent bad events from hitting about box

// Got these values from
// https://android.googlesource.com/platform/prebuilts/python/windows-x86/+/studio-master-release/x64/Lib/plat-mac/Carbon/Controls.py
#define kControlIndicatorPart 129
#define kControlUpButtonPart 20
#define kControlDownButtonPart 21
#define kControlPageUpPart 22
#define kControlPageDownPart 23

#define kMargin 10 /* between textbox and button */
#define kScrollMax 10

#define kButtonHeight 20
#define kButtonWidth 70

#define kLabelHeight 20

#define kInterTextboxHeight \
  (2 * kMargin + kButtonHeight + kInputHeight + kLabelHeight)
