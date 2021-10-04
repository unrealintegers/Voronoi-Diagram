#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "newshape.h"
#include "utils.h"

// Assumed that each CSV line is no more than 1000 characters long
#define BUFFERSIZE 1000 + 1
#define SEP ","
#define HEADER "Watchtower ID,Postcode,Population Served,Watchtower Point of Contact Name,x,y"

#define PRECISION 1e-9

void printTower(FILE *f, tower_t t, double diameter) {
    fprintf(f, "Watchtower ID: %s, Postcode: %s, "
               "Population Served: %d, "
               "Watchtower Point of Contact Name: %s, "
               "x: %lf, y: %lf, "
               "Diameter of Cell: %lf\n",
               t.id, t.postcode, t.pop, t.contact, 
               t.coord.x, t.coord.y, diameter);
}

void printLine(FILE * const stream, line_t line) {
    if (isfinite(line.gradient)) {
        fprintf(stream, "y = %lf * (x - %lf) + %lf\n", 
                line.gradient, line.centre.x, line.centre.y);
    } else if (isinf(line.gradient)) {
        fprintf(stream, "x = %lf\n", line.centre.x);
    } else {
        fprintf(stream, "Invalid Line!\n");
    }
}

bool compareDiameter(void *a, void *b) {
    return ((face_t *) a)->diameter >= ((face_t *) b)->diameter;
}

void freeTower(void *ptr) {
    tower_t *tower = (tower_t *) ptr;

    free(tower->id);
    free(tower->postcode);
    free(tower->contact);
    free(tower);
}

void freeFace(void *ptr) {
    face_t *face = (face_t *) ptr;
    edge_t *edge, *startEdge = face->edge;

    // We need to free this in both directions
    edge = startEdge->next;
    while (edge->next != NULL) {
        edge = edge->next;
        free(edge->prev);

        // In this case, we've freed everything 
        if (edge == startEdge) {
            free(edge);
            free(face);
            return;
        }
    }
    free(edge);
    edge = startEdge;
    while (edge->prev != NULL) {
        edge = edge->prev;
        free(edge->next);
    }
    free(edge);
    free(face);
}

double findGradient(coord_t A, coord_t B) {
    vec_t v = getVec(A, B);

    return v.dy / v.dx;
}

double findNormalGradient(coord_t A, coord_t B) {
    vec_t v = getVec(A, B);

    return -v.dx / v.dy;
}

vec_t getVec(coord_t A, coord_t B) {
    return (vec_t) {.dx = B.x - A.x,
                    .dy = B.y - A.y};
}

coord_t mid(edge_t edge) {
    return mid_c(edge.start, edge.end);
}

coord_t mid_c(coord_t coord1, coord_t coord2) {
    return (coord_t) {.x = (coord1.x + coord2.x) / 2, 
                      .y = (coord1.y + coord2.y) / 2};
}

double norm(vec_t v) {
    return sqrt(v.dx * v.dx + v.dy * v.dy);
}

double dot(vec_t u, vec_t v) {
    return u.dx * v.dx + u.dy * v.dy;
} 

// Finds if a Point is on the Interior of an Edge
// 
// Note that this simply checks that the point is inside of the bounding box 
// and does not actually check if the point is on the edge
int contained(edge_t edge, coord_t point) {
    double top = max(edge.start.y, edge.end.y),
           left = min(edge.start.x, edge.end.x),
           bot = min(edge.start.y, edge.end.y),
           right = max(edge.start.x, edge.end.x);

    return (point.y <= top + PRECISION) && (point.y + PRECISION >= bot) &&
           (point.x <= right + PRECISION) && (point.x + PRECISION >= left);
}

line_t edgeToLine(edge_t edge) {
    return (line_t) {.centre = mid(edge),
                     .gradient = findGradient(edge.start, edge.end)};
}

line_t __bisectorC(coord_t A, coord_t B) {
    return (line_t) {.centre = mid_c(A, B),
                     .gradient = findNormalGradient(A, B)};
}

// coded so that using an exterior face will return bounding edge as bisector
// asserts that A and B are not both exterior
line_t __bisectorF(face_t A, face_t B) {
    assert(!(A.tower == -1 && B.tower == -1));

    if (A.tower == -1) {
        return A.defaultLine;
    } else if (B.tower == -1) {
        return B.defaultLine;
    }

    return __bisectorC(A.centre, B.centre);
}

/* The idea here is, given some half edge AB and a point X,
 * we consider the vectors u = AB and v = AX.
 * We can rotate u 90 degrees clockwise and obtain u'
 * and now all we need is to find the sign of ||proj_u'(v)||
 * which is the same sign as <u', v> (inner/dot product)
 */
