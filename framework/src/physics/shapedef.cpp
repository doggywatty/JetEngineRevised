// framework/src/physics/shapedef.cpp
#include "physics/shapedef.h"
#include "stream/streammgr.h"
#include "pugixml.hpp"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <algorithm>

// @0x653b44: no name, the origin, no rotation, the scale 1
Shape::Shape() : m_position(0, 0, 0), m_rotation(0, 0, 0, 1), m_scale(1, 1, 1) {}

// Clone @0x654838, 0x654b94, 0x654de4, 0x655004, 0x655248: what the base holds, then the shape's own
static void CopyBase(Shape &to, const Shape &from) {
    to.m_name = from.m_name;
    to.m_position = from.m_position;
    to.m_rotation = from.m_rotation;
    to.m_scale = from.m_scale;
}

BoxShape::BoxShape() : m_extents(0, 0, 0) {}
Shape *BoxShape::Clone() const {
    BoxShape *s = new BoxShape;
    CopyBase(*s, *this);
    s->m_extents = m_extents;
    return s;
}

SphereShape::SphereShape() : m_radius(0) {}
Shape *SphereShape::Clone() const {
    SphereShape *s = new SphereShape;
    CopyBase(*s, *this);
    s->m_radius = m_radius;
    return s;
}

CapsuleShape::CapsuleShape() : m_radius(0), m_height(0) {}
Shape *CapsuleShape::Clone() const {
    CapsuleShape *s = new CapsuleShape;
    CopyBase(*s, *this);
    s->m_radius = m_radius;
    s->m_height = m_height;
    return s;
}

CylinderShape::CylinderShape() : m_radius(0), m_height(0) {}
Shape *CylinderShape::Clone() const {
    CylinderShape *s = new CylinderShape;
    CopyBase(*s, *this);
    s->m_radius = m_radius;
    s->m_height = m_height;
    return s;
}

ConvexHullShape::ConvexHullShape() {}
Shape *ConvexHullShape::Clone() const {
    ConvexHullShape *s = new ConvexHullShape;
    CopyBase(*s, *this);
    s->m_vertices = m_vertices;
    s->m_triangles = m_triangles;
    return s;
}

ShapeDef::ShapeDef() : m_volume(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1)) {}
ShapeDef::~ShapeDef() { Clear(); }

// @0x653c98
void ShapeDef::Clear() {
    for (Shape *s : m_shapes) delete s;
    m_shapes.clear();
}

// @0x653d04
const Shape *ShapeDef::GetShapeByName(const jet::String &name) const {
    for (const Shape *s : m_shapes)
        if (s->m_name.Equals(name)) return s;
    return 0;
}

bool ShapeDef::Init(jet::stream::IStream *s) {
    SetStream(s);
    return true;
}

// @0x653a6c ComputeShapeVolume, @0x6538e0 ComputeLocalShapeVolume: the box of the shape in the space of the body
static math::aabb<float> ComputeLocalShapeVolume(const Shape *s) {
    math::aabb<float> v(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1));
    switch (s->GetType()) {
    case Shape::TypeDef::Box: {
        const BoxShape *b = static_cast<const BoxShape*>(s);
        v.maxPoint = math::vec3<float>(b->m_extents.x * 0.5f, b->m_extents.y * 0.5f, b->m_extents.z * 0.5f);
        v.minPoint = math::vec3<float>(-v.maxPoint.x, -v.maxPoint.y, -v.maxPoint.z);
        break;
    }
    case Shape::TypeDef::Sphere: {
        const float r = static_cast<const SphereShape*>(s)->m_radius;
        v.maxPoint = math::vec3<float>(r, r, r);
        v.minPoint = math::vec3<float>(-r, -r, -r);
        break;
    }
    case Shape::TypeDef::Capsule:
    case Shape::TypeDef::Cylinder: {
        const float r = static_cast<const CapsuleShape*>(s)->m_radius, h = static_cast<const CapsuleShape*>(s)->m_height;
        // the capsule's caps add to its height, the cylinder's height is the whole
        const float half = s->GetType() == Shape::TypeDef::Capsule ? r + h * 0.5f : h * 0.5f;
        v.maxPoint = math::vec3<float>(r, r, half);
        v.minPoint = math::vec3<float>(-r, -r, -half);
        break;
    }
    case Shape::TypeDef::ConvexHull: {
        const ConvexHullShape *c = static_cast<const ConvexHullShape*>(s);
        if (!c->m_vertices.empty()) {
            v.maxPoint = v.minPoint = c->m_vertices[0];
            for (size_t i = 1; i < c->m_vertices.size(); i++) v.addInternalPoint(c->m_vertices[i]);
        }
        break;
    }
    }
    return v;
}

