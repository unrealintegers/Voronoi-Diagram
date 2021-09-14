#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "newshape.h"
#include "stage.h"

#define BUFFERSIZE 512

void stage1(char *point, char *out) {
    FILE *pf, *of;
    char buffer[BUFFERSIZE];

    pf = safeOpen(point, "r");
    of = safeOpen(out, "w");
    while (fgets(buffer, BUFFERSIZE, pf) != NULL) {
        // Get two points
        coord_t A, B;
        if (sscanf(buffer, "%lf %lf %lf %lf", &A.x, &A.y, &B.x, &B.y) != 4) {
            break;
        }
        // and print its bisector
        printLine(of, bisector(A, B));
    }
    
    fclose(pf); fclose(of);
}

void stage2(char *point, char *polygon, char *out) {
    FILE *f;
    char buffer[BUFFERSIZE];

    int index = 0;
    list_t *lineList = initList(),
           *faceList = initList();
    faceList->freeElem = freeFace;

    // First read a list of vertices
    f = safeOpen(point, "r");
    while (fgets(buffer, BUFFERSIZE, f) != NULL) {
        // Get two points
        coord_t A, B;
        if (sscanf(buffer, "%lf %lf %lf %lf", &A.x, &A.y, &B.x, &B.y) != 4) {
            break;
        }
        
        // Construct line and add to list
        line_t *line = safeMalloc(sizeof(line_t));
        *line = bisector(A, B);
        appendList(lineList, line);
    }
    fclose(f);

    // Now we read the initial polygon like A1
    f = safeOpen(polygon, "r"); 
    readPolygon(f, faceList, &index);
    fclose(f);

    // Loop through each bisector then each edge
    f = safeOpen(out, "w");
    line_t *line;
    iterList(lineList, (void **) &line);
    while (nextList(lineList)) {
        list_t *cuts = findCuts(*line, getList(faceList, index - 1));

        // Should always have 2 intersections
        assert(cuts->curSize == 2);
        cut_t *i1 = getList(cuts, 0),
              *i2 = getList(cuts, 1);
        
        fprintf(f, "From Edge %d (%lf, %lf) to Edge %d (%lf, %lf)\n",
                i1->edge->pair->face, i1->coord.x, i1->coord.y,
                i2->edge->pair->face, i2->coord.x, i2->coord.y);
        freeList(cuts);
    }

    freeList(faceList);
    freeList(lineList);
    fclose(f);
}

void stage34(char *towers, char *polygon, char *out, bool sorted) {
    FILE *f;

    int index = 0;

    tower_t *tower;
    face_t *face;
    list_t *towerList = initList(),
           *faceList = initList();
    towerList->freeElem = freeTower;
    faceList->freeElem = freeFace;
    faceList->cmp = compareDiameter;

    f = safeOpen(polygon, "r"); 
    readPolygon(f, faceList, &index);
    fclose(f);

    f = safeOpen(towers, "r");
    readTowers(f, towerList);
    fclose(f);

    iterList(towerList, (void **) &tower);
    nextList(towerList);
    tower->face = index - 1;
    face_t *firstFace = getList(faceList, index - 1);
    firstFace->centre = tower->coord;

    while (nextList(towerList)) {
        addCell(faceList, &index, tower, towerList->index - 1);
    }

    // Calculate diameter
    iterList(faceList, (void **) &face);
    while (nextList(faceList)) {
        face->diameter = diameter(face);
    }

    if (sorted) {
        iiSortList(faceList);
    }

    iterList(faceList, (void **) &face);
    while (nextList(faceList)) {
        edge_t *curEdge = face->edge;
        if (face->tower != -1) {
            tower_t *tower = getList(towerList, face->tower);
            printf("@W%ld %lf %lf\n", faceList->index - 1, tower->coord.x, tower->coord.y);
        }
        do {
            if (curEdge == NULL) break;
            printf("@E%d %lf %lf %lf %lf\n", curEdge->face, 
            curEdge->start.x, curEdge->start.y, curEdge->end.x, curEdge->end.y);
            curEdge = curEdge->prev;
        } while (curEdge != face->edge);
    }

    f = safeOpen(out, "w");

    // Iterate through towers, find face and print
    iterList(faceList, (void **) &face);
    while (nextList(faceList)) {
        if (face->tower == -1) continue;
        tower_t *tower = getList(towerList, face->tower);
        printTower(f, *tower, face->diameter);
    }
    fclose(f);

    freeList(towerList);
    freeList(faceList);
}
