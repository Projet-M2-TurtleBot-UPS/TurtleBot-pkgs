/*
  visib_init.cpp
  Thibaut AGHNATIOS

  displayGraphVisib : Function which create a graph and recover positions and orientation of markers
  Ecriture_carte_visib : Generating a visibility map (PGM P2) and need to modify window scale and number of markers
  pgm_imread : Reading pgm file and return image struct
 */

#include "visib_init.hpp"
#include "graph.cpp"

void displayGraphVisib(Graph g, float x1[], float y1[], float t[])
{
	int a=0;
    // Display vertices
    std::cout << "Display vertices" <<  std::endl;
    typedef boost::graph_traits<Graph>::vertex_iterator vertex_iter;
    std::pair<vertex_iter, vertex_iter> vertexPair;
    for (vertexPair = vertices(g); vertexPair.first != vertexPair.second; ++vertexPair.first)
    {
        std::cout << g[*vertexPair.first].id << " " << g[*vertexPair.first].label << " " << g[*vertexPair.first].x << " " << g[*vertexPair.first].y << " " << g[*vertexPair.first].orientation << std::endl;

	x1[a]=g[*vertexPair.first].x;
	y1[a]=g[*vertexPair.first].y;
	t[a]=g[*vertexPair.first].orientation;
	a+=1;
    }
    
    // Display edges weights
    std::cout << "Display edges weights" <<  std::endl;
    boost::property_map<Graph, boost::edge_weight_t>::type EdgeWeightMap = get(boost::edge_weight_t(), g);
    typedef boost::graph_traits<Graph>::edge_iterator edge_iter;
    std::pair<edge_iter, edge_iter> edgePair;
    for(edgePair = edges(g); edgePair.first != edgePair.second; ++edgePair.first)
    {
        std::cout << boost::source(*edgePair.first, g) << " --" << EdgeWeightMap[*edgePair.first] << "--> "<< boost::target(*edgePair.first, g) << std::endl;
    }
}


using namespace std;


void Ecriture_carte_visib()
{	
	//Window scale (!!! to modify if you don't use 512*640)
   	int largeur=512;
	int hauteur=640;
	//Markers number (!!! to modify)
	int nbr_amers=2;
	//Storage of markers positions in pixels	
	int x[nbr_amers];
    	int y[nbr_amers];   
	float t[nbr_amers]; //Markers orientation (theta)
	int i,j,k,l,o,p=0,r=0,s=0,u=0,w=0,a;
	float dist=0.0 ; //Variable for the calculation of the distance
	int pix[largeur][hauteur];
	int v[nbr_amers][nbr_amers];
	float yn[nbr_amers],xn[nbr_amers];
	float x1[nbr_amers],y1[nbr_amers]; // Position of markers in meters
    	float angle[nbr_amers-1];
	float alphamax[nbr_amers-1]; //angle of view
	int distancemax;
	int distancemin;
	float distancem=2; //Range of marker's visibility (2 meter)
	float xp[nbr_amers],yp[nbr_amers];


	//Reading the graph to retrieve positions of markers (positions x1 and y1; orientation t)
	Graph Graph_test = xmlToGraph("graph.xml");
    	displayGraphVisib(Graph_test,x1,y1,t);
		
	for(a=0;a<nbr_amers;a++)
	{	
		// Convert meter to pixel (resolution 0.05 and window scale 512*640) !!!to modify if window scale different
		x[a]=(x1[a]+12.2)/0.05;
		y[a]=-(y1[a]+13.8-32)/0.05;
		alphamax[a]=pi/4;
	}

	distancemax=distancem/0.05;
	ROS_INFO("Dmax = %d",distancemax);
	distancemin=0.20/0.05; // Minimal range of marker's visibility for turtlebot (0.20 meter)	
	ROS_INFO("Dmin = %d",distancemin);

    	ofstream fichier("src/TurtleBot-pkgs/turtlebot_proj_nav/map/visib.pgm", ios::out | ios::trunc);  
		// Opening file in writing mode with deletion of open file

   	if(fichier && (r<1))
        {	
		//Head of pgm
		ROS_INFO("Creation de la carte");
		fichier << "P2" << endl;
		fichier << "#Thibaut" << endl;
		fichier << "#Carte de visibilite" << endl;
		fichier << largeur << " " << hauteur << endl;
		fichier << "15" << endl;
	}
			
    
	for (i=0;i<hauteur;i++) // Rows
	{
		for (j=0;j<largeur;j++) // Line
		{
			pix[j][i]=15;
			
			for(k=0;k<nbr_amers;k++) // Loop for markers
			{
				dist = sqrt(pow(i-y[k],2)+pow(j-x[k],2)); // Distance calculation of the current pixel compared to markers

				// Calculation of normal
				if((t[k]<pi/2.0 && t[k]>=0) || (t[k]<2*pi && t[k]>=3*pi/2)) 
					xn[k] = 1;
				else 	xn[k] = -1;				
					
				if(t[k]>=0 && t[k]<pi)
				{	yn[k] = -(tan(t[k])*abs(xn[k]));
				}
				else yn[k] = (tan(t[k])*abs(xn[k]));
				
				//printf("t=%f \n",t[k]);
				
				if(t[k]>=0.0 && t[k]<pi/2.0)
				{
					xp[k]=(j-x[k]);
					yp[k]=-(-i+y[k]);
				}
				if(t[k]>=pi/2.0 && t[k]<pi)
				{
					xp[k]=(j-x[k]);
					yp[k]=(-i+y[k]);
				}
				if(t[k]>=pi && t[k]<3.0*pi/2.0)
				{
					xp[k]=(j-x[k]);
					yp[k]=-(-i+y[k]);
				}
				if(t[k]>=3.0*pi/2.0 && t[k]<2.0*pi)
				{
					xp[k]=(j-x[k]);
					yp[k]=(-i+y[k]);
				}
				
				//calculating the angle (vector product)
				angle[k]=acos((xp[k]*xn[k]+(yp[k]*yn[k]))/(sqrt(pow(xp[k],2)+pow(yp[k],2))*(sqrt(pow(xn[k],2)+pow(yn[k],2)))));					
				
				// If the distance is between the minimal distance and the maximal distance and too in the angle of view of the marker, the white pixel changes to gray	
				if((dist<=distancemax && dist>=distancemin) && (angle[k]<=alphamax[k] && angle[k]>=-alphamax[k]) && (pix[j][i]!=0) ) 
				{	
					pix[j][i]-=3;
				}
				else 
					if(dist == 0.0)//If the distance is null it means that it is the marker's postion (we put the pixel black=0)
					{	
						pix[j][i]=0;
					}
			}

			if(r<1)	//Flag to know if the writing has already been done 1 time
			{	
				fichier << pix[j][i] << " " ;
				p+=1;
			}
			if(p>=70 && (r<1))// Back to line each 70 terms to respect the P2 format
			{
				fichier << endl;
				p=0;		
			}
			else if((p<70 && (r==1)))
			{
				ROS_INFO("Integer false, image non generee correctement !");
			}
		}
	}

		if(r<1)      		        
        	{	
			fichier.close();
			ROS_INFO("Creation de la carte terminee \n________________________________________________________");
			r=1;
		}
}