int onHalfPlane(edge_t edge, coord_t coord) {
    vec_t u = getVec(edge.start, edge.end),
          v = getVec(edge.start, coord);

    // 90deg cw rotation
    vec_t uPerp = {.dx = u.dy,
                   .dy = -u.dx};
    
    double dp = dot(uPerp, v);

    // 1 = yes, 0 = incident, -1 = opposite
    return dp > 0 ? 1 : dp == 0 ? 0 : -1;
}

coord_t intersects(line_t l1, line_t l2) {
    // Edge cases where one or both are infinity
    if (isinf(l2.gradient)) {
        if (isinf(l1.gradient)) {
            return (coord_t) {HUGE_VAL, HUGE_VAL};
        }
        // m1 (xc2 - xc1) + yc1 = y
        double x = l2.centre.x;
        double y = l1.gradient * (x - l1.centre.x) + l1.centre.y;
        return (coord_t) {x, y};
    } else if (isinf(l1.gradient)) {
        // m2 (xc1 - xc2) + yc2 = y
        double x = l1.centre.x;
        double y = l2.gradient * (x - l2.centre.x) + l2.centre.y;
        return (coord_t) {x, y};
    }
    
    // Check if the lines are parallel
    if (fabs(l1.gradient - l2.gradient) < PRECISION) {
        return (coord_t) {HUGE_VAL, l1.gradient};
    } else {
        // m1 (x - xc1) + yc1 = m2 (x - xc2) + yc2
        // m1x - m2x = (m1 xc1) - (m2 xc2) - yc1 + yc2
        // x = RHS / (m1 - m2)
        double coeff = l1.gradient - l2.gradient;
        double rhs = l1.gradient * l1.centre.x - l2.gradient * l2.centre.x - 
                     l1.centre.y + l2.centre.y;
        double x = rhs / coeff;
        
        // sub back in for y
        double y = l1.gradient * (x - l1.centre.x) + l1.centre.y;
        return (coord_t) {x, y};
    }
}

// Face is simply a pointer to an edge on the face
list_t * findCuts(line_t line, face_t *face) {
    list_t *cuts = initList();
    edge_t *cur = face->edge;

    do {
        coord_t point = intersects(line, edgeToLine(*cur));

        if (contained(*cur, point)) {
            cut_t *cut = safeMalloc(sizeof(cut_t));
            *cut = (cut_t) {.coord = point,
                            .edge = cur};
            appendList(cuts, cut);
        }

        cur = cur->next;
    } while (cur != face->edge);

    return cuts;
}

long findContainingFace(list_t *faceList, coord_t coord) {
    face_t *face;
    iterList(faceList, (void **) &face);
    while (nextList(faceList)) {
        // Ignore degenerate faces (technically we shouldn't need to)
        if (face->tower == -1) {
            continue;
        }

        bool contained = true;
        edge_t *curEdge = face->edge;
        
        do {
            // If not on halfplane for some edge of face, it's not on face, so we short circuit
            if (onHalfPlane(*curEdge, coord) <= 0) {
                contained = false;
                break;
            }

            curEdge = curEdge->next;
        } while (curEdge != face->edge);

        if (contained) {
            return face->id;
        }
    }

    // Not found
    return -1;
}

double diameter(face_t *face) {
    // Degenerate face
    if (face->tower == -1) return NAN;

    edge_t * const firstEdge = face->edge;
    edge_t *curEdge1, *curEdge2;
    double maxDiameter = 0;

    curEdge1 = firstEdge;
    do {
        curEdge2 = curEdge1->next;
        do {
            double d = norm(getVec(curEdge1->start, curEdge2->start));
            maxDiameter = max(d, maxDiameter);

            curEdge2 = curEdge2->next;
        } while (curEdge2 != firstEdge);

        curEdge1 = curEdge1->next;
    } while (curEdge1 != firstEdge);

    return maxDiameter;
}

