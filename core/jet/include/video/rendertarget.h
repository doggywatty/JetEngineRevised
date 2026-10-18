// core/jet/include/video/rendertarget.h
#ifndef CORE_JET_INCLUDE_VIDEO_RENDERTARGET_H
#define CORE_JET_INCLUDE_VIDEO_RENDERTARGET_H

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "core/pstring.h"
#include "smath.h"
#include "video/texture.h"

namespace pugi { class xml_node; }

namespace jet::video {
class RenderTarget : public boost::noncopyable {
protected:
    virtual ~RenderTarget();
public:
    static boost::shared_ptr<RenderTarget> New();
    virtual void Load(const pugi::xml_node&);
    virtual void Create(const jet::String&, const jet::String&, const math::vec2<unsigned int>&) = 0;
    virtual bool IsValid() = 0;
    virtual void Destroy() = 0;
    virtual void Unload() = 0;
    virtual const jet::String &GetName() = 0;
    virtual const boost::shared_ptr<Texture> &GetColorTexture() = 0;
    virtual void SetColorTexture(boost::shared_ptr<Texture>) = 0;
    virtual const boost::shared_ptr<Texture> &GetDepthTexture() = 0;
    virtual const jet::String &GetFormat() = 0;
    virtual bool HasColorTarget() = 0;
    virtual bool HasDepthTarget() = 0;
    virtual void SetClearColor(const math::vec4<float>&) = 0;
    virtual const math::vec4<float> &GetClearColor() = 0;
    virtual void SetClearDepthBuffer(bool) = 0;
    virtual bool GetClearDepthBuffer() = 0;
    virtual void SetClearColorBuffer(bool) = 0;
    virtual bool GetClearColorBuffer() = 0;
    virtual void SetClearPerFrame(bool) = 0;
    virtual bool GetClearPerFrame() = 0;
    virtual const math::vec2<unsigned int> &GetSize() = 0;
    virtual const math::vec2<unsigned int> &GetAllocatedSize() = 0;
    virtual void SetScissorTest(bool) = 0;
    virtual bool GetScissorTest() = 0;
    virtual void SetScissorRect(const math::rect<unsigned int>&) = 0;
    virtual const math::rect<unsigned int> &GetScissorRect() = 0;
protected:
    virtual void LoadV100(const pugi::xml_node&);
};
}

#endif
