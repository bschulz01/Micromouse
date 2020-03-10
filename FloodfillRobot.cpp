//
//  FloodfillRobot.cpp
//  MicromouseMazeSimulator
//
//  Created by Bradley Schulz on 3/9/20.
//  Copyright © 2020 Bradley Schulz. All rights reserved.
//

#include "FloodfillRobot.h"

ourRobot::ourRobot(bool shouldPause) : pause(shouldPause) {
    visitedStart = false;
    heading = NORTH;
    // initialize the walls
    for (int i = 0; i < MazeDefinitions::MAZE_LEN + 1; i++) {
        if (i == 0 || i == MazeDefinitions::MAZE_LEN) {
            for (int j = 0; j < MazeDefinitions::MAZE_LEN + 1; j++) {
                verticalWalls[i][j] = true;
                horizontalWalls[j][i] = true;
            }
        } else {
            for (int j = 0; j < MazeDefinitions::MAZE_LEN + 1; j++) {
                verticalWalls[i][j] = false;
                horizontalWalls[j][i] = false;
            }
        }
    }
    
    // initialize the distances
    int mazeLen = MazeDefinitions::MAZE_LEN-1;
    int midpoint = mazeLen / 2;
    for (int i = 0; i <= midpoint; i++) {
        for (int j = 0; j <= midpoint; j++) {
            int currentDistance = abs(midpoint - i) + abs(midpoint - j);
            distances[i][j] = currentDistance;
            distances[i][mazeLen-j] = currentDistance;
            distances[mazeLen-i][j] = currentDistance;
            distances[mazeLen-i][mazeLen-j] = currentDistance;
        }
    }
    printMaze();
}

MouseMovement ourRobot::nextMovement(unsigned x, unsigned y, const Maze &maze) {
    const bool frontWall = maze.wallInFront();
    const bool leftWall  = maze.wallOnLeft();
    const bool rightWall = maze.wallOnRight();
    
    // Update our wall data
    if (frontWall) {
        switch(heading) {
            case NORTH: horizontalWalls[x][y+1] = true;
                break;
            case SOUTH: horizontalWalls[x][y] = true;
                break;
            case EAST: verticalWalls[x+1][y] = true;
                break;
            case WEST: verticalWalls[x][y] = true;
                break;
            default:
                break;
        }
    }
    if (leftWall) {
        switch(heading) {
            case NORTH: verticalWalls[x][y] = true;
                break;
            case SOUTH: verticalWalls[x+1][y] = true;
                break;
            case EAST: horizontalWalls[x][y+1] = true;
                break;
            case WEST: horizontalWalls[x][y] = true;
                break;
            default:
                break;
        }
    }
    if (rightWall) {
        switch(heading) {
            case NORTH: verticalWalls[x+1][y] = true;
                break;
            case SOUTH: verticalWalls[x][y] = true;
                break;
            case EAST: horizontalWalls[x][y] = true;
                break;
            case WEST: horizontalWalls[x][y+1] = true;
                break;
            default:
                break;
        }
    }
    
    // Pause at each cell if the user requests it.
    // It allows for better viewing on command line.
    if(pause) {
        std::cout << "Hit enter to continue..." << std::endl;
        std::cin.ignore(10000, '\n');
        std::cin.clear();
    }

    std::cout << maze.draw(5) << std::endl << std::endl;

    // If we somehow miraculously hit the center
    // of the maze, just terminate and celebrate!
    if(isAtCenter(x, y)) {
        std::cout << "Found center! Good enough for the demo, won't try to get back." << std::endl;
        return Finish;
    }

    // If we hit the start of the maze a second time, then
    // we couldn't find the center and never will...
    if(x == 0 && y == 0) {
        if(visitedStart) {
            std::cout << "Unable to find center, giving up." << std::endl;
            return Finish;
        } else {
            visitedStart = true;
        }
    }
    
    // See if any of our possible actions decrease the distance
    MouseMovement nextMove = bestMove(frontWall, rightWall, leftWall, x, y);
    if (nextMove != Wait) {
        return(nextMove);
    } else {
        // If it gets to this point, our distances function needs to be updated so we have to re-run the floodfill algorithm
        runFloodfill(x, y);
        return(bestMove(frontWall, rightWall, leftWall, x, y));
    }
}

bool ourRobot::isAtCenter(unsigned x, unsigned y) const {
    unsigned midpoint = MazeDefinitions::MAZE_LEN / 2;

    if(MazeDefinitions::MAZE_LEN % 2 != 0) {
        return x == midpoint && y == midpoint;
    }

    return  (x == midpoint     && y == midpoint    ) ||
    (x == midpoint - 1 && y == midpoint    ) ||
    (x == midpoint     && y == midpoint - 1) ||
    (x == midpoint - 1 && y == midpoint - 1);
}

    // When none of the adjacent traversible cells has a lower distance than the current cell.
