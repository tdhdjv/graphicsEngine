# IcoSphere Generation
Heavily Based on: [CatLikeCoding Icosphere](https://catlikecoding.com/unity/tutorials/procedural-meshes/icosphere/)
## Algorithm Overview

The Steps behind generating an IcoSphere can be broken down to 4 steps
1. Generate Icosahedrone
1. Subdivide the Icosahedrone
1. Normalize the vertex position
1. Generate Triangles

## Generating the Icosahedrone
An icosahedrone can be generated using the golden ratio and these coordinates. (phi is the golden ratio) </br>
[Golden Rectangle used for icosahedron](https://i0.wp.com/math.ucr.edu/home/baez/icosidodecahedron/icosahedron_golden_rectangles.png)

(0, 1, phi), (0, -1, phi), (0, -1, -phi), (0, 1, -phi)
(phi, 0, 1), (-phi, 0, 1), (-phi, 0, -1), (phi, 0, -1)
(1, phi, 0), (-1, phi, 0), (-1, -phi, 0), (1, -phi, 0)

Each of this points are then grouped into 5 strips. A strip is rombus shape that goes from the top to the bottom. The points on a strip are aranged in zig zag pattern. Since the poles are shared amongst the stips they are not contained in the grouping. instead the pole are each place in index 0, and the final index

[Image of Icohedron Strips](https://catlikecoding.com/unity/tutorials/procedural-meshes/icosphere/from-octasphere-to-icosphere/icosahedron-layout.png)

```
        N---------1---------3
         \       / \       / \
          \     /   \     /   \
           \   /     \   /     \
            \ /       \ /       \
             0---------2---------S
```

## Subdividing  The Icosahedrone

We can think about sub dividing the strips into rows and columns and use (u, v) as the point inside the strip

```
            (u)
        N--------> /\- - - - - /\
         \      
          \      /    \      /    \
        (v)\               
            \  /        \  /        \
            \| - - - - - - - - - - - S
```

Then we divide the sections into 4 triangles </br>
(1) u < 1 - v </br>
(2) 1 > u > v </br>
(3) 2 - v > u > 1 </br>
(4) u > 2 -v
```
        N---------1---------3
         \       / \       / \
          \ (1) /   \ (3) /   \
           \   / (2) \   / (4) \
            \ /       \ /       \
             0---------2---------S
```

The indices are written 0 to 1, u then v. </br>
However the edges overlap with other strips and therefore only one side of the edge is indexed

ex)
```
        N-- 0-- 1-- 2-- 3-- 4--()
         \   \   \   \   \   \   \
          5-- 6-- 7-- 8-- 9--10--()
           \   \   \   \   \   \   \
           11--12--13--14--15--16--()
             \   \   \   \   \   \   \
             ()--()--()--()--()--()-- S
```

## Generating Triangles

The inside of the Triangles are formed first
ex)
```
        N   0-- 1-- 2-- 3-- 4-- 5
             \ / \ / \ / \ / \ / \
         ()   6-- 7-- 8-- 9--10--11
               \ / \ / \ / \ / \ / \
           ()  12--13--14--15--16--17
                                     
             ()  ()  ()  ()  ()  ()   S
```

Then we calculate the edge indies, and form the edges

ex)
```
        N   0-- 1-- 2-- 3-- 4-- 5
             \ / \ / \ / \ / \ / \
          18--6-- 7-- 8-- 9--10--11
           \ / \ / \ / \ / \ / \ / \
           19--12--13--14--15--16--17
             \ / \ / \ / \ / \ / \ / 
             20--21--22-- 23--29--35   S
```

Then we form the connect the north and south poles

ex)
```
        N--0-- 1-- 2-- 3-- 4-- 5
         \   \ / \ / \ / \ / \ / \
          18--6-- 7-- 8-- 9--10--11
           \ / \ / \ / \ / \ / \ / \
           19--12--13--14--15--16--17
             \ / \ / \ / \ / \ / \ / \
             20--21--22-- 23--29--35--S
```
