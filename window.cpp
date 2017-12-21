/*
 * Simple glut demo that can be used as a template for
 * other projects by Garrett Aldrich
 */


#ifdef WIN32
#include <windows.h>
#endif

#if defined (__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#else //linux
#include <GL/gl.h>
#include <GL/glut.h>
#endif

//other includes
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <cctype>
#include <fstream>

#include "window.h"
using namespace std;

/****set in main()****/
//the number of pixels in the grid
int grid_width;
int grid_height;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int win_height;
int win_width;

void init();
void idle();
void display();
void draw_lines();
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();

class Button{
public:
    
    Button(string nm, float xc, float yc, float length, float breadth, bool sel):name(nm), x(xc), y(yc), l(length), b(breadth), selected(sel) {
        
    }
    
    string name;
    float x, y, l, b;  //top left x and y, l is vertical b is horizontal
    vpt color;
    bool selected;
    bool clicked(float x, float y);
};

//DRAW : display curve points as well as curve
//       begin computing curve as # of points > 3
//DISPLAY : only show curves
//EDIT : Once edit button (toggled) is clicked, show/hide curve points
//SAAVE : Save to file if scene has been edited since last save

//void createList(string filename);
//void createFile();

void printText();
void makeColorList();
void fillButtonList();
void drawButtonList();
void fillColButtonList();
void fillKnots();
void writeToFile();
void readFromFile();
void drawBezCurve(vector<vpt> bezPoints, vector<vpt> &curvePoints);
void drawBsplCurve(vector<vpt> dbpoints, vector<float> knots, vector<vpt> &curvePoints);

string filename;
string input;
float resolution;
int curve_num;
int point_num;
int col_idx;
vpt chosen_color;
vpt clickpoint;
int k;

vector<vector<vpt> > curves;            //Actual points on the curve
vector< vector<vpt> > curve_list;       //curve points
vector<vpt> color_list;
Button eknots("Edit Bspl", 0.86, 0.05, 0.05, 0.1, false);
vector<Button> button_list;
vector<Button> sel_colors;          //colors already selected and in use
vector<Button> avail_colors;        //colors available to draw a new curve
vector<vpt> temp_pt_list;
vector<vector<float> > knots;
vector<bool> typeBez;
vector<int> orders;

bool disp = false, save = false, edit = false, draw = false, res = false, color_chosen = false, drawing = false, editing = false, m_down = false, bez = true;


int main(int argc, char **argv)
{
    k = 3;
    resolution = 20.0;
    curve_num = 0;
    point_num = 0;
    //the number of pixels in the grid
    grid_width = 220;
    grid_height = 100;
    makeColorList();
    fillButtonList();
    fillColButtonList();
    //the size of pixels sets the inital window height and width
    //don't make the pixels too large or the screen size will be larger than
    //your display size
    pixel_size = 5;
    
    /*Window information*/
    win_height = grid_height*pixel_size;
    win_width = grid_width*pixel_size;
    
	/*Set up glut functions*/
    /** See https://www.opengl.org/resources/libraries/glut/spec3/spec3.html ***/
    
	glutInit(&argc,argv);
    
    if(argc > 1){
        filename = argv[1];
        //createList(filename);
    }
    
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    /*initialize variables, allocate memory, create buffers, etc. */
    //create window of size (win_width x win_height
    glutInitWindowSize(win_width,win_height);
    //windown title is "glut demo"
	glutCreateWindow("glut demo");
    
	/*defined glut callback functions*/
	glutDisplayFunc(display); //rendering calls here
	glutReshapeFunc(reshape); //update GL on window size change
	glutMouseFunc(mouse);     //mouse button events
	glutMotionFunc(motion);   //mouse movement events
	glutKeyboardFunc(key);    //Keyboard events
	glutIdleFunc(idle);       //Function called while program is sitting "idle"
    
    //initialize opengl variables
    init();
    //start glut event loop
	glutMainLoop();
	return 0;
}