static math::aabb<float> ComputeShapeVolume(const Shape *s) {
    math::aabb<float> local = ComputeLocalShapeVolume(s);
    math::mat4<float> m;
    m = s->m_rotation.getAsMat3();
    m.setTranslation(s->m_position);
    m.postScale(s->m_scale);
    math::aabb<float> out(math::vec3<float>(-1, -1, -1), math::vec3<float>(1, 1, 1));
    out.reset(m, local);
    return out;
}

// @0x6544e4
void ShapeDef::ComputeVolume() {
    m_volume = ComputeShapeVolume(m_shapes[0]);
    for (size_t i = 1; i < m_shapes.size(); i++) {
        math::aabb<float> v = ComputeShapeVolume(m_shapes[i]);
        m_volume.addInternalPoint(v.minPoint);
        m_volume.addInternalPoint(v.maxPoint);
    }
}

// @0x653798, 0x65383c: the numbers of a text, up to the count
static void ParseFloats(const char *text, unsigned count, float *out) {
    std::istringstream in(text ? text : "");
    for (unsigned i = 0; i < count && (in >> out[i]); i++) {}
}
static void ParseInts(const char *text, unsigned count, int *out) {
    std::istringstream in(text ? text : "");
    for (unsigned i = 0; i < count && (in >> out[i]); i++) {}
}

// @0x653d60: <def><shape type="box|sphere|capsule|cylinder|convex hull" name position rotation scale .../>...</def>
bool ShapeDef::Load() {
    jet::stream::IStream *s = GetStream();
    s->Start();
    s->SeekBeg(0);
    std::vector<char> text(s->Size() + 1, 0);
    s->Read(&text[0], s->Size());
    s->Finish();
    pugi::xml_document doc;
    if (!doc.load(&text[0])) return false;
    pugi::xml_node def = doc.child("def");
    if (!def) return false;
    for (pugi::xml_node n = def.child("shape"); n; n = n.next_sibling()) {
        const char *type = n.attribute("type").value();
        Shape *shape = 0;
        if (!strcmp(type, "box")) {
            BoxShape *b = new BoxShape;
            b->m_extents = math::vec3<float>(n.attribute("width").as_float(), n.attribute("length").as_float(), n.attribute("height").as_float());
            shape = b;
        } else if (!strcmp(type, "sphere")) {
            SphereShape *b = new SphereShape;
            b->m_radius = n.attribute("radius").as_float();
            shape = b;
        } else if (!strcmp(type, "capsule")) {
            CapsuleShape *b = new CapsuleShape;
            b->m_radius = n.attribute("radius").as_float();
            b->m_height = n.attribute("height").as_float();
            shape = b;
        } else if (!strcmp(type, "cylinder")) {
            CylinderShape *b = new CylinderShape;
            b->m_radius = n.attribute("radius").as_float();
            b->m_height = n.attribute("height").as_float();
            shape = b;
        } else if (!strcmp(type, "convex hull")) {
            ConvexHullShape *h = new ConvexHullShape;
            pugi::xml_node vertices = n.child("vertices");
            h->m_vertices.resize(vertices.attribute("count").as_uint());
            ParseFloats(vertices.child_value(), (unsigned)h->m_vertices.size() * 3, h->m_vertices.empty() ? 0 : &h->m_vertices[0].x);
            // each poly is a fan of triangles around its first index
            for (pugi::xml_node poly = n.child("poly"); poly; poly = poly.next_sibling()) {
                std::vector<int> idx(poly.attribute("count").as_uint());
                ParseInts(poly.child_value(), (unsigned)idx.size(), idx.empty() ? 0 : &idx[0]);
                const unsigned count = (unsigned)idx.size();
                for (unsigned i = 0; i + 2 < count; i++) {
                    h->m_triangles.push_back((unsigned short)idx[0]);
                    h->m_triangles.push_back((unsigned short)idx[(i + 1) % count]);
                    h->m_triangles.push_back((unsigned short)idx[(i + 2) % count]);
                }
            }
            shape = h;
        }
        if (!shape) continue;
        shape->m_name = n.attribute("name").value();
        ParseFloats(n.attribute("position").value(), 3, &shape->m_position.x);
        ParseFloats(n.attribute("rotation").value(), 4, &shape->m_rotation.x);
        ParseFloats(n.attribute("scale").value(), 3, &shape->m_scale.x);
        m_shapes.push_back(shape);
    }
    if (m_shapes.empty()) return false;
    ComputeVolume();
    return true;
}

