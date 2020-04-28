#include "linmath.hpp"
#include "structs.hpp"

struct Transform {
    Vector3 Position;
    float Scale;
};

struct RigidBody {
    Vector3 Velocity;
};

struct Material {
    Color Color;
    float DiffuseCF;
    Vector3 AlbedoCF;
    Vector2 RefractCF;
    int ShadowQuality;
};

struct SphereRenderer {
    float Radius;
};

struct LightSource {
    float Power;
};

struct ChessBoardRenderer {
    Vector2 Size;
};

struct Camera {
    Vector3 Direction;
    float FocusDistance;
};