void fillKnots(){
    vector<vector <float> > u(curve_list.size());
    for(int i = 0; i < curve_list.size(); i++){
        orders.push_back(k);
        vector<float> temp;
        for(int j = 0; j <= curve_list[i].size() + k; j++){
            temp.push_back((float)j);
        }
        u[i] = temp;
    }
    knots = u;
}

/*initialize gl stufff*/
void init()
{
    //set clear color (Default background to white)
	glClearColor(1.0,1.0,1.0,1.0);
    //checks for OpenGL errors
	check();
}

//called repeatedly when glut isn't doing anything else
void idle()
{
    //redraw the scene over and over again
	glutPostRedisplay();	
}

//this is where we render the screen
void display()
{
    //clears the screen
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
    //clears the opengl Modelview transformation matrix
	glLoadIdentity();
	
    draw_lines();
    printText();
    //blits the current opengl framebuffer on the screen
    glutSwapBuffers();
    //checks for opengl errors
	check();
}
void drawCurvePoints(){
    for(int i = 0; i < curves.size(); i++){
        glBegin(GL_LINE_STRIP);
        glColor3f(sel_colors[i].color.x, sel_colors[i].color.y, sel_colors[i].color.z);
        for(int j = 0; j < curves[i].size(); j++){
            glVertex2f(curves[i][j].x, curves[i][j].y);
            //printf("%f, %f\n",curves[i][j].x, curves[i][j].y);
        }
        glEnd();
    }
}

void drawPolygon(float x, float y, float l, float b, vpt color){        //l is vertical, b is horizontal
    glBegin(GL_POLYGON);
    glColor3f(color.x, color.y, color.z);
    glVertex2f(x, y);                   //upper left
    glVertex2f(x, y - l);
    glVertex2f(x + b, y - l);
    glVertex2f(x + b, y);
    glEnd();
}

void movePoint(float dx, float dy){
    temp_pt_list[point_num].x = dx;
    temp_pt_list[point_num].y = dy;
    curve_list[curve_num] = temp_pt_list;
    vector<vpt> curvePoints;
    if(bez){
        drawBezCurve(curve_list[curve_num], curvePoints);
    }
    else{
        
        drawBsplCurve(curve_list[curve_num], knots[curve_num], curvePoints);
    }
    curves[curve_num] = curvePoints;
    drawCurvePoints();
}

int pt_click(float x, float y){
    vector<vpt> curve = temp_pt_list;
    int pt = -1;
    for(int i = 0; i < curve.size(); i++){
        if( x >= curve[i].x - 0.003 && x <= curve[i].x + 0.003 && y >= curve[i].y - 0.006 && y <= curve[i].y +0.006){
            pt = i;
            break;
        }
    }
    return pt;
}

void drawPoints(){
    
    if(temp_pt_list.size() > 1){
        glBegin(GL_LINE_STRIP);
        glColor3f(chosen_color.x, chosen_color.y, chosen_color.z);
        for(int i = 0; i < temp_pt_list.size(); i++){
            vpt pt = temp_pt_list[i];
            glVertex2f(pt.x, pt.y);
        }
        glEnd();
    }
    for(int i = 0; i < temp_pt_list.size(); i++){
        vpt pt = temp_pt_list[i];
        drawPolygon(pt.x - 0.003, pt.y + 0.006, 0.012, 0.006, chosen_color);
    }
}

void add_point(float cx, float cy){
    vpt pt = {cx, cy, 0};
    if(draw){
        drawing = true;
        editing = false;
        temp_pt_list.push_back(pt);
    }
    if(edit){
        editing = true;
        drawing = false;
        temp_pt_list.clear();
        temp_pt_list = curve_list[curve_num];
        temp_pt_list.push_back(pt);
        curve_list[curve_num] = temp_pt_list;
    }
}