void ourRobot::runFloodfill(unsigned int x, unsigned int y) {
    
    std::cout << "x: " << x << " y: " << y << std::endl;
    printMaze();

    /*
     1) Look at possible adjacent cells and make the current cell 1 more than all the previous cells
     2) Make sure that all the adjacent cells have at least one cell with a lower distance
     3) Keep doing this until you reach the end
     */
    // possible actions determines whether or not we consider left, right, above, or below (respectively)
    bool possibleDirections[4] = {false, false, false, false};
    distances[x][y] = lowestAdjacent(x, y, possibleDirections) + 1;
    // rerun floodfill on adjacent cells until the cell has a value that is less than it
    // cell to the left
    if (x > 0 && !cellFinished(x-1, y) && possibleDirections[0]) {
        runFloodfill(x-1, y);
    }
    // cell to the right
    if (x < MazeDefinitions::MAZE_LEN && !cellFinished(x+1, y) && possibleDirections[1]) {
        runFloodfill(x+1, y);
    }
    // cell above
    if (y < MazeDefinitions::MAZE_LEN && !cellFinished(x, y+1) && possibleDirections[2]) {
        runFloodfill(x, y+1);
    }
    // cell below
    if (y > 0 && !cellFinished(x, y-1) && possibleDirections[3]) {
        runFloodfill(x, y-1);
    }
    // update the current cell after re-running the floodfill with the adjacent cells
    if (!cellFinished(x, y)) {
        distances[x][y] = lowestAdjacent(x, y, possibleDirections);
    }
    
//        new stack //Stack of cells to be processed (can also use queue)
//        push current cell onto stack
//        while stack is not empty:
//            cur = top of the stack
//            pop off top of the stack
//            if cur.distance == 0, continue //don’t want to process the end goal
//            min_distance = infinity //placeholder/”invalid” distance
//            for each neighboring cell of cur:
//                if no wall between cur and neighbor:
//                    if neighbor.distance < min_distance:
//        min_distance = neighbor.distance
//            if min_distance == infinity: //something went wrong. terminate
//                continue
//            if cur.distance > min_distance: //everything is fine, move on
//                Continue
//            if cur.distance <= min_distance //we reach this point
//            cur.distance = min_distance + 1 //new minimum distance
//            push every neighbor onto stack

}

int ourRobot::lowestAdjacent(int x, int y, bool possibleDirections[]) {
    int minDistance = distances[x][y] + 2;
    // if there's a wall to the left
    if (!verticalWalls[x][y] && x > 0) {
        possibleDirections[0] = true;
        if (distances[x-1][y] < minDistance) {
            minDistance = distances[x-1][y];
        }
    }
    // if there's a wall to the right
    if (!verticalWalls[x+1][y] && x < MazeDefinitions::MAZE_LEN - 1) {
        possibleDirections[1] = true;
        if (distances[x+1][y] < minDistance) {
            minDistance = distances[x+1][y];
        }
    }
    // if there's a wall above
    if (!horizontalWalls[x][y+1] && y < MazeDefinitions::MAZE_LEN - 1) {
        possibleDirections[2] = true;
        if (distances[x][y+1] < minDistance) {
            minDistance = distances[x][y+1];
        }
    }
    // if there's a wall below
    if (!horizontalWalls[x][y] && y > 0) {
        possibleDirections[3] = true;
        if (distances[x][y-1] < minDistance) {
            minDistance = distances[x][y-1];
        }
    }
    return (minDistance);
}

// determine if a cell has at least one lower value around it
bool ourRobot::cellFinished(unsigned int x, unsigned int y) {
    // cell to the left has a lower value
    if (x > 0 && !verticalWalls[x][y] && distances[x-1][y] < distances[x][y]) {
        return(true);
    }
    // cell to the right has a lower value
    if (x < MazeDefinitions::MAZE_LEN - 1 && !verticalWalls[x+1][y] && distances[x+1][y] < distances[x][y]) {
        return(true);
    }
    // cell above has a lower value
    if (y < MazeDefinitions::MAZE_LEN - 1 && !horizontalWalls[x][y+1] &&  distances[x][y+1] < distances[x][y]) {
        return(true);
    }
    // cell below has a lower value
    if ( y > 0 && !horizontalWalls[x][y] && distances[x][y-1] < distances[x][y]) {
        return(true);
    }
    // if none of the possible actions return a lower distance, the cell is not finished
    return(false);
}