/******** PGM READING FUNCTION ******************/
table pgm_imread(char *argv)			//reads pgm image
	{
	char line[70];
	int imagetype = 0, cols, rows, maxintensity, p2read;
	int **data;
	unsigned char p5read;
	stringstream buffer;
	ifstream infile(argv);			//opens image
	if (infile == NULL)
    		{
	        cerr<<argv[1]<<" either cannot be read or does not exist\n";
        	exit(0);
	        }
	infile.getline(line, 70);
	if(line[0] == 'P')
		switch(line[1])
			{
			case '5' :	imagetype = 1; cout<<"P5 PGM image detected\n"; 		break; //raw image
			case '2' :	imagetype = 2; ROS_INFO("P2 PGM image detected"); 		break; //ASCII image
			default	 :	imagetype = 0; cerr<<"unsupported PGM image format\n"; exit(0); break;
			}
	else
		{
		cerr<<"Invalid image format\n";
		exit(0);
		}
	while (infile.peek()=='#')
        	infile.getline(line,70); 	//read all the comments and oomit them.
	infile >> cols >> rows >> maxintensity;	//read the no of coloumns rows and pixel intensity
	data = new int* [cols];		//memory allocation
	if (!data)
		{
      	     cout << "allocation failure in matrix";
	     exit(1);
    		}
	for(int i = 0; i<cols;i++)
		{
		data[i] = new int[rows];	//making allocation for 2d matrix
	        if (!data[i])
			{
	       		cout << "allocation failure in matrix";
	       		exit(1);
      			}
		}
	if(imagetype == 1)			//raw image mode
		for (int i=0;i<cols;i++)
        	{
        	 for (int j=0;j<rows;j++)
			{
         	        infile>>p5read;
			data[i][j] = (int)p5read;
			//printf("data[%d][%d]= %d \n",i,j,data[i][j]);
			}
	        }
	else if(imagetype == 2)			//ASCII image mode
		{
		buffer << infile.rdbuf();
		for (int i=0;i<cols;i++)
       			{
       			 for (int j=0;j<rows;j++)
				{
       			        buffer>>p2read;
				data[i][j] = p2read;
				//printf("data[%d][%d]= %d \n",i,j,data[i][j]);
				}
       			 }
		}
	table image = {data, cols, rows};
	ROS_INFO("Image read complete");
	infile.close();
    	return image; 		 		//returns the image as a 2-D array in a structure
}