// @0x6545a8
boost::shared_ptr<ShapeDef> ShapeDef::Clone() const {
    boost::shared_ptr<ShapeDef> d(new ShapeDef);
    for (const Shape *s : m_shapes) d->m_shapes.push_back(s->Clone());
    return d;
}

// @0x654618: the shapes whose name is the given
boost::shared_ptr<ShapeDef> ShapeDef::CreateSubDef(const jet::String &name) const {
    boost::shared_ptr<ShapeDef> d(new ShapeDef);
    for (const Shape *s : m_shapes)
        if (!strncmp(s->m_name.c_str(), name.c_str(), name.size())) d->m_shapes.push_back(s->Clone());
    return d;
}

// @0x6547c0
void ShapeDef::AddCopy(const Shape *s) { m_shapes.push_back(s->Clone()); }

ShapeDefLoader *ShapeDefLoader::s_instance = 0;
ShapeDefLoader *ShapeDefLoader::GetInstance() {
    if (!s_instance) s_instance = new ShapeDefLoader;
    return s_instance;
}
void ShapeDefLoader::FreeInstance() {
    delete s_instance;
    s_instance = 0;
}
ShapeDefLoader::ShapeDefLoader() {}
ShapeDefLoader::~ShapeDefLoader() { FreeAll(); }

// @0x6557b8: a definition once, however many ask for it; one that does not load is not kept
const ShapeDef *ShapeDefLoader::Load(const jet::String &name) {
    boost::unordered_map<jet::String, boost::shared_ptr<ShapeDef> >::iterator it = m_map.find(name);
    ShapeDef *d = 0;
    if (it == m_map.end()) {
        jet::stream::IStream *s = jet::stream::StreamMgr::GetInstance().CreateStream(name);
        if (!s) {
            fprintf(stderr, "ERROR: invalid stream %s\n", name.c_str());
            m_map[name] = boost::shared_ptr<ShapeDef>();
            return 0;
        }
        d = new ShapeDef;
        d->SetResourceName(name);
        if (!d->Init(s) || !d->Load()) {
            delete d;
            return 0;
        }
        m_map[name] = boost::shared_ptr<ShapeDef>(d);
    } else
        d = it->second.get();
    if (d) m_array.push_back(d);
    return d;
}

// @0x6556b0: one of the users lets go
void ShapeDefLoader::Free(const ShapeDef *d) {
    if (!d) return;
    std::vector<ShapeDef*>::iterator it = std::find(m_array.begin(), m_array.end(), d);
    if (it != m_array.end()) m_array.erase(it);
}

// @0x65579c
void ShapeDefLoader::FreeAll() { m_array.clear(); }
