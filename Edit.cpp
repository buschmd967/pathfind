
/* Copyright (c) Mark J. Kilgard, 1994. */

/* This program is freely distributable without licensing fees 
   and is provided without guarantee or warrantee expressed or 
   implied. This program is -not- in the public domain. */

#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <GL/freeglut.h> 
//#include "freeglut/include/GL/freeglut.h"
#include "grid.h"
#include "pathfind.cpp"

//grid options
int row_size = 30;
int col_size = 40;
Grid g(row_size, col_size, 1);
Grid backup_grid(row_size, col_size, 1);

//window values
int wWidth = 1600;
int wHeight = 900;

int borderHeight = 200;
int borderWidth = wWidth;
int bheight = wHeight - 200;
int bwidth = wWidth;

int width = wWidth;
int height = wHeight - borderHeight;

float gridWidth = (float) width / ((float) col_size - 2.0);
float gridHeight = (float) height / ((float) row_size - 2.0);

//string edit
bool stringEditMode = false;
std::string file_name = "";

//keyboard values
int currentKey = -1;
int lastKey = -1;

//mouse values
int mx = -1; //mouse X pos
int my = -1; // mouse Y pos
int mb = -1; // mouse button
int ms = 0; // mouse state
bool buttonClicked = false;

//Common Tests
bool fileExists = false;

bool greenExists = false; 
bool redExists = false;

bool can_edit = true;

//===Algorithm Helper Vars===
int greenCount = 0;
int redCount = 0;
int steps = 0;
int found_after_steps = -1;
bool steps_update = false;

//A*
int start_index = -1;
int end_index = -1;
int min_hcost = -1;
int min_fcost = -1;
int min_fcost_index = -1;
bool running_Astar = false;


//Box drawing global values
int currentIndex = -1;
std::vector<int> changedIndexes;

int changingValue = -1;
int currentToggleValue = -1;
int brushValue = -1;

//Font stuff
void *font = GLUT_BITMAP_TIMES_ROMAN_24;
void *fonts[] =
{
  GLUT_BITMAP_9_BY_15,
  GLUT_BITMAP_TIMES_ROMAN_10,
  GLUT_BITMAP_TIMES_ROMAN_24
};
char defaultMessage[] = "GLUT means OpenGL.";
char *message = defaultMessage;

void
selectFont(int newfont)
{
  font = fonts[newfont];
  glutPostRedisplay();
}


void
selectColor(int color)
{
  switch (color) {
  case 1:
    glColor3f(0.0, 1.0, 0.0);
    break;
  case 2:
    glColor3f(1.0, 0.0, 0.0);
    break;
  case 3:
    glColor3f(1.0, 1.0, 1.0);
    break;
  }
  glutPostRedisplay();
}

void
tick(void) {
  glutPostRedisplay();
}

void
output(int x, int y, char *string)
{
  int len, i;

  glRasterPos2f(x, y);
  len = (int) strlen(string);
  for (i = 0; i < len; i++) {
    glutBitmapCharacter(font, string[i]);
  }
}


//gets grid index of cell given screen x and y pos
int getMouseIndex(int x = mx, int y = my){
	int col = x / gridWidth;
	int row = y / gridHeight;
	int index = row * col_size + col + 1 + col_size;
	if(index >= g.size()){
		return -1;
	}
	else{
		return index;
	}
	
}

void drawText(int x, int y, const std::string & string, void* font = GLUT_BITMAP_8_BY_13){
	glRasterPos2f(x, y);
	glutBitmapString(font, (const unsigned char *) string.c_str());
}

void drawTextInBox(int index, const std::string & string){
	index -= (1 + col_size);
	float y = index / col_size * (float) gridHeight;
	float x = index % col_size  * (float) gridWidth;
	y += 2 * gridHeight / 3;
	x += gridWidth / 2 - 4 * string.size();
	//std::cout << "x: " << x << " y: " << y << std::endl;
	glColor3f(0,0,0);
	drawText(x, y, string);
}

void check_file(){
	//Check if file exists
	std::ifstream file_check(file_name);
	if(file_check && file_name.substr(file_name.size() - 4, file_name.size() - 1) != ".cpp"){
		fileExists = true;
	}
	else{
		fileExists = false;
	}
	file_check.close();


}

