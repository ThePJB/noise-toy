# Controls
 * R for new seed
 * number keys to change noise fn
 * space to toggle 2d and 3d

# Noises
## Currently doing:
 * Value noise [IQ](https://www.iquilezles.org/www/articles/morenoise/morenoise.htm)
 * (domain warping) [IQ](https://www.iquilezles.org/www/articles/warp/warp.htm])
 * billow noise
 * ridge noise

## Planned noise:
 *  Nicer value noise, smooth it out cubic or quintic
 * Gradient noise [IQ](https://www.iquilezles.org/www/articles/gradientnoise/gradientnoise.htm)
 * Derivatives, fake erosion
 * Voronoi [IQ1](https://www.iquilezles.org/www/articles/voronoise/voronoise.htm) [IQ2](https://www.iquilezles.org/www/articles/smoothvoronoi/smoothvoronoi.htm)
 * Exp distribution http://jcgt.org/published/0004/02/01/ 
 * worley noise
 * combinations, fbm of ridge noise etc

## Planned other:
 * Benchmarking
 * any performance free lunch for a minecraft clone?
 * moving around
 * 3d terrain view
 * colour palette
 * colour/time dimensions
 * 2d heightmap but with 3d domain warping for cliffs? may have sort of tried that
 * add a water plane

# More reading and notes
 * [hash + noise function guts -- not that hard](https://www.youtube.com/watch?v=LWFzPP8ZbdU)
 * [hello games awesome noise talk](https://www.youtube.com/watch?v=C9RyEiEzMiU)


# refactoring ideas
camera to its own thing
noise to its own thing

mesh should maybe just contain a vao

add R to regenerate
make it infinite


# object placement
so you can have a grid, where you conditionally place an object randomly within each grid square and then remove collisions at the edges
can you vary grid size is what I want to know
i think this kinda has to be done on a per chunk basis
then u could vary number of subdivisions or whatever
hmm but sampling for neighbour collisions gets hard then