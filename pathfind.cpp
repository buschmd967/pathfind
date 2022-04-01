#include "grid.h"
#include "cell.h"
#include <algorithm>
#include <iostream>

namespace pathfind{

int astar_find_target_index(const Grid & g){
	int min_hcost = -1;
	int min_fcost = -1;
	int target = -1;
	int start = -1;
	for(int i = 0; i < g.col_size() * g.row_size(); i++){
		if(g[i] == 4){
			if(min_fcost > g[i].fcost() || min_hcost == -1){
				min_hcost = g[i].hcost();
				min_fcost = g[i].fcost();
				target = i;
			}
			else if(min_fcost == g[i].fcost() && min_hcost > g[i].hcost()){
				min_hcost = g[i].hcost();
				min_fcost = g[i].fcost();
				target = i;
			}
		}
		else if(g[i] == 2){
			start = i;
		}
	}
	return (target != -1) ? target : start;

}


void mark_path(Grid & g, Cell & c){
	std::cout << "marking" << std::endl;
	if(c != 2){
		c = 6;
		std::cout << c.last_cell_index << std::endl;
		mark_path(g, g[c.last_cell_index]);
	}
	else{

	std::cout << "DONE";
	}
}



bool evaluateNeighbors(Grid & g, int start_index, int end_index, int target_index){
	g[target_index] = 5;
	if(target_index == start_index){
		g[target_index] = 2;
		g[start_index].gcost(0);
		g[start_index].hcost( (int) (g.distance(start_index, end_index) * 10) );
	}
	int starting_gcost = g[target_index].gcost();
	
	for(int i = 0; i < 8; i++){
		int gcost_add = 10;
		int index = -1;
		if(i % 2 == 1){
			gcost_add = 14;
		}

		switch(i){
			case 0:
				index = g.up(target_index);	
				break;
			case 1:
				index = g.upRight(target_index);
				break;
			case 2:
				index = g.right(target_index);	
				break;
			case 3:
				index = g.downRight(target_index);	
				break;
			case 4:
				index = g.down(target_index);	
				break;
			case 5:
				index = g.downLeft(target_index);	
				break;
			case 6:
				index = g.left(target_index);	
				break;
			case 7:
				index = g.upLeft(target_index);	
				break;
		}
		
		if(g[index] == 1  ){ // New White Square
			g[index] = 4;
			g[index].hcost( (int) (g.distance(index, end_index) * 10) );
			g[index].gcost( starting_gcost + gcost_add);
			g[index].last_cell_index = target_index;
		}
		else if(g[index] == 4){ // Already calculated
			if(starting_gcost + gcost_add < g[index].gcost()){
				
			g[index].last_cell_index = target_index;
			g[index].gcost( starting_gcost + gcost_add );
			}
		}
		else if(g[index] == 3){ // Found Red

			
			mark_path(g, g[target_index]);

			return true;
		}


	}
	return false;

}

bool Astar(Grid & g, int start_index, int end_index, int target_index){
	std::cout << target_index << std::endl;
	return evaluateNeighbors(g, start_index, end_index, target_index);
	//return g[target_index].hcost() <= 14;
	
}



}
