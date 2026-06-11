#pragma once 
#include <box2d/box2d.h>

struct PhysicsComponent {
	b2BodyDef bodyDef;
	b2ShapeDef shapeDef;
	b2BodyId id;
};