// Determine which move will move the robot to a square with a lower distance to the center
MouseMovement ourRobot::bestMove( bool frontWall, bool rightWall, bool leftWall, unsigned int x, unsigned int y) {
    if (!frontWall) {
        int forwardDistance = 1000;
        switch(heading) {
            case NORTH:
                if (y < MazeDefinitions::MAZE_LEN - 1) {
                    forwardDistance = distances[x][y+1];
                }
                break;
            case SOUTH:
                if (y > 0) {
                    forwardDistance = distances[x][y-1];
                }
                break;
            case EAST:
                if (x < MazeDefinitions::MAZE_LEN - 1) {
                    forwardDistance = distances[x+1][y];
                }
                break;
            case WEST:
                if (x > 0) {
                    forwardDistance = distances[x-1][y];
                }
                break;
            default: break;
        }
        if (forwardDistance < distances[x][y]) {
            return (MoveForward);
        }
    }
    if (!rightWall) {
        // Check if moving right decreases the distance
        int rightDistance = 1000;
        switch(heading) {
            case NORTH:
                if (x < MazeDefinitions::MAZE_LEN - 1) {
                    rightDistance = distances[x+1][y];
                }
                break;
            case SOUTH:
                if (x > 0) {
                    rightDistance = distances[x-1][y];
                }
                break;
            case EAST:
                if (y > 0) {
                    rightDistance = distances[x][y-1];
                }
                break;
            case WEST:
                if (y < MazeDefinitions::MAZE_LEN - 1) {
                    rightDistance = distances[x][y+1];
                }
                break;
            default: break;
        }
        if (rightDistance < distances[x][y]) {
            heading = clockwise(heading);
            return (TurnClockwise);
        }
    }
    if (!leftWall) {
        // Check if moving left decreases the distance
        int leftDistance = 1000;
        switch(heading) {
            case NORTH:
                if (x > 0) {
                    leftDistance = distances[x-1][y];
                }
                break;
            case SOUTH:
                if (x < MazeDefinitions::MAZE_LEN - 1) {
                    leftDistance = distances[x+1][y];
                }
                break;
            case EAST:
                if (y < MazeDefinitions::MAZE_LEN - 1) {
                    leftDistance = distances[x][y+1];
                }
                break;
            case WEST:
                if (y > 0) {
                    leftDistance = distances[x][y-1];
                }
                break;
            default: break;
        }
        if (leftDistance < distances[x][y]) {
            heading = counterClockwise(heading);
            return (TurnCounterClockwise);
        }
    }
    // Check if moving backward decreases the distance
    int backwardDistance = 1000;
    switch(heading) {
        case NORTH:
            if (y > 0) {
                backwardDistance = distances[x][y-1];
            }
            break;
        case SOUTH:
            if (y < MazeDefinitions::MAZE_LEN - 1) {
                backwardDistance = distances[x][y+1];
            }
            break;
        case EAST:
            if (x > 0) {
                backwardDistance = distances[x-1][y];
            }
            break;
        case WEST:
            if (x < MazeDefinitions::MAZE_LEN - 1) {
                backwardDistance = distances[x+1][y];
            }
            break;
        default: break;
    }
    if (backwardDistance < distances[x][y]) {
        return (MoveBackward);
    }
    return(Wait);
}

void ourRobot::printMaze() {
    std::string output;
    for (int y = MazeDefinitions::MAZE_LEN - 1; y >= 0 ; y--) {
        // horizontal walls above the current row
        output += "  ";
        for (int x = 0; x < MazeDefinitions::MAZE_LEN; x++) {
            if (horizontalWalls[x][y+1]) {
                output += "---";
            } else {
                output += "   ";
            }
            output += "    ";
        }
        output += '\n';
        for (int x = 0; x < MazeDefinitions::MAZE_LEN; x++) {
            if (verticalWalls[x][y]) {
                output += "|  ";
            } else {
                output += "   ";
            }
            output += std::to_string(distances[x][y]);
            if (distances[x][y] < 10) {
                output += " ";
            }
            output += "  ";
        }
        // rightmost wall
        if (verticalWalls[MazeDefinitions::MAZE_LEN][y]) {
            output += " | ";
        } else {
            output += "   ";
        }
        output += '\n';
    }
    // print out the bottom walls
    output += "  ";
    for (int x = 0; x < MazeDefinitions::MAZE_LEN; x++) {
        if (horizontalWalls[x][0]) {
            output += "---";
        } else {
            output += "   ";
        }
        output += "    ";
    }
    
    output += "\n\n";
    
    std::cout << output;
}