void addCell(list_t *faceList, int *index, tower_t *tower, int towerId) {
    coord_t newCentre = tower->coord;

    long faceId = findContainingFace(faceList, newCentre);
    if (faceId == -1) {
        printf("Containing Face Not Found (%lf, %lf)! Exiting...\n", newCentre.x, newCentre.y);
        return;
    }
    face_t *face = getList(faceList, faceId);

    // Find our two initial intersections
    line_t bisector = bisector(newCentre, face->centre);
    list_t *cuts = findCuts(bisector, face);
    assert(cuts->curSize == 2);
    
    // Construct a temporary edge for Half-Plane check
    edge_t edge = {.start = newCentre, .end = face->centre};
    cut_t *cut1 = getList(cuts, 0),
          *cut2 = getList(cuts, 1);

    // Here, we ensure that cut1 to cut2 is the minor arc 
    // That is, if we traverse exterior faces in a clockwise order
    // Starting from cut1, we should end up at cut2
    if (onHalfPlane(edge, cut1->coord) < 0) {
        cut_t *tmp = cut1;
        cut1 = cut2;
        cut2 = tmp;
    }

    // Create the new edge and pair
    edge_t *newEdge = safeMalloc(sizeof(edge_t)),
           *newPair = safeMalloc(sizeof(edge_t));
    *newEdge = (edge_t) {.start = cut2->coord,
                         .end = cut1->coord,
                         .pair = newPair,
                         .face = *index};
    *newPair = (edge_t) {.start = cut1->coord,
                         .end = cut2->coord,
                         .pair = newEdge,
                         .face = faceId,
                         .next = cut2->edge,
                         .prev = cut1->edge};
    face->edge = newPair;

    // Create the new face
    face_t *newFace = safeMalloc(sizeof(face_t));
    *newFace = (face_t) {.id = *index,
                         .centre = newCentre,
                         .edge = newEdge,
                         .tower = towerId};
    appendList(faceList, newFace);
    tower->face = (*index)++;

    // Note: This will set newEdge {.prev, .next}, and newPair is done already
    updateCells(faceList, newFace, *cut1, *cut2);

    freeList(cuts);
}

void updateCells(list_t *faceList, face_t *face, cut_t startCut, cut_t endCut) {
    // These are our new edges
    edge_t *prevNEdge = face->edge, *curNEdge, *curNPair, *firstNEdge;
    // This is the edge we traverse
    edge_t *curTEdge = startCut.edge->pair, *firstTEdge;
    int startFace = startCut.edge->face;
    firstNEdge = prevNEdge;

    // Clean up geometry on initial face
    curTEdge = endCut.edge->prev;

    while (curTEdge != startCut.edge) {
        curTEdge->pair->pair = NULL;
        curTEdge = curTEdge->prev;
        free(curTEdge->next);
    }

    // Fixing initial face pointers
    startCut.edge->end = startCut.coord;
    endCut.edge->start = endCut.coord;
    startCut.edge->next = face->edge->pair;
    endCut.edge->prev = face->edge->pair;

    curTEdge = curTEdge->pair;

    while (true) {
        // If we are back to our original face
        if (curTEdge->face == startFace) {
            curNEdge->next = face->edge;
            break;
        }

        // Alloc our new split edges
        curNEdge = safeMalloc(sizeof(edge_t));
        curNPair = safeMalloc(sizeof(edge_t));
        
        // This is the starting edge of this face, update pointers and vertex
        firstTEdge = curTEdge;
        curTEdge = curTEdge->prev;
        firstTEdge->start = prevNEdge->end;
        firstTEdge->prev = curNPair;

        while (true) {
            if (curTEdge->pair != NULL) {
                // Find our two adjacent faces
                int faceId1 = curTEdge->face,
                    faceId2 = curTEdge->pair->face;
                face_t *face1 = getList(faceList, faceId1),
                       *face2 = getList(faceList, faceId2);

                // Construct two bisectors
                line_t bisector1 = bisector(*face, *face1);
                line_t bisector2 = bisector(*face, *face2);

                coord_t intersection = intersects(bisector1, bisector2);
                
                // Find intersection point and check if it's on our edge
                if (contained(*curTEdge, intersection)) {
                    // If it is on our edge, we have successfully found 
                    // the next intersection point
                    *curNEdge = (edge_t) {.start = prevNEdge->end,
                                          .end = intersection,
                                          .pair = curNPair,
                                          .prev = prevNEdge,
                                          .next = NULL,
                                          .face = prevNEdge->face};
                    *curNPair = (edge_t) {.start = intersection,
                                          .end = prevNEdge->end,
                                          .pair = curNEdge,
                                          .prev = curTEdge,
                                          .next = firstTEdge,
                                          .face = faceId1};
                    prevNEdge->next = curNEdge;
                    
                    // Update face pointer
                    face1->edge = curNPair;

                    // Update curTEdge and pointer
                    curTEdge->end = intersection;
                    curTEdge->next = curNPair;
                    curTEdge = curTEdge->pair;
                    break;
                } else {
                    // This is a useless edge (as below)
                    // We un-reference it from its pair
                    curTEdge->pair->pair = NULL;
                }
            }

            // Useless edge, we traverse and free
            curTEdge = curTEdge->prev;
            free(curTEdge->next);
        }

        prevNEdge = curNEdge;
    }

    firstNEdge->prev = curNEdge;
    curNEdge->next = firstNEdge;
}