void drawTextBox(){
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0,0.0,0.0);
    glVertex2f(0.09,0.0);
    glVertex2f(0.6, 0.0);
    glVertex2f(0.6, 0.05);
    glVertex2f(0.09, 0.05);
    glVertex2f(0.09, 0.0);
    glEnd();
}

void draw_lines(){
    drawButtonList();
    drawTextBox();
    drawPoints();
    drawCurvePoints();
    if(res){
        cout << "Current resolution: " << resolution << "\n"
            <<"Please enter a new resolution: ";
        cin >> resolution;
        res = false;
        button_list[4].selected = false;
        for(int i = 0; i < curve_list.size(); i++){
            vector<vpt> curvePoints;
            if(bez){
                drawBezCurve(curve_list[i], curvePoints);
            }
            else{
                drawBsplCurve(curve_list[i], knots[i], curvePoints);
            }
            curves[i] = curvePoints;
        }
        drawCurvePoints();
    }
}

void reshape(int width, int height)
{

	win_width = width;
	win_height = height;
	glViewport(0,0,width,height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	glOrtho(0,1.0,0.0,1.0,-10,10);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    //check for opengl errors
    check();
}

void printText(){
    glRasterPos2f( 0.1, 0.01 );
    for ( int i = 0; i < input.size(); ++i ) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, input[i]);
    }
}

//keyboard function for the dialog box
void key(unsigned char ch, int x, int y)
{
    int n = ch;
	switch(ch)
	{
		default:
            if(!m_down){
                if( isalpha(ch) || n == 32 || (n >= 32 && n <= 64) ){
                    if(input.size() < 70)
                        input += ch;
                }
                else{
                    if(!input.empty())
                        input.erase(input.size() - 1, 1);
                }
                break;
            }
            else{
                if(editing){
                    float gx, gy;
                    gx = ((float)x) / ((float) win_width);
                    gy = ((float)(win_height-y))/((float)win_height);
                    int idx = pt_click(gx, gy);
                    if(idx >= 0){
                        if(ch == 'd'){
                            temp_pt_list.erase(temp_pt_list.begin() + idx);
                            curve_list[curve_num] = temp_pt_list;
                            vector<vpt> curvePoints;
                            if(bez){
                                drawBezCurve(curve_list[curve_num], curvePoints);
                            }
                            else{
                                drawBsplCurve(curve_list[curve_num], knots[curve_num], curvePoints);
                            }
                            curves[curve_num] = curvePoints;
                            drawCurvePoints();
                        }
                    }
                }
            }
    }
    //redraw the scene after keyboard input
	glutPostRedisplay();
}

void buttonUpListHandler(float gx, float gy){
    if(button_list[2].clicked(gx, gy)){
        button_list[2].selected = false;
    }
    if(button_list[3].clicked(gx, gy)){
        button_list[3].selected = false;
    }
}