void recalculate_window_vals(){
	/*
	wHeight = glutGet(GLUT_WINDOW_HEIGHT);
	wWidth = glutGet(GLUT_WINDOW_WIDTH);
	*/

	//borderHeight =((float) 1/ (float) 5) *  wHeight;
	borderHeight = 200;
	borderWidth = wWidth;
	bheight = wHeight - borderHeight;
	bwidth = wWidth;

	width = wWidth;
	height = wHeight - borderHeight;

	gridWidth = (float) width / ((float) col_size - 2.0);
	gridHeight = (float) height / ((float) row_size - 2.0);

	//reshape(wWidth, wHeight);
}

void
reshape(int w, int h)
{
  wWidth = w;
  wHeight = h;

  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLdouble) wWidth, (GLdouble) wHeight, 0.0);
  //gluOrtho2D(0, w, 0, h);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  recalculate_window_vals();
}


void run_algs(){

	if(running_Astar){
		if(steps > 0 && steps_update){
			g = backup_grid;
			for(int i = steps; i > 0; i--){
				std::cout << "Steps: " << i << std::endl;
				min_fcost_index = pathfind::astar_find_target_index(g);
				if(pathfind::Astar(g, start_index, end_index, min_fcost_index)){
					found_after_steps = steps - i + 1;
					steps = found_after_steps;
					break;
				}
			}

			steps_update = false;
		}
		else if (steps < 0 && steps_update){
			found_after_steps++;
			if(pathfind::Astar(g, start_index, end_index, min_fcost_index)){
				steps = found_after_steps;
				steps_update = false;
			}
		}
		else{
			steps_update = false;
		}

	}
}

void buttons(){

	//handle left click toggle color
	if(ms == 0 && mb == 0){
		if(stringEditMode){
		stringEditMode = false;
		}
		if(my <= height && can_edit){ // in grid window
			int index = getMouseIndex();
			if(index != currentIndex && index >= 0){

				//check to see if box has already been changed
				bool newIndex = true;
				for(int x: changedIndexes){
					if(x == index){
						newIndex = false;
						break;
					}
				}
				
				//switch color
				if(newIndex){
					currentIndex = index;
					changedIndexes.push_back(currentIndex);
					auto a = g.coord(index);
					//std::cout << a.first << ", " << a.second << std::endl;

					int currentValue = g[index];
					if(changingValue == -1){
						changingValue = currentValue;
					}

					if(brushValue == -1){
						if(currentToggleValue < 0){
							currentToggleValue = currentValue;
						}
						if(g[index] == currentToggleValue){
							g[index] = !currentToggleValue;
						}
					}
					else{
						if(currentValue == brushValue){
							g[index] = 1;
						}
						else if(currentValue == changingValue){
							g[index] = brushValue;	
						}
					}
				}
			}
			//std::cout << g[index] << std::endl;
		}
		else { // normal button in border window
			//Check if want to change file name
			if(mx <= 85 + 8 * file_name.size() && my >= wHeight - 20){
				//std::cout << "ON FILE NAME" << std::endl;
				buttonClicked = true;
				stringEditMode = true;
			}
			if(!buttonClicked){
				//===Brushes===

				//Black/White Brush
				if(mx >= 5 && mx <= 55 && my >= bheight + 5 && my <= bheight + 55){
					brushValue = -1;
					buttonClicked = true;
				}
				//Green Brush
				if(mx >= 60 && mx <= 110 && my >= bheight + 5 && my <= bheight + 55){
					brushValue = 2;
					buttonClicked = true;
				}

				//Red Brush
				if(mx >= 115 && mx <= 165 && my >= bheight + 5 && my <= bheight + 55){
					brushValue = 3;
					buttonClicked = true;
				}

				//===Algorithms===
				//prev
				if(steps > 0 && mx >= bwidth / 2 - 80 && mx <= bwidth / 2 - 30 && my >= wHeight - 55 && my <= wHeight - 5){
					buttonClicked = true;
					std::cout << "--" << std::endl;
					steps -= 1;	
					steps_update = true;
				}
					
				//nav play
				if(mx >= bwidth / 2 - 25 && mx <= bwidth / 2 + 25 && my >= wHeight - 55 && my <= wHeight - 5){
					buttonClicked = true;
					std::cout << "Go" << std::endl;
					steps = -1;	
					steps_update = true;
				}

				//next
				if(mx >= bwidth / 2 + 30 && mx <= bwidth / 2 + 80 && my >= wHeight - 55 && my <= wHeight - 5){
					buttonClicked = true;
					std::cout << "++" << std::endl;
					steps += 1;	
					steps_update = true;
				}
					
					
				//A*
				if(mx >= 5 && mx <= 55 && my >= bheight+60 && my <= bheight + 110){
					buttonClicked = true;
					//std::cout << "min_fcost: " << min_fcost << " min_fcost_index: " << min_fcost_index << std::endl;
					if(running_Astar){
						running_Astar = false;
						g = backup_grid;
						//pathfind::Astar(g, start_index, end_index, min_fcost_index);
					}
					else{
						backup_grid = g;
						min_fcost_index = start_index;
						//pathfind::Astar(g, start_index, end_index, start_index);
						running_Astar = true;
					}
					min_fcost = -1;
				}


				//Save
				if(mx >= 5 && mx <= 65 && my >= wHeight - 60 && my <= wHeight - 30){
					buttonClicked = true;
					g.save_to_file(file_name.c_str());
					check_file();
				}
				//Load
				if(fileExists && 75 <= mx && mx <= 135 && my >= wHeight - 60 && my <= wHeight - 30 ){
					buttonClicked = true;
					g.load_from_file(file_name.c_str());
					row_size = g.row_size();
					col_size = g.col_size();
					gridWidth = (float) width / ((float) col_size - 2.0);
					gridHeight = (float) height / ((float) row_size - 2.0);

					greenExists = g.val_exists(2);
					redExists = g.val_exists(3);




			glVertex2f(5, bheight + 60);
			glVertex2f(55, bheight + 60);
			glVertex2f(55, bheight + 110);
			glVertex2f(5, bheight + 110);



				}
			}

		}

	}

	//On mouse up
	if(ms == 1){
		ms = -1;
		buttonClicked = false;
		//std::cout << "Clearing" << std::endl;
		changedIndexes.clear();
		currentToggleValue = -1;
		changingValue = -1;
		currentIndex = -1;
	}



}

