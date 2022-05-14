# Sword Iownit Renderer

I was mainly inspired by the Rise of the Tomb Raider renderer, shown here: https://www.elopezr.com/the-rendering-of-rise-of-the-tomb-raider/

Main features:
* Core
    *	Tiled deferred lighting
    *	Clustered forward lighting
    *	Dynamic shadow atlas
        *	Supports 1 cascaded directional light
        *	Spot and point lights are only limited by atlas size
    *	PCF shadow filtering
* Post effects
    *	Bloom w/ separable gaussian filter
    *	Depth of field w/ separable disc filters
    *	SSR methods:
        * 3D raymarch (for testing)
        * Nonconservative DDA
        * Binary search
    *	HDR + tonemapping
*	Optimizations
    * Depth prepass
    * Hi-Z buffer
    *	Frustum culling via AABB scene graph
    *	Draw call sorting to minimize state changes

## Tiled Deferred Lighting

The idea behind tiled lighting is to only calculate lights (and shadows!) for opaque geometry only where pixels are in the light range. It divides the screen into tiles and uses the depth buffer to get min and max depth for each tile.

Normally, this is done in 1 compute shader with 3 subpasses.

1. Find depth range of tile
2. Find indices of lights within tile
3. Calculate per-pixel lighting

In my implementation, I skipped the first pass and instead read the depth range from a Hi-Z buffer computed beforehand.

When calculating light intersections, I used both Frustum-Sphere and AABB-Sphere tests. By themselves, these methods create false positives, which the other accounts for well.

For the per-pixel lighting, I wrote to diffuse and specular light textures. This gave materials more freedom to use simple or complex lighting models.

## Clustered Forward Lighting

Clustered lighting is great for transparent objects, as it has minimal reliance on the z-buffer. It uses a 3D buffer of mini-frustums that encapsulate the full camera frustum. This buffer stores indices of intersected lights, which shaders can read from.

The z-bounds of each 3D cluster should be divided in a way that makes far clusters take about as much screen space as near clusters. It should also use a function that has an easily calculated inverse. I used the function described here: http://www.aortiz.me/2018/12/21/CG.html#part-2

As an optimization, I checked the Hi-Z buffer for each cluster. If the maximum depth was closer than the far Z of the cluster, that meant the cluster was entirely occluded by opaque geometry. In this case, there was no need to calculate light intersections.

## Dynamic Shadow Atlas

(todo)

## Bloom

(todo: finish)

Instead of multiple up and down-sampling passes, I used a compute shader and calculated a separable gaussian blur in 2 passes (horizontal and vertical).

## Depth of Field

Depth of Field is an effect that approximates a "circle of confusion" (CoC) for pixels based on how far they are from a focal point. Cameras usually have a CoC that looks like a disk or hexagon-shaped bokeh.

Disk filters normally require 2D filters, which are very slow. However, I found some articles describing a disk approximation which used separable 1D filters and complex numbers.

Passes:
1. Prefilter - calculates near and far CoC and pre-multiplies RGB by the CoC
2. Far blur pass - executes 2 components in 4 dispatches
3. Near blur pass - executes 1 component in 2 dispatches
4. Composite - camera color with near and far bokehs

References:
* http://yehar.com/blog/?p=1495
* https://bartwronski.com/2017/08/06/separable-bokeh/
* https://dl.acm.org/doi/10.1145/3084363.3085022



