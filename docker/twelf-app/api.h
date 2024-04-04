// https://github.com/phracker/MacOSX-SDKs/blob/041600eda65c6a668f66cb7d56b7d1da3e8bcc93/MacOSX11.0.sdk/usr/include/ConditionalMacros.h#L327
// https://github.com/phracker/MacOSX-SDKs/blob/master/MacOSX10.6.sdk/System/Library/Frameworks/Carbon.framework/Versions/A/Frameworks/HIToolbox.framework/Versions/A/Headers/TextEdit.h

/* #define CALLBACK_API(_type, _name) _type (* _name) */
/* typedef CALLBACK_API( Boolean , TEClickLoopProcPtr )(TEPtr pTE); */
typedef ProcPtr TEClickLoopUPP;
