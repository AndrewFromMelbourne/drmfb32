# Conway's Game of Life

A version of Conway's Game of Life for the Odroid Go Super, written in C++.

# usage
        life <options>

        --device,-d - framebuffer device to use (default is /dev/fb0)
        --help,-h - print usage and exit

## Controls:-
- (A) Switch between displaying cells and displaying a 'heat map' of cell's neighbour count.
- (B) Create a new random arrangement of cells with approximately half of the cells 'Alive'.
- (X) Create a 'Gosper Glider Gun' in the middle of the field.
- (Y) Create a 'Simkin Glider Gun' in the middle of the field.
- [Top right function key] Exit.

![Conway's Game of Life](assets/life.png)