void borderMenu(void){

	//Draw Border Stuff
	glColor3f(.5,.5,.5);
	glBegin(GL_POLYGON);
		glVertex3f(0, bheight, 0);
		glVertex3f(bwidth, bheight, 0);
		glVertex3f(bwidth, wHeight, 0);
		glVertex3f(0, wHeight, 0);
	glEnd();


	//Border Filename
	if(stringEditMode){
		if(currentKey > 0){
			//std::cout << "string edit, key: " << currentKey << std::endl;
			if(currentKey > 31 && currentKey < 127){
				//std::cout << "adding char" << std::endl;
				file_name += (char) currentKey;
			}
			else{
				switch(currentKey){
					case 127: //backspace
					case 8: // delete
					if(file_name.size() > 0){
						file_name = file_name.substr(0, file_name.size() - 1);
					}
					break;
					case 27: //escape
					case 13: //enter
					stringEditMode = false;
				}
			}
			lastKey = currentKey;
			currentKey = -1;
		}
		check_file();
	}

	//draw filename text
	if(!stringEditMode){
		glColor3f(1, 0, 0);
	}
	else{
		glColor3f(0, 0, 1);
	}
	drawText(5, wHeight - 10, "filename: ");

	if(fileExists){
		glColor3f(0, 1, 0);
	}

	drawText(80, wHeight - 10, file_name);

	//Brush Buttons

	//Black/White
	if(brushValue == -1){
		glColor3f(0, 0, 1);
	}
	else{
		glColor3f(.75, .75, .75);

	}
	glBegin(GL_POLYGON);
		glVertex2f(5, bheight+5);
		glVertex2f(55, bheight+5);
		glVertex2f(55, bheight+55);
		glVertex2f(5, bheight+55);
	glEnd();

	glColor3f(0, 0, 0);
	glBegin(GL_POLYGON);
		glVertex2f(7, bheight+7);
		glVertex2f(53, bheight+7);
		glVertex2f(7, bheight+53);
	glEnd();

	glColor3f(1, 1, 1);
	glBegin(GL_POLYGON);
		glVertex2f(53, bheight+7);
		glVertex2f(53, bheight+53);
		glVertex2f(7, bheight+53);
	glEnd();

	//Green
	if(brushValue == 2){
		glColor3f(0, 0, 1);
	}
	else{
		glColor3f(.75, .75, .75);
	}
	glBegin(GL_POLYGON);
		glVertex2f(60, bheight+5);
		glVertex2f(110, bheight+5);
		glVertex2f(110, bheight+55);
		glVertex2f(60, bheight+55);
	glEnd();

	glColor3f(0, 1, 0);
	glBegin(GL_POLYGON);
		glVertex2f(62, bheight+7);
		glVertex2f(108, bheight+7);
		glVertex2f(108, bheight+53);
		glVertex2f(62, bheight+53);
	glEnd();

	//Red
	if(brushValue == 3){
		glColor3f(0, 0, 1);
	}
	else{
		glColor3f(.75, .75, .75);
	}
	glBegin(GL_POLYGON);
		glVertex2f(115, bheight+5);
		glVertex2f(165, bheight+5);
		glVertex2f(165, bheight+55);
		glVertex2f(115, bheight+55);
	glEnd();

	glColor3f(1, 0, 0);
	glBegin(GL_POLYGON);
		glVertex2f(117, bheight+7);
		glVertex2f(163, bheight+7);
		glVertex2f(163, bheight+53);
		glVertex2f(117, bheight+53);
	glEnd();


	bool draw_nav;
	draw_nav = running_Astar;

	can_edit = !draw_nav;

	//Algorithm Buttons

	if(draw_nav){
		// <
		glColor3f(.75,.75,.75);
		glBegin(GL_POLYGON);
			glVertex2f(bwidth / 2 - 80, wHeight -55);
			glVertex2f(bwidth / 2 - 30, wHeight -55);
			glVertex2f(bwidth / 2 - 30, wHeight -5);
			glVertex2f(bwidth / 2 - 80, wHeight -5);
		glEnd();

		glColor3f(0,0,0);
		drawText(bwidth / 2 - 60, wHeight - 20, "<", GLUT_BITMAP_TIMES_ROMAN_24);

		// Play
		glColor3f(.75,.75,.75);
		glBegin(GL_POLYGON);
			glVertex2f(bwidth / 2 - 25, wHeight -55);
			glVertex2f(bwidth / 2 + 25, wHeight -55);
			glVertex2f(bwidth / 2 + 25, wHeight -5);
			glVertex2f(bwidth / 2 - 25, wHeight -5);
		glEnd();

		glColor3f(0,0,0);
		drawText(bwidth / 2 - 10, wHeight - 20, "Go", GLUT_BITMAP_TIMES_ROMAN_24);

		// >
		glColor3f(.75,.75,.75);
		glBegin(GL_POLYGON);
			glVertex2f(bwidth / 2 + 30, wHeight -55);
			glVertex2f(bwidth / 2 + 80, wHeight -55);
			glVertex2f(bwidth / 2 + 80, wHeight -5);
			glVertex2f(bwidth / 2 + 30, wHeight -5);
		glEnd();

		glColor3f(0,0,0);
		drawText(bwidth / 2 + 60, wHeight - 20, ">", GLUT_BITMAP_TIMES_ROMAN_24);

	}

	if(greenExists && redExists){
		//A*
		if(!running_Astar){
			glColor3f(.75,.75,.75);
			glBegin(GL_POLYGON);
				glVertex2f(5, bheight + 60);
				glVertex2f(55, bheight + 60);
				glVertex2f(55, bheight + 110);
				glVertex2f(5, bheight + 110);
			glEnd();

		}
		else{
			glColor3f(0,0,1);
			glBegin(GL_POLYGON);
				glVertex2f(5, bheight + 60);
				glVertex2f(55, bheight + 60);
				glVertex2f(55, bheight + 110);
				glVertex2f(5, bheight + 110);
			glEnd();

			glColor3f(0.75,0.75,0.75);
			glBegin(GL_POLYGON);
				glVertex2f(7, bheight + 62);
				glVertex2f(53, bheight + 62);
				glVertex2f(53, bheight + 108);
				glVertex2f(7, bheight + 108);
			glEnd();

		}

		glColor3f(0,0,1);
		drawText(19, bheight + 94, "A*", GLUT_BITMAP_TIMES_ROMAN_24);
  
	}


	//Save Button
	glColor3f(0,0,0);
	//top
	glBegin(GL_LINES);
		glVertex2f(5, wHeight - 60);
		glVertex2d(65, wHeight - 60);
	glEnd();

	//left
	glBegin(GL_LINES);
		glVertex2f(5, wHeight - 61);
		glVertex2d(5, wHeight - 30);
	glEnd();

	//bottom
	glBegin(GL_LINES);
		glVertex2f(5, wHeight - 30);
		glVertex2d(65, wHeight - 30);
	glEnd();

	//right
	glBegin(GL_LINES);
		glVertex2f(65, wHeight - 61);
		glVertex2d(65, wHeight - 30);
	glEnd();

	glRasterPos2f(17, wHeight - 41);
	glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char *) "Save");

	if(fileExists){
		//Load Button
		glColor3f(0,0,0);
		//top
		glBegin(GL_LINES);
			glVertex2f(75, wHeight - 60);
			glVertex2d(135, wHeight - 60);
		glEnd();

		//left
		glBegin(GL_LINES);
			glVertex2f(75, wHeight - 61);
			glVertex2d(75, wHeight - 30);
		glEnd();

		//bottom
		glBegin(GL_LINES);
			glVertex2f(75, wHeight - 30);
			glVertex2d(135, wHeight - 30);
		glEnd();

		//right
		glBegin(GL_LINES);
			glVertex2f(135, wHeight - 61);
			glVertex2d(135, wHeight - 30);
		glEnd();


		glRasterPos2f(89, wHeight - 41);
		glutBitmapString(GLUT_BITMAP_8_BY_13, (const unsigned char *) "Load");
	}

}

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);


	//Draw Boxes
	for(float i=0; i < width; i += gridWidth){
		glBegin(GL_LINES);
			glColor3f(0,0,1);
			glVertex2f(i, 0);
			glVertex2f(i, height);
		glEnd();
	}
	for(float i=0; i < height; i += gridHeight){
		glBegin(GL_LINES);
			glColor3f(0,0,1);
			glVertex2f(0, i);
			glVertex2f(width, i);
		glEnd();
	}

	//Draw Values
	/*
	for(int x = gridWidth/2; x < width; x += gridWidth){
		for(int y = gridHeight/2; y < height; y += gridHeight){
			output(x, y, "0");
		}
	}
	*/

	//Draw Box colors, New scope because of the necessary index variable, which could get mixed up later on
	{
	greenCount = 0;
	redCount = 0;
	int index = 0;
	min_fcost = -1;
	for(float y = 0; y < height; y += gridHeight){
		for(float x = 0; x < width; x += gridWidth){
			/*
			if(running_Astar && steps != 0){
				std::string a;
				std::getline(std::cin, a);
				glutSwapBuffers();
			}
			*/

			index = getMouseIndex(x+1,y+1);
			bool draw_fcost = false;
			min_hcost = -1;

			//switch color
			switch(g[index]){
			case 0: // Black Border
				glColor3f(0,0,0);
				break;
			case 1: // White Space
				glColor3f(1,1,1);
				break;
			case 2: // Green Start
				glColor3f(0,1,0);
				++greenCount;
				start_index = index;
				break;
			case 3: // Red Finish
				glColor3f(1,0,0);
				++redCount;
				end_index = index;
				break;
			case 4: // Cyan Possibility
				glColor3f(0,1,1);
				draw_fcost = true;

				if(min_fcost == g[index].fcost() && min_hcost > g[index].hcost()){
					min_fcost_index = index;
					min_hcost = g[index].hcost();
				}
				else if(min_fcost > g[index].fcost() || min_fcost == -1){
					min_fcost_index = index;
					min_fcost = g[index].fcost();
					min_hcost = g[index].hcost();
				}
				break;
			case 5: // Purple Evaluated
				glColor3f(1,0,1);
				draw_fcost = true;
				break;
			case 6: // Blue Found
				glColor3f(0,0,1);
				break;
			default:
				glColor3f(.5,.5,.5);
				glBegin(GL_POLYGON);
					int off = 1;
					glVertex3f(x+off, y+off, 0);
					glVertex3f(x + gridWidth-off, y+off, 0);
					glVertex3f(x + gridWidth-off, y + gridHeight-off, 0);
					glVertex3f(x+off, y+gridHeight-off, 0);
				glEnd();
				glColor3f(0,0,1);
				drawTextInBox(index, std::to_string(g[index]));


				continue;
			}

			glBegin(GL_POLYGON);
				int off = 1;
				glVertex3f(x+off, y+off, 0);
				glVertex3f(x + gridWidth-off, y+off, 0);
				glVertex3f(x + gridWidth-off, y + gridHeight-off, 0);
				glVertex3f(x+off, y+gridHeight-off, 0);
			glEnd();

			if(draw_fcost){
				glColor3f(0,0,1);
				drawTextInBox(index, std::to_string(g[index].fcost()));
			}
		}
	}
	greenExists = (greenCount == 1);
	redExists = (redCount == 1);
	} // end of scope
	
	borderMenu();
	buttons();
	run_algs();
	/*
	glColor3f(0,0,1);
	for(int i = 0; i < g.size(); i++){
		drawTextInBox(i, std::to_string(i));
	}
	*/

	/*
	if(greenExists && redExists){
		glColor3f(0,0,1);
		for(int i = 0; i < g.size(); i++){
			drawTextInBox(i, std::to_string( (int) (g.distance(i, end_index) * 10)));
		}

	}

*/
//	recalculate_window_vals();
	
	std::cout << glutGet(GLUT_WINDOW_HEIGHT) << std::endl;

	glutSwapBuffers();
		
}



