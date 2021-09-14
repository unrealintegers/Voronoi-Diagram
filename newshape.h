#ifndef NSHAPE_H
#define NSHAPE_H

#include "utils.h"

typedef struct Coordinate {
    double x, y;
} coord_t;

typedef struct Watchtower {
    char *id;      // Watchtower ID
    char *postcode;  // Postcode
    int pop;         // Population Served
    char *contact;   // Watchtower Point of Contact Name
    coord_t coord;   // x, y

    int face;
} tower_t;

typedef struct Vector {
    double dx, dy;
} vec_t;

typedef struct Line {
    coord_t centre;
    double gradient;
} line_t;

typedef struct HalfEdge edge_t;

struct HalfEdge {
    coord_t start, end;
    int face;
    edge_t *pair;
    edge_t *next;
    edge_t *prev;
};

typedef struct Intersection {
    coord_t coord;
    edge_t *edge;
} cut_t;

typedef struct VoronoiCell {
    int id;
    coord_t centre;
    double diameter;
    edge_t *edge;
    line_t defaultLine;
    int tower;
} face_t;

// Prints a tower
void printTower(FILE *, tower_t, double);

// Prints a line
void printLine(FILE *, line_t);

// Compares the diameter of two faces
// Returns true if first element is larger than or equal to second
bool compareDiameter(void *, void *);

// Frees a Tower
void freeTower(void *);

// Frees a Face
void freeFace(void *);

// Finds the gradient between two points
double findGradient(coord_t, coord_t);

// Finds the normal gradient between two points
double findNormalGradient(coord_t, coord_t);

// Constructs a Vector from two points
vec_t getVec(coord_t, coord_t);

// Finds Midpoint of Edge
coord_t mid(edge_t);

// Finds Midpoint of Two Points
coord_t mid_c(coord_t, coord_t);

// Vector Norm
double norm(vec_t);

// Vector Dot Product
double dot(vec_t, vec_t);

// Finds if a point is on the interior of an edge
int contained(edge_t, coord_t);

// Constructs a Line from an edge
line_t edgeToLine(edge_t);

// Finds the perpendicular bisector given two points
line_t __bisectorC(coord_t, coord_t);

// Finds the perpendicular bisector given two faces
line_t __bisectorF(face_t, face_t);

// Checks if a Point is on the Right Half-Plane of an Edge
int onHalfPlane(edge_t, coord_t);

// Finds the Intersection Point between Two Lines
coord_t intersects(line_t, line_t);

// Finds the Intersections between a Line and a Face
list_t * findCuts(line_t, face_t *);

// Finds which face a Point is in
long findContainingFace(list_t *, coord_t);

// Calculates the diameter of a face
double diameter(face_t *);

// Inserts a new Voronoi Cell
void addCell(list_t *, int *, tower_t *, int);

// Updates Cells after insertion
void updateCells(list_t *, face_t *, cut_t, cut_t);

// Reads in a list of Watchtowers
void readTowers(FILE *, list_t *);

// Reads in an Initial Polygon from a file
void readPolygon(FILE *, list_t *, int *);

#define bisector(x, y) _Generic((x), coord_t: __bisectorC, face_t: __bisectorF)(x, y)

#endif
