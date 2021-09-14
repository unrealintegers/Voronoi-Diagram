Construct Voronoi Diagrams given an initial (bounding) polygon and a list of watchtowers (points).

## Usage
Run `make voronoi2` for compilation, and then optionally run one of four stages using `voronoi2 <stage_num> <args>`:

1. Computes a list of equations for bisectors. Args: `<bisector_file> <output_file>`
2. Computes a list of intersections between bisectors and a polygon. Args: `<bisector_file> <polygon_file> <output_file>`
3. Constructs a voronoi diagram and calculates the diameter of each cell. Args: `<tower_file> <polygon_file> <output_file>`
4. Stage 3, but sorts cells by increasing order of diameter. Args: `<tower_file> <polygon_file> <output_file>`
