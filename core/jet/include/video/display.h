// core/jet/include/video/display.h
#ifndef CORE_JET_INCLUDE_VIDEO_DISPLAY_H
#define CORE_JET_INCLUDE_VIDEO_DISPLAY_H

#include <boost/shared_ptr.hpp>
#include "smath.h"
#include "video/displaymode.h"

namespace jet::video {
class RenderTarget;

// Platform windowing/GL-context abstraction (EAGLContext on iOS, EGL on Android, NSOpenGLContext/
// WGL on desktop). No concrete subclass exists in this rebuild yet -- that belongs with the
// System display/window layer, not yet reimplemented.
class Display {
public:
    struct StateDef { enum type { Active, Suspended, Lost }; };
    struct OrientationDef { enum type { LandscapeLeft, Portrait, LandscapeRight, PortraitUpsideDown }; };

    virtual ~Display() {}
    virtual unsigned GetClassId() = 0;
    virtual void* GetContext() = 0;
    virtual void* CreateSharedContext() = 0;
    virtual bool SetThreadContext(void*) = 0;
    virtual void Swap() = 0;
    virtual safe_enum<StateDef,StateDef::type> GetState() = 0;
    virtual bool Init(const DisplayMode&) = 0;
    virtual void Cleanup() = 0;
    virtual DisplayMode GetMode() = 0;
    virtual void SetOrientation(safe_enum<OrientationDef,OrientationDef::type>) = 0;
    virtual safe_enum<OrientationDef,OrientationDef::type> GetOrientation() = 0;
    virtual math::vec2<unsigned int> GetSize() = 0;
    virtual math::vec2<unsigned int> GetNativeSize() = 0;
    virtual const boost::shared_ptr<RenderTarget> &GetRenderTarget() = 0;
    virtual boost::shared_ptr<RenderTarget> CreateCompatibleRenderTarget() = 0;
    virtual const boost::shared_ptr<RenderTarget> &CreatePostEffectRenderTarget(unsigned, unsigned) = 0;
    virtual boost::shared_ptr<RenderTarget> GetPostEffectRenderTarget(unsigned) = 0;
    virtual bool IsLocked() = 0;
protected:
    virtual void SetLocked(bool) = 0;
    virtual bool MakeCurrent() = 0;
    virtual void Disconnect() = 0;
};
}

#endif
