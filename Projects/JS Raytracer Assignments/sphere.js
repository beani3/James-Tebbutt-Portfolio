// A sphere in 3D space. Has centre, radius and colour all of which are Vec3s
class Sphere
{
    constructor (centre, radius, colour)
    {
        this.centre = centre
        this.radius = radius
        this.colour = colour
    }


    // Calculate the point on the sphere  where the ray intersects using 
    // a quadratic equation and return the t value of the ray for that point
    // If two solutions exist return the minus solution
    // If no solutions exist return -1
    rayIntersects(ray)  
    {
        let a = ray.direction.dot(ray.direction)
        let o_minus_c = ray.origin.minus(this.centre)
        let b = 2 * o_minus_c.dot(ray.direction)
        let c = o_minus_c.dot(o_minus_c) - this.radius**2
        let desc = b**2 - 4*a*c

        if (desc > 0) {
            return (-b-Math.sqrt(desc)) / (2*a)
        } else {
            return -1
        }
    }
}