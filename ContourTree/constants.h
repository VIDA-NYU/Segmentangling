#ifndef CONSTANTS_H
#define CONSTANTS_H

const char REGULAR = 0;
const char MINIMUM = 1;
const char MAXIMUM = 2;
const char SADDLE = 4;

// Following the nomenclature of original Carr paper.
// JoinTree -> maxima and SplitTree -> minima
enum TreeType {JoinTree, SplitTree, ContourTree};


#endif // CONSTANTS_H
