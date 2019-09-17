#ifndef DISPLAYFLAG_H
#define DISPLAYFLAG_H

typedef struct {
    bool cloud;
    bool prior;
    bool grid;
    bool picked;
    bool origin;
    bool path;
    bool pose;
    bool trail;

    bool mode;
    bool style;
}DisplayFlag;

#endif // DISPLAYFLAG_H
