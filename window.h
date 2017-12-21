#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

struct vpt{
    float x;
    float y;
    float z;
};

class Curve{
public:
    int resolution;
    vector< vector<vpt> > bezPoints;
    vector<vpt> curvePoints;
    vector<int> knots;
    vpt color;
    Curve(int res, vpt col): resolution(res), color(col){
        
    }
    void drawBez();
    void drawBspline();
};