void readTowers(FILE *f, list_t *towerList) {
    char buffer[BUFFERSIZE];

    fscanf(f, "%[^\n]s", buffer);
    if (strcmp(HEADER, buffer)) {
        printf("Wrong Header!\n");
        exit(EXIT_FAILURE);
    }
    fscanf(f, "\n");

    for(int n = 1; fgets(buffer, BUFFERSIZE, f) != NULL; n++) {
        tower_t *tower = (tower_t *) safeMalloc(sizeof(tower_t));

        // ID 
        char *token = strtok(buffer, SEP);
        tower->id = safeMalloc((strlen(token) + 1) * sizeof(char));
        strcpy(tower->id, token);

        // Postcode
        token = strtok(NULL, SEP);
        tower->postcode = safeMalloc((strlen(token) + 1) * sizeof(char));
        strcpy(tower->postcode, token);

        // Population
        token = strtok(NULL, SEP); 
        sscanf(token, "%d", &(tower->pop));

        // Contact
        token = strtok(NULL, SEP);
        tower->contact = safeMalloc((strlen(token) + 1) * sizeof(char));
        strcpy(tower->contact, token);

        // Coords
        token = strtok(NULL, SEP);
        sscanf(token, "%lf", &(tower->coord.x));
        token = strtok(NULL, SEP);
        sscanf(token, "%lf", &(tower->coord.y));

        tower->face = -1;

        appendList(towerList, tower);
    }
}

void readPolygon(FILE *f, list_t *list, int *index) {
    coord_t first, cur, prev;

    edge_t *cur_cw = NULL, 
           *cur_ccw = NULL,
           *out1 = NULL,
           *out2 = NULL;
    edge_t *first_cw, *prev_cw, *first_out, *prev_out;
    face_t *cur_face = NULL;
    
    double x, y;
    bool endLoop = false, 
         firstLoop = true;

    fscanf(f, "%lf %lf", &x, &y);
    first.x = x, first.y = y;
    cur.x = x, cur.y = y;
    
    while (!endLoop) {
        prev = cur;
        prev_cw = cur_cw;
        prev_out = out2;

        // Invariant here: prev and cur edges/vertices equal

        if (fscanf(f, "%lf %lf", &x, &y) == 2) {
            cur.x = x, cur.y = y;
        } else {  // Cycle back to start
            cur = first;
            endLoop = true;
        }
        cur_cw = safeMalloc(sizeof(edge_t));
        cur_ccw = safeMalloc(sizeof(edge_t));
        cur_face = safeMalloc(sizeof(face_t));
        out1 = safeMalloc(sizeof(edge_t));
        out2 = safeMalloc(sizeof(edge_t));

        // Initialise edges and face
        *cur_cw = (edge_t) {.start = prev,
                            .end = cur,
                            .face = -1,
                            .next = NULL,
                            .prev = prev_cw,
                            .pair = cur_ccw};
        *cur_ccw = (edge_t) {.start = cur,
                             .end = prev,
                             .face = *index, 
                             .next = out1,
                             .prev = out2,
                             .pair = cur_cw};
        *cur_face = (face_t) {.id = *index,
                              .edge = cur_ccw,
                              .defaultLine = edgeToLine(*cur_cw),
                              .tower = -1};
        *out1 = (edge_t) {.start = prev,
                          .end = prev,
                          .face = *index, 
                          .next = NULL,
                          .prev = cur_ccw,
                          .pair = prev_out};
        *out2 = (edge_t) {.start = cur,
                          .end = cur,
                          .face = (*index)++, 
                          .next = cur_ccw,
                          .prev = NULL};

        if (firstLoop) {
            firstLoop = false;
            first_cw = cur_cw;
            first_out = out1;
        }

        // Append exterior/degenerate face to face list
        appendList(list, cur_face);

        if (prev_cw != NULL) prev_cw->next = cur_cw;
        if (prev_out != NULL) prev_out->pair = out1;

        // Invariant: prev is prvious of cur
    }

    // Now need to link first and last edges together
    // cur is last edge
    first_cw->prev = cur_cw; cur_cw->next = first_cw;
    out2->pair = first_out; first_out->pair = out2;

    // additionally construct our interior face
    cur_cw = first_cw;
    do {
        cur_cw->face = *index;
        cur_cw = cur_cw->next;
    } while (cur_cw != first_cw);

    cur_face = safeMalloc(sizeof(face_t));
    *cur_face = (face_t) {.id = (*index)++,
                          .edge = first_cw,
                          .tower = 0};
    appendList(list, cur_face);
}