void buttonDownListHandler(float x, float y){
    for(int i = 0; i < button_list.size(); i++){
        if(button_list[i].clicked(x, y)){
            button_list[i].selected = true;
            for(int j = 0; j < 4; j++){
                if(j != i){
                    button_list[j].selected = false;
                }
            }
        }
    }
    
    if(button_list[6].clicked(x, y)){
        bez = true;
        button_list[5].selected = false;
        button_list[6].selected = true;
    }
    if(button_list[5].clicked(x, y)){
        bez = false;
        button_list[5].selected = true;
        button_list[6].selected = false;
    }
    draw = button_list[0].selected;
    edit = button_list[1].selected;
    disp = button_list[2].selected;
    save = button_list[3].selected;
    res = button_list[4].selected;
    if(draw){
        for(int i = 0; i < avail_colors.size(); i++){
            if(avail_colors[i].clicked(x, y)){
                avail_colors[i].selected = true;
                chosen_color = avail_colors[i].color;
                col_idx = i;
                color_chosen = true;
                for(int j = 0; j < avail_colors.size(); j++){
                    if(j != i){
                        avail_colors[j].selected = false;
                    }
                }
                return;
            }
        }
    }
    else if(edit){
        for(int i = 0; i < sel_colors.size(); i++){
            if(sel_colors[i].clicked(x, y)){
                sel_colors[i].selected = true;
                chosen_color = sel_colors[i].color;
                color_chosen = true;
                curve_num = i;
                editing = true;
                temp_pt_list = curve_list[curve_num];
                for(int j = 0; j < sel_colors.size(); j++){
                    if(j != i){
                        sel_colors[j].selected = false;
                    }
                }
                return;
            }
        }
    }
    else if(disp){
        if(drawing){
            avail_colors[col_idx].selected = false;
            sel_colors.push_back(avail_colors[col_idx]);
            avail_colors.erase(avail_colors.begin() + col_idx);
            curve_list.push_back(temp_pt_list);
            fillKnots();
            vector<vpt> curvePoints;
            typeBez.push_back(bez);
            if(bez){
                drawBezCurve(curve_list[curve_list.size() - 1], curvePoints);
            }
            else{
                drawBsplCurve(curve_list[curve_list.size() - 1], knots[curve_list.size() - 1], curvePoints);
            }
            curves.push_back(curvePoints);
            drawCurvePoints();
        }
        if(editing){
            sel_colors[curve_num].selected = false;
            curve_list[curve_num] = temp_pt_list;
            vector<vpt> curvePoints;
            if(bez){
                drawBezCurve(curve_list[curve_num], curvePoints);
            }
            else{
                drawBsplCurve(curve_list[curve_num], knots[curve_num], curvePoints);
            }
            curves[curve_num] = curvePoints;
            drawCurvePoints();
        }
        drawing = false;
        editing = false;
        color_chosen = false;
        temp_pt_list.clear();
    }
    else if(save){
        if(input.size() > 0){
            writeToFile();
        }
    }
    else{
        color_chosen = false;
    }
}


//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
    float gx, gy;
    gx = ((float)x) / ((float) win_width);
    gy = ((float)(win_height-y))/((float)win_height);
    if(state == GLUT_DOWN){
        m_down = true;
        if(gx < 0.833){
            if(color_chosen){
                int pt = pt_click(gx, gy);
                if(pt < 0){
                    add_point(gx, gy);
                }
                else{
                    if(editing){
                        clickpoint = curve_list[curve_num][pt];
                        point_num = pt;
                    }
                }
            }
        }
        else{
            buttonDownListHandler(gx, gy);
        }
        //redraw the scene after mouse click
    }
    else{
        m_down = false;
        if(gx >= 0.833){
            buttonUpListHandler(gx, gy);
        }
    }
    glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
    if(editing){
        float gx, gy;
        gx = ((float)x) / ((float) win_width);
        gy = ((float)(win_height-y))/((float)win_height);
        if(gx < 0.83 && gx >= 0.003 && gy <= 0.994 && gy >= 0.006)
            movePoint(gx, gy);
    }
    //redraw the scene after mouse movement
	glutPostRedisplay();
}

//checks for any opengl errors in the previous calls and
//outputs if they are present
void check()
{
	GLenum err = glGetError();
	if(err != GL_NO_ERROR)
	{
		printf("GLERROR: There was an error %s\n",gluErrorString(err) );
		exit(1);
	}
}

void makeColorList(){
    vpt red = { 1.0, 0, 0 };
    vpt orange = { 1.0, 0.5, 0};
    vpt yellow = { 1.0, 1.0, 0};
    vpt lgreen = { 0.5, 1.0, 0};
    vpt green = { 0, 1.0, 0};
    vpt sgreen = { 0, 1.0, 0.5};
    vpt lblue = { 0, 1.0, 1.0};
    vpt blue = { 0, 0.5, 1.0};
    vpt dblue = { 0, 0, 1.0};
    vpt indigo = {0.5, 0, 1.0};
    vpt pink = {1.0, 0, 1.0};
    vpt magenta = { 1.0, 0, 0.5};
    color_list.push_back(red);
    color_list.push_back(orange);
    color_list.push_back(yellow);
    color_list.push_back(lgreen);
    color_list.push_back(green);
    color_list.push_back(sgreen);
    color_list.push_back(lblue);
    color_list.push_back(blue);
    color_list.push_back(dblue);
    color_list.push_back(indigo);
    color_list.push_back(pink);
    color_list.push_back(magenta);
}

