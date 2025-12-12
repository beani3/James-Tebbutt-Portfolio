// Simple vector in 3D with numbers for x, y and z
class Vec3
{
    constructor (x, y, z)
    {
        this.x = x
        this.y = y
        this.z = z
    }

    // Add other vector to this one and return the result
    add(other)
    {
        return new Vec3(this.x + other.x, this.y + other.y, this.z + other.z)
    }

    // Subtract other vector from this one and return the result
    minus(other)
    {
        return new Vec3(this.x - other.x, this.y - other.y, this.z - other.z)
    }

    // Multiply other vector by this one and return the result
    multiply(other)
    {
        return new Vec3(this.x * other.x, this.y * other.y, this.z * other.z)
    }

    // Scale this vector by the number scalar and return the result
    scale(scalar)
    {
        return new Vec3(this.x * scalar, this.y * scalar, this.z * scalar)
    }
    
    // Calculate the dot product of this vector with the other and return the result
    dot(other) 
    {
        return ((this.x * other.x) + (this.y * other.y) + (this.z * other.z))
    }

    // Calculate and return the magnitude of this vector
    magnitude() 
    {
        return (Math.sqrt((this.x **2) + (this.y **2) + (this.z **2)))
    }
    
    // Calculate and return the magnitude of this vector without the square root
    magnitudeSquared() 
    {
        return ((this.x **2) + (this.y **2) + (this.z **2))
    }

    // Return a normalised version of this vector
    normalised() 
    {
        let mag = 1 / this.magnitude()
        return this.scale(mag)
    }
}