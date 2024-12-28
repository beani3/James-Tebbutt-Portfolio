// Main code
let imageWidth = document.getElementById("canvas").width
let imageHeight = document.getElementById("canvas").height
let aspectRatio = document.getElementById("canvas").height / document.getElementById("canvas").width

let viewportWidth = 2
let viewportHeight = viewportWidth * aspectRatio
let f = 1.0

let camPos = new Vec3(0, 0, 0)
let horizontal = new Vec3(viewportWidth, 0, 0)
let vertical = new Vec3(0, viewportHeight, 0)
let lCorner = camPos.minus(horizontal.scale(0.5)).minus(vertical.scale(0.5)).minus(new Vec3(0, 0, f))

const spheres = new Array(
    new Sphere(new Vec3(0,0,-1), 0.3, new Vec3(1,0,0)),       // Red sphere
    new Sphere(new Vec3(0,0.2,-0.8), 0.15, new Vec3(0,0,1)),  // Blue sphere
    new Sphere(new Vec3(0,-100.5,-1), 100, new Vec3(0,1,0))   // Big green sphere
    );

const pseudo = 50

    for (let i = 0; i < imageWidth; i++)
    {
        for (let j = 0; j <= imageHeight; j++)
        {
            let colour = new Vec3(0,0,0)
            for (let k = 0; k < 50; k++ )
            {
                let u = i / (imageWidth -1)
                let v = j / (imageHeight -1)
                let dir = lCorner.add(horizontal.scale(u)).add(vertical.scale(v)).minus(camPos)
                dir = dir.add(new Vec3(Math.random() / imageWidth, Math.random() / imageHeight, 0))
                let ray = new Ray(camPos, dir)
                colour = colour.add(rayColour(ray)) 
            }
            colour = colour.scale(1 / 50)
            let gammaCorrect = new Vec3(Math.sqrt(colour.x), Math.sqrt(colour.y), Math.sqrt(colour.z))
            colour = gammaCorrect.scale(255)
            setPixel(i,j,colour)
        }
    }