void drawButton(Button bu){
    vpt color;
    if(bu.selected){
        vpt c = {0.461, 0.461, 0.461};
        color = c;
    }
    else{
        vpt c = {0.92, 0.92, 0.92};
        color = c;
    }
    drawPolygon(bu.x, bu.y, bu.l, bu.b, color);
    glBegin(GL_LINE_STRIP);
    glColor3f(0.0, 0.0, 0.0);
    glVertex2f(bu.x, bu.y);
    glVertex2f(bu.x, bu.y-bu.l);
    glVertex2f(bu.x+bu.b, bu.y-bu.l);
    glEnd();
    glRasterPos2f(bu.x + (bu.b * 0.27), bu.y - (bu.l * 0.7));
    for(int i = 0; i < bu.name.size(); i++){
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, bu.name[i]);
    }
}

void drawColButton(Button bu){
    if(bu.selected){
        glColor3f(0.0, 0.0, 0.0);
        glBegin(GL_LINE_LOOP);
        glVertex2f(bu.x, bu.y);
        glVertex2f(bu.x, bu.y - bu.l);
        glVertex2f(bu.x + bu.b, bu.y - bu.l);
        glVertex2f(bu.x + bu.b, bu.y);
        glEnd();
    }
    drawPolygon(bu.x, bu.y, bu.l, bu.b, bu.color);
}

void drawButtonList(){
    glLineWidth(2.0);
    glBegin(GL_LINES);
    glColor3f(0.0,0.0,0.0);
    glVertex2f(0.833,0);
    glVertex2f(0.833, 1.0);
    glEnd();
    glBegin(GL_POLYGON);
    glColor3f(0.86,0.86,0.86);
    glVertex2f(0.833, 1.0);
    glVertex2f(0.833, 0);
    glVertex2f(1.0, 0);
    glVertex2f(1.0, 1.0);
    glEnd();
    for(int i = 0; i < button_list.size(); i++){
        drawButton(button_list[i]);
    }
    if(draw){
        if(!drawing){
            for(int i = 0; i < avail_colors.size(); i++){
                drawColButton(avail_colors[i]);
            }
        }
        else{
            drawColButton(avail_colors[col_idx]);
        }
    }
    if(edit){
        for(int i = 0; i < sel_colors.size(); i++){
            drawColButton(sel_colors[i]);
        }
    }
}

void fillButtonList(){
    Button dr("DRAW", 0.859, 0.75, 0.1, 0.11, false);
    Button ed("EDIT", 0.859, 0.6, 0.1, 0.11, false);
    Button di("DISPLAY", 0.859, 0.45, 0.1, 0.11, false);
    Button sa("SAVE", 0.859, 0.3, 0.1, 0.11, false);
    
    Button res("Res", 0.885, 0.86, 0.05, 0.05, false);
    Button bspl("BSp", 0.845, 0.95, 0.05, 0.05, false);
    Button bez("Bez", 0.93, 0.95, 0.05, 0.05, true);
    
    button_list.push_back(dr);
    button_list.push_back(ed);
    button_list.push_back(di);
    button_list.push_back(sa);
    button_list.push_back(res);
    
    button_list.push_back(bspl);
    button_list.push_back(bez);
}

