#include "grid.h"
#include "cell.h"
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>

void Grid::reset_grid(int val = 0){
	int vector_size = _row_size * _col_size;
	for(int i = 0; i < vector_size; i++){
		_grid.push_back(val);
	}
}

Grid::Grid(int row_size, int col_size, int val){
	_row_size = row_size;
	_col_size = col_size;
	reset_grid(val);
}

Grid::Grid(int row_size, int col_size, const std::vector<Cell> & v){
	_row_size = row_size;
	_col_size = col_size;
	_grid = v;
}

int Grid::set_square(int row, int col, int val){
	int index = row * _col_size + col;
	int value = _grid[index];
	_grid[index] = val;
	return value;
}

void Grid::print() const {
	for(int row = 0; row < _row_size; row++){
		for(int col = 0; col < _col_size; col++){
			int index = row * _col_size + col;
			std::cout << _grid[index] << " ";
		}
		std::cout << std::endl;
	}
}

void Grid::save_to_file(const char * file_name) const{
	std::ofstream out(file_name, std::ofstream::out);
	out << _row_size << " " << _col_size;
	for(int x : _grid){
		out << " " << x;
	}
	out.close();
}

void Grid::load_from_file(const char * file_name){
	std::ifstream in(file_name);
	_grid.clear();
	in >> _row_size >> _col_size;
	int val;
	for(int i = 0; i < _row_size * _col_size; i++){
		in >> val;
		_grid.push_back(val);
	}

	in.close();
}

void Grid::reset_grid_border(int innerVal, int borderVal){
	_grid.clear();
	for(int i = 0; i < _col_size; i++){
		_grid.push_back(borderVal);
	}
	for(int row = 1; row+1 < _row_size; row++){
		_grid.push_back(borderVal);
		for(int col = 1; col + 1 < _col_size; col++){
			_grid.push_back(innerVal);
		}
		_grid.push_back(borderVal);
	}
	for(int i = 0; i < _col_size; i++){
		_grid.push_back(borderVal);
	}
}

bool Grid::val_exists(int val) const {
	for(int x: _grid){
		if(x == val){
			return true;
		}
	}
	return false;
}

std::pair<int, int> Grid::coord(int index) const{
	int y = index / _col_size;
	int x = index % _col_size;
	return {x, y};
}

float Grid::distance(int index1, int index2) const{

	std::pair<int, int> a = coord(index1);
	std::pair<int, int> b = coord(index2);

	int x = abs(a.first - b.first);
	int y = abs(a.second - b.second);
	
	float distance = 0;

	while(x * y > 0){ // diagonals
		distance += 1.4;
		--x;
		--y;
	}
	float remaining = std::max(x,y);

	distance += remaining;

	return distance;

}

//Stupid relative operations


int Grid::up(int index){
	int new_index = index - _col_size;
	return new_index;
}

int Grid::upRight(int index){
	int new_index = index - _col_size + 1;
	return new_index;
}

int Grid::right(int index){
	int new_index = index + 1;
	return new_index;
}

int Grid::downRight(int index){
	int new_index = index + _col_size + 1;
	return new_index;
}

int Grid::down(int index){
	int new_index = index + _col_size;
	return new_index;
}

int Grid::downLeft(int index){
	int new_index = index + _col_size - 1;
	return new_index;
}

int Grid::left(int index){
	int new_index = index - 1;
	return new_index;
}

int Grid::upLeft(int index){
	int new_index = index - _col_size - 1;
	return new_index;
}

/*
int main(){
	Grid g(5, 5, 0);
	g.reset_grid_border(0, 1);
	g.print();
	return 0;
}
*/