void mouseClicks(int button, int state, int x, int y){
	/*
	std::cout << "button : " << button << std::endl;
	std::cout << "state : " << state << std::endl;
	*/
	ms = state;
	mb = button;
}

void mouse_debug(int x, int y){

	mx = x;
	my = y;
	/*
	std::cout << "col: " << col << std::endl;
	std::cout << "row: " << row << std::endl;
	std::cout << "Mouse Move: "<< x << ", " << y << std::endl;
	*/
}
void menu(int i){
	switch(i){
	case 1: // Save Grid
		break;

	}
}

void keyPressed(unsigned char key, int x, int y){
	currentKey = key;
}

int
main(int argc, char **argv)
{
  g.reset_grid_border(1, 0);
  int i, msg_submenu, color_submenu;

  glutInit(&argc, argv);
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-mono")) {
      font = GLUT_BITMAP_9_BY_15;
    }
  }
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(wWidth, wHeight);
  glutCreateWindow("Test");
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  gluOrtho2D( 0.0, wWidth, wHeight, 0.0);

  glutMouseFunc(mouseClicks);
  glutDisplayFunc(display);
  glutMotionFunc(mouse_debug);
  glutPassiveMotionFunc(mouse_debug);
  glutReshapeFunc(reshape);
  glutIdleFunc(tick);
  glutKeyboardFunc(keyPressed);
  
  //msg_submenu = glutCreateMenu(menu);
  //glutAddMenuEntry("Save Grid", 1);
  //glutAddMenuEntry("ABC", 2);
  //color_submenu = glutCreateMenu(selectColor);
  //glutAddMenuEntry("Green", 1);
  //glutAddMenuEntry("Red", 2);
  //glutAddMenuEntry("White", 3);
  //glutCreateMenu(selectFont);
  //glutAddMenuEntry("9 by 15", 0);
  //glutAddMenuEntry("Times Roman 10", 1);
  //glutAddMenuEntry("Times Roman 24", 2);
  //glutAddSubMenu("Messages", msg_submenu);
  //glutAddSubMenu("Color", color_submenu);
  //glutAttachMenu(GLUT_RIGHT_BUTTON);
  glutMainLoop();
  return 0;             /* ANSI C requires main to return int. */
}