//called once
void fillColButtonList(){
    float x = 0.0, y = 0.0;
    for(int i = 0; i < color_list.size(); i++){
        Button cB("", 0.836 + x, 0.15 + y, 0.03, 0.016, false);
        cB.color = color_list[i];
        avail_colors.push_back(cB);
        if((0.833 + x) <= 1.0){
            x += 0.021;
        }
        else{
            x = 0.0;
            y -= 0.05;
        }
    }
}

bool Button::clicked(float xc, float yc){
    if( (xc >= x && xc <= x + b) && (yc >= y - l && yc <= y) ){
        return true;
    }
    return false;
}

void calcBezPoint(vector<vector <vpt> >&dv, float t){
    for(int j = 1; j < dv[0].size(); j++){
        vector<vpt> temp(dv[j-1].size());
        for(int i = 0; i < dv[0].size() - j ; i++){
            temp[i].x = ((1 - t) * dv[j-1][i].x) + (t * dv[j-1][i+1].x);
            temp[i].y = ((1 - t) * dv[j-1][i].y) + (t * dv[j-1][i+1].y);
        }
        dv.push_back(temp);
    }
}

void drawBezCurve(vector<vpt> bezPoints, vector<vpt> &curvePoints){
    vector< vector <vpt> > dv;
    dv.push_back(bezPoints);
    for(int j = 0; j <= resolution; j++){
        float t = 0;
        t = (j * (1.0 / resolution));
        calcBezPoint(dv, t);
        curvePoints.push_back(dv[dv.size() - 1][0]);
        dv.clear();
        dv.push_back(bezPoints);
    }
}

void calcBezPoint(vector< vector <vpt> > &dv, float u, vector<float> knots, int idx){
    int kk = orders[curve_num];
    for(int j = 1; j < kk; j++){
        vector<vpt> temp(dv[0].size());
        for(int i = idx - kk + 1; i <= idx - j; i++){
            temp[i].x = (((knots[i + kk] - u) / (knots[i+kk] - knots[i+j])) * dv[j-1][i].x)  +
                        (((u - knots[i + j])/(knots[i + kk] - knots[i + j])) * dv[j-1][i+1].x);
            temp[i].y = (((knots[i + kk] - u) / (knots[i+kk] - knots[i+j])) * dv[j-1][i].y)  +
                        (((u - knots[i + j])/(knots[i + kk] - knots[i + j])) * dv[j-1][i+1].y);
        }
        dv.push_back(temp);
    }
    
}


void drawBsplCurve(vector<vpt> dbPoints, vector<float> knots, vector<vpt> &curvePoints){
    
    float d = (knots[dbPoints.size()] - knots[k - 1]) / resolution;
    for(float u = knots[k - 1]; u < dbPoints.size(); u += d){
        int idx;
        vector< vector <vpt> > dv;
        dv.push_back(dbPoints);
        for(int i = k - 1; i < dbPoints.size(); i++){
            if(u >= knots[i] && u < knots[i+1]){
                idx = i;
                break;
            }
        }
        calcBezPoint(dv, u, knots, idx);
        curvePoints.push_back(dv[k - 1][idx - k + 1]);
    }
}

//number of curves, color, type, order, knots, curve points(for bspl)
void writeToFile(){
    ofstream ofile(input.c_str());
    ofile << curve_list.size() << "\n";
    for(int i = 0; i < curve_list.size(); i++){
        ofile << sel_colors[i].color.x << " "
            <<sel_colors[i].color.y << " "
            <<sel_colors[i].color.z << "\n";
        if(typeBez[i]){
            ofile << "1" << "\n";
        }
        else{
            ofile << "2" << "\n";
            ofile << k << "\n";
            for(int j = 0; j < knots[i].size(); j++){
                ofile << knots[i][j] << " ";
            }
            ofile << "\n";
        }
        for(int j = 0; j < curve_list[i].size(); j++){
            ofile << curve_list[i][j].x << " "
                    <<curve_list[i][j].y << " "
                    <<"\n";
        }
        
    }
}

