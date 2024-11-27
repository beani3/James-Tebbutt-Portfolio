// Ray which has an origin and direction, both are Vec3s
class Ray 
{
    constructor(origin, direction) 
    {
        this.origin = origin
        this.direction = direction
    }

    // Calculate and return the point in space (a Vec3) for this ray for the given value of t
    pointAt(t) 
    {
        return this.origin.add(this.direction).scale(t)
    }
}