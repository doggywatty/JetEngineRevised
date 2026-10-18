// framework/src/camera/directcamera.cpp
#include "camera/directcamera.h"

jet::String DirectCamera::k_cameraType("DirectCamera");

// @0x5ab388
void DirectCamera::Serialize(jet::stream::IStreamW &s) const {
    s.Write(&m_position, 0xc);
    s.Write(&m_rotation, 0x10);
    s.Write(&m_fov, 4);
}

// @0x5ab3e0
void DirectCamera::Deserialize(const jet::stream::IStream &s) {
    s.Read(&m_position, 0xc);
    s.Read(&m_rotation, 0x10);
    s.Read4(&m_fov);
}
