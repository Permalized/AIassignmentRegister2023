

/*
 * By removing this comment the program will show the intermediate steps
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define breadth 1		// Constants denoting the four algorithms
#define depth	2
#define best	3
#define astar	4

#define increase 1		// Constants denoting the four algorithms
#define decrease 2
#define Double   3
#define half   	 4
#define square	 5	// Constants denoting the four algorithms
#define Root     6


struct tree_node
{
	int node_value;
	int node_depth; // the depth of this node
	int h;				// the absolute value between the target value and the node's value
	int g;				// the total cost of this node wrt the root of the search tree
	int f;				// f=0 or f=h or f=h+g, depending on the search algorithm used.
	struct tree_node *parent;	// pointer to the parrent node (NULL for the root).
	int operation;			// The operation of the last move
};

// A node of the frontier. Frontier is kept as a double-linked list,
// for efficiency reasons for the breadth-first search algorithm.
struct frontier_node
{
	struct tree_node *n;			// pointer to a search-tree node
	struct frontier_node *previous;		// pointer to the previous frontier node
	struct frontier_node *next;		// pointer to the next frontier node
};

struct frontier_node *frontier_head=NULL;	// The one end of the frontier
struct frontier_node *frontier_tail=NULL;	// The other end of the frontier

clock_t t1;				// Start time of the search algorithm
clock_t t2;				// End time of the search algorithm
#define TIMEOUT		60	// Program terminates after TIMOUT secs

int solution_length;	// The lenght of the solution table.
struct tree_node *solution;		// Pointer to a dynamic table with the moves of the solution.
int target_value; //The target value based on the arguments of the main call
int initial_value; //The initial value based on the arguments of the main call


// Reading run-time parameters.
int get_method(char* s)
{
	if (strcmp(s,"breadth")==0)
		return  breadth;
	else if (strcmp(s,"depth")==0)
		return depth;
	else if (strcmp(s,"best")==0)
		return best;
	else if (strcmp(s,"astar")==0)
		return astar;
	else
		return -1;
}

// This function checks whether a node in the search tree
// holds exactly the same puzzle with at least one of its
// predecessors. This function is used when creating the childs
// of an existing search tree node, in order to check for each one of the childs
// whether this appears in the path from the root to its parent.
// This is a moderate way to detect loops in the search.
// Inputs:
//		struct tree_node *new_node	: A search tree node (usually a new one)
// Output:
//		1 --> No coincidence with any predecessor
//		0 --> Loop detection
int check_with_parents(struct tree_node *new_node)
{
	struct tree_node *parent=new_node->parent;
	while (parent!=NULL)
	{
		if (new_node->node_value == parent->node_value)
			return 0;
		parent=parent->parent;
	}
	return 1;
}

// Giving a puzzle, this function computes the sum of the manhattan
// distances between the current positions and the intended positions
// for all the tiles of the puzzle.
// Inputs:
//		int p[N][N];	A puzzle
// Output:
//		As described above.
int heuristic(long node_value)
{
	int i,j;
	int score=0;
	score=abs(target_value - node_value);
	return score;
}


// This function adds a pointer to a new leaf search-tree node at the front of the frontier.
// This function is called by the depth-first search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_front(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=(struct frontier_node*)
                                malloc(sizeof(struct frontier_node));
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n = node;
	new_frontier_node->previous = NULL;
	new_frontier_node->next = frontier_head;

	if (frontier_head==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		frontier_head->previous=new_frontier_node;
		frontier_head=new_frontier_node;
	}

	return 0;
}

// This function adds a pointer to a new leaf search-tree node at the back of the frontier.
// This function is called by the breadth-first search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_back(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=(struct frontier_node*) malloc(sizeof(struct frontier_node));
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->next=NULL;
	new_frontier_node->previous=frontier_tail;

	if (frontier_tail==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		frontier_tail->next=new_frontier_node;
		frontier_tail=new_frontier_node;
	}

	return 0;
}

// This function adds a pointer to a new leaf search-tree node within the frontier.
// The frontier is always kept in increasing order wrt the f values of the corresponding
// search-tree nodes. The new frontier node is inserted in order.
// This function is called by the heuristic search algorithm.
// Inputs:
//		struct tree_node *node	: A (leaf) search-tree node.
// Output:
//		0 --> The new frontier node has been added successfully.
//		-1 --> Memory problem when inserting the new frontier node .
int add_frontier_in_order(struct tree_node *node)
{
	// Creating the new frontier node
	struct frontier_node *new_frontier_node=(struct frontier_node*)
                malloc(sizeof(struct frontier_node));
	if (new_frontier_node==NULL)
		return -1;

	new_frontier_node->n=node;
	new_frontier_node->previous=NULL;
	new_frontier_node->next=NULL;

	if (frontier_head==NULL)
	{
		frontier_head=new_frontier_node;
		frontier_tail=new_frontier_node;
	}
	else
	{
		struct frontier_node *pt;
		pt=frontier_head;

		// Search in the frontier for the first node that corresponds to either a larger f value
		// or to an equal f value but larger h value
		// Note that for the best first search algorithm, f and h values coincide.
		while (pt!=NULL && (pt->n->f<node->f || (pt->n->f==node->f && pt->n->h<node->h)))
			pt=pt->next;

		if (pt!=NULL)
		{
			// new_frontier_node is inserted before pt .
			if (pt->previous!=NULL)
			{
				pt->previous->next=new_frontier_node;
				new_frontier_node->next=pt;
				new_frontier_node->previous=pt->previous;
				pt->previous=new_frontier_node;
			}
			else
			{
				// In this case, new_frontier_node becomes the first node of the frontier.
				new_frontier_node->next=pt;
				pt->previous=new_frontier_node;
				frontier_head=new_frontier_node;
			}
		}
		else
		{
			// if pt==NULL, new_frontier_node is inserted at the back of the frontier
			frontier_tail->next=new_frontier_node;
			new_frontier_node->previous=frontier_tail;
			frontier_tail=new_frontier_node;
		}
	}

	return 0;
}

//This functions calculated the f function for astar method
int f(int g,int h,int method){
	if(method==astar) return g+h/2;
	 return 0;
	
	}
// This function expands a leaf-node of the search tree.
// A leaf-node may have up to 4 childs. A table with 4 pointers
// to these childs is created, with NULLs for those childrens that do not exist.
// In case no child exists (due to loop-detections), the table is not created
// and a 'higher-level' NULL indicates this situation.
// Inputs:
//		struct tree_node *current_node	: A leaf-node of the search tree.
// Output:
//		The same leaf-node expanded with pointers to its children (if any).
int find_children(struct tree_node *current_node, int method)
{
	int x;


	// Operation: Root
	if (current_node->node_value>1 && (floor(sqrt(current_node->node_value))==sqrt(current_node->node_value)))
	{
		// Initializing the new child
		struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
		if (child==NULL) return -1;

		child->parent = current_node;                 //The parent of the created node
		child->operation = Root;                     //The operation
		child->node_value= sqrt(current_node->node_value) ;
		x=child->parent->node_value; 
		child->node_depth = current_node->node_depth + 1;		// The depth of the new child
		child->g = current_node->g + ((x - sqrt(x))/4)+1; //The total cost of operations until the node


		// Check for loops
		if (!check_with_parents(child))
			// In case of loop detection, the child is deleted
			free(child);
		else
		{
			// Computing the heuristic value
			child->h=heuristic(child->node_value);
			if (method==best)
				child->f = child->h;
			else if (method==astar)
				child->f = f(child->g,child->h,method);
			else
				child->f = 0;

            int err=0;
            if (method==depth)
				err=add_frontier_front(child);
			else if (method==breadth)
				err=add_frontier_back(child);
			else if (method==best || method==astar)
				err=add_frontier_in_order(child);
			if (err<0)
                return -1;
		}

	}
	
	
    // Operation: square
	if ((current_node->node_value^2)>10^9)
	{
		// Initializing the new child
		struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
		if (child==NULL) return -1;

		child->parent = current_node;                 //The parent of the created node
		child->operation = square;                     //The operation
		child->node_value= pow(current_node->node_value,2) ;
		x=child->parent->node_value; 
		child->node_depth = current_node->node_depth + 1;		// The depth of the new child
		child->g = current_node->g + ((pow(x,2) - x )/4) +1; //The total cost of operations until the node


		// Check for loops
		if (!check_with_parents(child))
			// In case of loop detection, the child is deleted
			free(child);
		else
		{
			// Computing the heuristic value
			child->h=heuristic(child->node_value);
			if (method==best)
				child->f = child->h;
			else if (method==astar)
				child->f = f(child->g,child->h,method);
			else
				child->f = 0;

            int err=0;
            if (method==depth)
				err=add_frontier_front(child);
			else if (method==breadth)
				err=add_frontier_back(child);
			else if (method==best || method==astar)
				err=add_frontier_in_order(child);
			if (err<0)
                return -1;
		}

	}
	
	
    // Operation: half
	if (current_node->node_value>0)
	{
		// Initializing the new child
		struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
		if (child==NULL) return -1;

		child->parent = current_node;                 //The parent of the created node
		child->operation = half;                     //The operation
		child->node_value= floor(current_node->node_value/2); 
		x=child->parent->node_value; 
		child->node_depth = current_node->node_depth + 1;		// The depth of the new child
		child->g = current_node->g + ceil(x/4)+1; //The total cost of operations until the node


		// Check for loops
		if (!check_with_parents(child))
			// In case of loop detection, the child is deleted
			free(child);
		else
		{
			// Computing the heuristic value
			child->h=heuristic(child->node_value);
			if (method==best)
				child->f = child->h;
			else if (method==astar)
				child->f = f(child->g,child->h,method);
			else
				child->f = 0;

            int err=0;
            if (method==depth)
				err=add_frontier_front(child);
			else if (method==breadth)
				err=add_frontier_back(child);
			else if (method==best || method==astar)
				err=add_frontier_in_order(child);
			if (err<0)
                return -1;
		}

	}
	
    // Operation: double
	if (current_node->node_value>0 && 2*current_node->node_value<=10^9 )
	{
		// Initializing the new child
		struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
		if (child==NULL) return -1;

		child->parent = current_node;                 //The parent of the created node
		child->operation = Double;                     //The operation
		child->node_value= current_node->node_value*2 ;
		x=child->parent->node_value; 
		child->node_depth = current_node->node_depth + 1;		// The depth of the new child
		child->g = current_node->g + ceil(x/2)+1; //The total cost of operations until the node


		// Check for loops
		if (!check_with_parents(child))
			// In case of loop detection, the child is deleted
			free(child);
		else
		{
			// Computing the heuristic value
			child->h=heuristic(child->node_value);
			if (method==best)
				child->f = child->h;
			else if (method==astar)
				child->f = f(child->g,child->h,method);
			else
				child->f = 0;

            int err=0;
            if (method==depth)
				err=add_frontier_front(child);
			else if (method==breadth)
				err=add_frontier_back(child);
			else if (method==best || method==astar)
				err=add_frontier_in_order(child);
			if (err<0)
                return -1;
		}

	}
	
	
	 // Operation: decrease
	if (current_node->node_value>0 )
	{
		// Initializing the new child
		struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
		if (child==NULL) {printf("child null"); return -1 ;}

		child->parent = current_node;                 //The parent of the created node
		child->operation = decrease;                     //The operation
		child->node_value= current_node->node_value - 1 ;
		child->node_depth = current_node->node_depth + 1;		// The depth of the new child
		child->g = current_node->g + 2; //The total cost of operations until the node


		// Check for loops
		if (!check_with_parents(child))
			// In case of loop detection, the child is deleted
			free(child);
		else
		{
			// Computing the heuristic value
			child->h=heuristic(child->node_value);
			if (method==best)
				child->f = child->h;
			else if (method==astar)
				child->f = f(child->g,child->h,method);
			else
				child->f = 0;

            int err=0;
            if (method==depth)
				err=add_frontier_front(child);
			else if (method==breadth)
				err=add_frontier_back(child);
			else if (method==best || method==astar)
				err=add_frontier_in_order(child);
			if (err<0)
                return -1;
		}

	}
	
	
	 // Operation: increase
	if (current_node->node_value<10^9 )
	{
		// Initializing the new child
		struct tree_node *child=(struct tree_node*) malloc(sizeof(struct tree_node));
		if (child==NULL) return -1;

		child->parent = current_node;                 //The parent of the created node
		child->operation = increase;                     //The operation
		child->node_value= current_node->node_value + 1 ;
		child->node_depth = current_node->node_depth + 1;		// The depth of the new child
		child->g = current_node->g + 2; //The total cost of operations until the node


		// Check for loops
		if (!check_with_parents(child))
			// In case of loop detection, the child is deleted
			free(child);
		else
		{
			// Computing the heuristic value
			child->h=heuristic(child->node_value);
			if (method==best)
				child->f = child->h;
			else if (method==astar)
				child->f = f(child->g,child->h,method);
			else
				child->f = 0;

            int err=0;
            if (method==depth)
				err=add_frontier_front(child);
			else if (method==breadth)
				err=add_frontier_back(child);
			else if (method==best || method==astar)
				err=add_frontier_in_order(child);
			if (err<0)
                return -1;
		}

	}
	return 1;
}

// Auxiliary function that displays a message in case of wrong input parameters.
void syntax_message()
{
	printf("Syntax of the main call: \n");
	printf("Register2023 <method> <initial number> <target value> <output-file>\n\n");
	printf("where: ");
	printf("<method> = breadth|depth|best|astar\n");
	printf("<initial number> is the positive integer number of the root of the tree.\n");
	printf("<target value> is the positive integer target value of the search algorithm.\n");
	printf("<output-file> is the output file where the solution and the steps will be extracted.\n");
	printf("example : register.exe breadth 5 18 solution.txt.\n");
}

// This function checks whether a puzzle is a solution puzzle.
// Inputs:
//		int p[N][N]		: A puzzle
// Outputs:
//		1 --> The puzzle is a solution puzzle
//		0 --> The puzzle is NOT a solution puzzle
int is_solution(int node_value)
{
	if(node_value==target_value)return 1;
	return 0;
}

// Giving a (solution) leaf-node of the search tree, this function computes
// the moves of the blank that have to be done, starting from the root puzzle,
// in order to go to the leaf node's puzzle.
// Inputs:
//		struct tree_node *solution_node	: A leaf-node
// Output:
//		The sequence of blank's moves that have to be done, starting from the root puzzle,
//		in order to receive the leaf-node's puzzle, is stored into the global variable solution.
void extract_solution(struct tree_node *solution_node)
{
	int i;

	struct tree_node *temp_node=solution_node;
	solution_length = solution_node->node_depth;

	solution= (struct tree_node*) malloc(solution_length*sizeof(struct tree_node));
	temp_node=solution_node;
	i=solution_length;
	while (temp_node->parent!=NULL)
	{
		i--;
		solution[i] = *temp_node;
		temp_node=temp_node->parent;
	}
}

// This function writes the solution into a file
// Inputs:
//		char* filename	: The name of the file where the solution will be written.
// Outputs:
//		Nothing (apart from the new file)
void write_solution_to_file(char* filename, int solution_length,struct tree_node *solution)
{
	int i;
	FILE *fout;
	fout=fopen(filename,"w");
	if (fout==NULL)
	{
		printf("Cannot open output file to write solution.\n");
		printf("Now exiting...");
		return;
	}
	fprintf(fout,"%d, %d\n",solution_length,solution[solution_length-1].g);
	for (i=0;i<solution_length;i++)
		switch(solution[i].operation)
		{
		case increase:
			fprintf(fout,"increase %d %d \n" ,solution[i].parent->node_value,solution[i].g - solution[i].parent->g);
			break;
		case decrease:
			fprintf(fout,"decrease %d %d\n" ,solution[i].parent->node_value,solution[i].g - solution[i].parent->g) ;
			break;
		case Double:
			fprintf(fout,"double %d %d\n" ,solution[i].parent->node_value,solution[i].g - solution[i].parent->g) ;
			break;
		case half:
			fprintf(fout,"half %d %d \n" ,solution[i].parent->node_value,solution[i].g - solution[i].parent->g) ;
			break;
		case square:
			fprintf(fout,"square %d %d \n" ,solution[i].parent->node_value,solution[i].g - solution[i].parent->g) ;
			break;
		case Root:
			fprintf(fout,"root %d %d \n" ,solution[i].parent->node_value,solution[i].g - solution[i].parent->g );
			break;
			
			
	}
	fclose(fout);
}

// This function initializes the search, i.e. it creates the root node of the search tree
// and the first node of the frontier.
void initialize_search(int initial_value, int method)
{
	struct tree_node *root=(struct tree_node*) malloc(sizeof(struct tree_node));	// the root of the search tree.

	// Initialize search tree
	root->parent=NULL;
	root->operation=-1;
	root->node_value=initial_value;
	printf("Root node_value: %d\n",root->node_value);
	root->g=0;
	root->node_depth=0;
	root->h=heuristic(root->node_value);
	if (method==best)
		root->f=root->h;
	else if (method==astar)
		root->f=root->g+root->h;
	else
		root->f=0;

	// Initialize frontier
	add_frontier_front(root);
}

// This function implements at the higest level the search algorithms.
// The various search algorithms differ only in the way the insert
// new nodes into the frontier, so most of the code is commmon for all algorithms.
// Inputs:
//		Nothing, except for the global variables root, frontier_head and frontier_tail.
// Output:
//		NULL --> The problem cannot be solved
//		struct tree_node*	: A pointer to a search-tree leaf node that corresponds to a solution.
struct tree_node *search(int method)
{
	clock_t t;
	int i, err;
	struct frontier_node *temp_frontier_node;
	struct tree_node *current_node;

	while (frontier_head!=NULL)
	{
		t=clock();
		if (t-t1 > CLOCKS_PER_SEC*TIMEOUT)
		{
			printf("Timeout\n");
			return NULL;
		}

		// Extract the first node from the frontier
		current_node = frontier_head->n;

		if (is_solution(current_node->node_value))
			return current_node;

		// Delete the first node of the frontier
		temp_frontier_node=frontier_head;
		frontier_head = frontier_head->next;
		free(temp_frontier_node);
		if (frontier_head==NULL)
			frontier_tail=NULL;
		else
			frontier_head->previous=NULL;

		// Find the children of the extracted node
		int err=find_children(current_node, method);

		if (err<0)
	        {
            		printf("Memory exhausted while creating new frontier node. Search is terminated...\n");
			return NULL;
        	}
	}

	return NULL;
}

int main(int argc, char** argv)
{
	int err;
	struct tree_node *solution_node;
	int method;				// The search algorithm that will be used to solve the puzzle.

	if (argc!=5)
	{
		printf("Wrong number of arguments. Use correct syntax:\n");
		syntax_message();
		return -1;
	}

	method=get_method(argv[1]);
	if (method<0)
	{
		printf("Wrong method. Use correct syntax:\n");
		syntax_message();
		return -1;
	}
	char* p;
	char* p1;
    long value1 = strtol(argv[2], &p, 10);
    if (*p != '\0') {
	    syntax_message();  // an invalid character was found before the end of the string
	     return -1;
	}  
	initial_value=value1;
	printf("initial value: %d, ",value1);
		 
    long value2 = strtol(argv[3], &p1, 10);
    if (*p1 != '\0') {
	    syntax_message();  // an invalid character was found before the end of the string 
		return -1;
	 }
		  target_value=value2;
		  printf("target value: %d, ",value2);
		  
	//Setting values for initial_time and target_value base on the call
	if(initial_value<0||target_value==0){
		printf("\n Error: Only positive integers as input... Try again \n\n");
		syntax_message();
		return -1;
	}

	printf("Solving %s to %s using %s...\n",argv[2],argv[3],argv[1]);
	t1=clock();

	initialize_search(initial_value, method);

	solution_node = search(method);			// The main call

	t2=clock();

	if (solution_node!=NULL)
		extract_solution(solution_node);
	else
		printf("No solution found.\n");

	if (solution_node!=NULL)
	{
		printf("Solution found! (%d steps)\n",solution_length);
		printf("Time spent: %f secs\n",((float) t2-t1)/CLOCKS_PER_SEC);
		write_solution_to_file(argv[4], solution_length, solution);
	}

	return 0;
}
