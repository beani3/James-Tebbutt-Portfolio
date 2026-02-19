// The result of casting a ray into our scene
// Position is the point where the ray intersects a sphere in the scene
// Normal is the normal unit vector of the sphere at the intersection point
// t is the t value along the ray where the intersection point is.  This value should, be -1 when the ray hits nothing
// SphereIndex is the array index of the sphere hit by the ray
class RayCastResult
{
    constructor(position, normal, t, sphereIndex)
    {
        this.position = position
        this.normal = normal
        this.t = t
        this.sphereIndex = sphereIndex
    }
}

// Calculate the intersection point and normal when a ray hits a sphere. Returns a RayCastResult.
function hit(ray, t, sphereIndex) 
{
    intersectionPoint = ray.pointAt(t)
    intersectionNormal = intersectionPoint.minus(spheres[sphereIndex].centre).normalised()
    return new RayCastResult(intersectionPoint, intersectionNormal, t, sphereIndex)
}

// Return a RayCastResult when a ray misses everything in the scene
function miss()
{
    return new RayCastResult(new Vec3(0,0,0), new Vec3(0,0,0), -1, -1)
}

// Check whether a ray hits anything in the scene and return a RayCast Result
function traceRay(ray)
{
    let t = 1000000
    let closestSphereIndex = -1

    for (let i = 0; i < spheres.length; i++)
    {
        let current_t = spheres[i].rayIntersects(ray)
        if (current_t > 0 && current_t < t)
        {
            t = current_t
            closestSphereIndex = i
        }
    }

    if (closestSphereIndex < 0) return miss()
    
    return hit(ray, t, closestSphereIndex)
}

// Calculate and return the background colour based on the ray
function backgroundColour(ray)
{
    let white = new Vec3(1, 1, 1)
    let blue = new Vec3(.3, .5, .9)
    t = 0.5*(ray.direction.y + 1.0)
    return white.scale(1-t).add(blue.scale(t))
}

// Returns the colour the ray should have as a Vec3 with RGB values in [0,1]
function rayColour(ray) 
{
    let castResult = traceRay(ray)
    if (castResult.t < 0) return backgroundColour(ray)

    let lightDirection = new Vec3(-1.1, -1.3, -1.5).normalised();
    let negLightDirection = new Vec3(-lightDirection.x, -lightDirection.y, -lightDirection.z)
    let shadow = new Ray(castResult.position, negLightDirection)
    let shadowCastRes = traceRay(shadow)
    let reflectionVec = lightDirection.minus(castResult.normal.scale(2 * castResult.normal.dot(lightDirection)))
    let viewDirection = camPos.minus(castResult.position) 
    let specCont = Math.pow(Math.max(viewDirection.dot(reflectionVec), 0), 5) * 0.8
    let albedo = spheres[castResult.sphereIndex].colour
    let diffuse = Math.max(castResult.normal.dot(negLightDirection), 0)
    let colour = albedo.scale(0.05 + diffuse + specCont)

    if (shadowCastRes.t > 0) colour = colour.scale(.4)
    return colour
}

// Sets a pixel at (x, y) in the canvas with an RGB Vec3
function setPixel(x, y, colour)
{
    var c = document.getElementById("canvas")
    var ctx = c.getContext("2d")
    ctx.fillStyle = "rgba("+colour.x+","+colour.y+","+colour.z+","+1+")"
    ctx.fillRect(x, c.height - y, 1, 1)
}