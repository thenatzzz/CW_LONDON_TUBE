######### NEED TO FIX NUMBER VAR PROBLEM ##################

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cassert>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <string>
#include <sstream>
#include <typeinfo>

using namespace std;

#include "tube.h"

/* You are pre-supplied with the functions below. Add your own
   function definitions to the end of this file. */

/* internal helper function which allocates a dynamic 2D array */
char** allocate_2D_array(int rows, int columns) {
  char** m = new char* [rows];
  assert(m);
  for(int r=0; r<rows; r++) {
    m[r] = new char[columns];
    assert(m[r]);
  }
  return m;
}

/* internal helper function which deallocates a dynamic 2D array */
void deallocate_2D_array(char** m, int rows) {
  for(int r=0; r<rows; r++)
    delete [] m[r];
  delete [] m;
}

/* internal helper function which gets the dimensions of a map */
bool get_map_dimensions(const char* filename, int& height, int& width) {
  char line[512];
  
  ifstream input(filename);
  
  height = width = 0;
  
  input.getline(line, 512);
  while(input) {
    if( (int) strlen(line) > width)
      width = strlen(line);
    height++;
    input.getline(line, 512);
  }
  
  if(height > 0)
    return true;
  return false;
}

/* pre-supplied function to load a tube map from a file*/
char** load_map(const char* filename, int& height, int& width) {
  
  bool success = get_map_dimensions(filename, height, width);
  
  if(!success) return NULL;
  
  char** m = allocate_2D_array(height, width);
  
  ifstream input(filename);
  
  char line[512];
  char space[] = " ";
  
  for(int r = 0; r<height; r++) {
    input.getline(line, 512);
    strcpy(m[r], line);
    while( (int) strlen(m[r]) < width ) {
      strcat(m[r], space);
    }
  }
  
  return m;
}

/* pre-supplied function to print the tube map */
void print_map(char** m, int height, int width) {
  cout << setw(2) << " " << " ";
  for(int c=0; c<width; c++) {
    if(c && (c % 10) == 0) {
      cout << c/10;
    } else {
      cout << " ";
    }
	}
  
  cout << endl;
  cout << setw(2) << " " << " ";
  
  for(int c=0; c<width; c++) cout << (c % 10);
  
  cout << endl;
  
  for(int r=0; r<height; r++) {
    cout << setw(2) << r << " ";
    for(int c=0; c<width; c++) cout << m[r][c];
    cout << endl;
  }
  
}

/* pre-supplied helper function to report the errors encountered in Question 3 */
const char* error_description(int code) {
  switch(code) {
  case ERROR_START_STATION_INVALID:
    return "Start station invalid";
  case ERROR_ROUTE_ENDPOINT_IS_NOT_STATION:
    return "Route endpoint is not a station";
  case ERROR_LINE_HOPPING_BETWEEN_STATIONS:
    return "Line hopping between stations not possible";
  case ERROR_BACKTRACKING_BETWEEN_STATIONS:
    return "Backtracking along line between stations not possible";
  case ERROR_INVALID_DIRECTION:
    return "Invalid direction";
  case ERROR_OFF_TRACK:
    return "Route goes off track";
  case ERROR_OUT_OF_BOUNDS:
    return "Route goes off map";
  }
  return "Unknown error";
}

/* presupplied helper function for converting string to direction enum */
Direction string_to_direction(string token) {
  string strings[] = {"N", "S", "W", "E", "NE", "NW", "SE", "SW"};
  for(int n=0; n<8; n++) {
    if(strings[n]==token) return (Direction) n;
  }
  return INVALID_DIRECTION;
}

bool get_symbol_position(char** m, int height, int width, char symbol, int& row, int& column)
{
  bool result= false;
  int col_number;
  int row_number;
  
  for( row_number = 0; row_number < height; row_number++)
    {
      for( col_number = 0; col_number < width ; col_number++)
	{
	  if(m[row_number][col_number] == symbol)
	    {
	      result = true;
	      row = row_number;
	      column = col_number;
	    }
	}
    }
  return result;
}

string get_symbol_for_station_or_line(string station_name)
{
  ifstream in_stream_lines;
  ifstream in_stream_stations;
  
  string station_or_line;
  
  in_stream_lines.open("lines.txt");
  in_stream_stations.open("stations.txt");
  
  while( !in_stream_lines.fail())
    {
      while( ( getline(in_stream_lines, station_or_line)) ||  (getline(in_stream_stations, station_or_line))    )
	{
	  if( station_name == station_or_line.substr(2, station_or_line.length())) //compare station_name to station in file   
	    return  station_or_line.substr(0,1);
	}
    }
  return "_"; // return _ if there are no symbols in given file
  
  in_stream_lines.close();
  in_stream_stations.close();
}

void moving(int direction_store,char** m, int& station_location_row, int& station_location_column)
{
  switch(direction_store)
    {
    case 0: //North
      station_location_row--;
      break;
    case 1: //South
      station_location_row++;
      break;
    case 2: //West
      station_location_column--; 
      break;
    case 3: //East
      station_location_column++;
      break;
    case 4: // North East
      station_location_row--;
      station_location_column++; 
      break;
    case 5: //North West
      station_location_row--;
      station_location_column--; 
      break;
    case 6: //South East
      station_location_row++;
      station_location_column++;
      break;
    case 7: //South West
      station_location_row++;
      station_location_column--;
      break;
    }
}

//forward declaration of function to validate route
int validate_line(char** m, int height, int width, string start_station,string route, string& destination);

int validate_route(char** m,int height, int width, string station_name, char* route, char* destination )
{
  //transform char* into string
  string route_string(route);
  string destination_string(destination);
  
  int result = validate_line(m, height, width, station_name,route_string,destination_string);;
  
  // .c_str helps transform string back to char*
  strcpy(destination , destination_string.c_str());
  
  return result;
}


int validate_line(char** m, int height, int width, string start_station,string route,string& destination)
{
  int number_of_station_change = 0;
  
  // open stations.txt to match start_station to given symbol
  ifstream in_stream_stations("stations.txt");
  string station;
  char start_station_symbol;
  bool find_start_station = false;
  
  while( !in_stream_stations.fail())
    {
      while( getline(in_stream_stations, station) )
	{
	  if( start_station == station.substr(2, station.length()))
	    {
	      start_station_symbol =  station[0];
	      find_start_station = true;
	    }
	}
    }
  //check whether start_station is valid or not
  if( !find_start_station)
    return ERROR_START_STATION_INVALID;
  
  //find location of station in map.txt
  int station_location_row, station_location_column;
  int start_location_row, start_location_column;
  
  for(int row  =0 ; row  < height ; row++)
    {
      for(int column = 0 ;  column < width ; column++)
	{
	  if( m[row][column] == start_station_symbol)
	    {
	      station_location_row = row;
	      start_location_row = row;  //variable to collect row number of start station
	      
	      station_location_column = column;
	      start_location_column = column; // variable to collect column number of start station
	    }
	}
    }

  //############### DIRECTION ERROR CHECKING ############################################################
  
  
  //translate route input into number and store into 'list'
  istringstream string_stream(route); 
  string token;  
  int list[100];
  int count = 0; // count = number of move (given direction)
  int direction_number;
  
  while(getline(string_stream, token, ','))  // use string_stream function to cut ','  || token is direction symbol ie. "N"
    {
      //check whether route input is valid or not (check token)
      if( token != "N" && token != "S" && token != "E" && token != "W" && token != "NE" && token != "NW" && token != "SE" && token != "SW")
	return ERROR_INVALID_DIRECTION;
      
      direction_number = string_to_direction(token);  //translate token to number by using pre-supplied function
      list[count] = direction_number; //direction( in integer) in list
      
      count++;
    }

  //############## BACKTRACKING ERROR CHECKING #######################################################

  
  //compare between location x and location x+1
  //thus, number of comparisons will be fewer than number of direction by one
  for(int x = 0; x < count-1 ; x++)
    {
      //check whether there is retracement of direction
      if((list[x] ==0 && list[x+1] == 1) || (list[x] == 1 && list[x+1] == 0)) // North should not be with South
	return ERROR_BACKTRACKING_BETWEEN_STATIONS;
      if((list[x] == 5 && list[x+1] == 6)||(list[x] == 6 && list[x+1] == 5)) // North West should not be with South East
	return ERROR_BACKTRACKING_BETWEEN_STATIONS;
      if((list[x] == 3 && list[x+1] == 2)||(list[x] ==2 && list[x+1] == 3)) // West should not be with East
	return ERROR_BACKTRACKING_BETWEEN_STATIONS;
      if((list[x] == 4 && list[x+1] == 7)||(list[x] == 7 && list[x+1] == 4)) // North East should not be with South West
	return ERROR_BACKTRACKING_BETWEEN_STATIONS;
    }

  
  //############### BOUND AND TRACK ERROR CHECKING #####################################################

  
  //check whether route is going out of bound or off track or not
  //check by going through whole direction in given route
  for(int count2 = 0 ; count2 < count ; count2++)
    { 
      moving(list[count2],m,station_location_row, station_location_column);
      
      if((station_location_row < 0 || station_location_row >= height) || ( station_location_column >= width || station_location_column < 0))
	return ERROR_OUT_OF_BOUNDS;
      
      if(m[station_location_row][station_location_column] == ' ')
	return ERROR_OFF_TRACK;
    }

  
  //########### LINE HOPPING ERROR CHECKING ############################################################

  
  //check whether there is line hopping or not
  int count_hopping = 0; 
  char current_symbol, next_symbol; // check by comparing current_location against next_location
  
  // need to assign beginning location since last function already moved from the original location
  station_location_row = start_location_row;
  station_location_column = start_location_column;
  
  // current_location = location 0 in list[]--- [current][..][..][..][..]...
  moving(list[count_hopping],m,station_location_row, station_location_column);
  current_symbol = m[station_location_row][station_location_column];
  count_hopping++;
  
  //next_location = location 1 in list[] --- [..][next][..][..][..]...
  moving(list[count_hopping], m , station_location_row, station_location_column);
  next_symbol = m[station_location_row][station_location_column];
  count_hopping++;
  
  //check for line hopping between the first current_location and the first next_location
  //if there is a line hopping, current symbol and next symbol will not be equal
  //as well as next symbol will not be alphabet and number.
  if(current_symbol != next_symbol && ((next_symbol =='-') ||(next_symbol =='*')||(next_symbol =='#') ||(next_symbol =='&') ||(next_symbol =='$') ||(next_symbol =='+') ||(next_symbol =='|') ||(next_symbol =='<') ||(next_symbol =='>')))
    return ERROR_LINE_HOPPING_BETWEEN_STATIONS;
  
  //check whether there is a station on the first next_location, by checking whether next_station is alphabet and number
  if((next_symbol !='-') &&(next_symbol !='*') &&(next_symbol !='#') &&(next_symbol !='&') &&(next_symbol !='$') &&(next_symbol !='+') &&(next_symbol !='|') &&(next_symbol !='<') &&(next_symbol !='>'))
    number_of_station_change++;
  
  while( count-count_hopping>0)
    {     
      //check whether there is station or not
      // if there is station in route, move current_location 1 step and move next_location to next 2 step from station
      if( next_symbol != '*' && next_symbol != '-'&& next_symbol != '#'&& next_symbol != '&'&& next_symbol != '$'&& next_symbol != '+'&& next_symbol != '|'&& next_symbol != '<'&& next_symbol != '>' )
	{
	  // if seeing station, move current one spot off station ---  [..][..][A][curernt][..]...
	  moving(list[count_hopping], m ,station_location_row, station_location_column);
	  next_symbol= m[station_location_row][station_location_column];
	  current_symbol = next_symbol; 
	  count_hopping++;
	  
	  // and move next two spots off station --- [..][..][A][..][next][..]...
	  moving(list[count_hopping], m , station_location_row, station_location_column);
	  next_symbol = m[station_location_row][station_location_column];
	  count_hopping++;
	  
	  // check for line hopping, by checking whether next_symbol is not a station symbol (alphabet and number)
	  if((count_hopping<count)&&(current_symbol != next_symbol) && ((next_symbol =='-') ||(next_symbol =='*') ||(next_symbol =='#') ||(next_symbol =='&') ||(next_symbol =='$') ||(next_symbol =='+') ||(next_symbol =='|') ||(next_symbol =='<') ||(next_symbol =='>') ))
	    return ERROR_LINE_HOPPING_BETWEEN_STATIONS;
	  
	  continue;
	}
      
      //compare symbol between current_location and next_location if it is valid, move to next location in list 
      if( next_symbol == current_symbol)
	{
	  current_symbol = next_symbol;
	  moving(list[count_hopping], m, station_location_row, station_location_column);
	  next_symbol = m[station_location_row][station_location_column];
	  count_hopping++;
	  
	  //if next symbol is alphabet or number, we increment number of station change
	  if((next_symbol !='-') &&(next_symbol !='*') &&(next_symbol !='#') &&(next_symbol !='&') &&(next_symbol !='$') &&(next_symbol !='+') &&(next_symbol !='|') &&(next_symbol !='<') &&(next_symbol !='>'))
	    number_of_station_change++;
	}
      
      //check for line hopping by checking whether next symbol is not alphabet and number
      if((current_symbol != next_symbol) && ((next_symbol =='-') ||(next_symbol =='*') ||(next_symbol =='#') ||(next_symbol =='&') ||(next_symbol =='$') ||(next_symbol =='+') ||(next_symbol =='|') ||(next_symbol =='<') ||(next_symbol =='>') ))
	return ERROR_LINE_HOPPING_BETWEEN_STATIONS;
    }


  //############## END POINT STATION ERROR CHECKING ######################################################

  
  // assign alread-changed location to the original location( start location)
  station_location_row = start_location_row;
  station_location_column = start_location_column;
  
  // move until getting to destination
  for(int count_end = 0; count_end < count ; count_end++)
    moving(list[count_end],m, station_location_row, station_location_column);
  
  //check destination whether it is valid and existing or not
  ifstream in_stream_stations1("stations.txt");
  string end_station;
  bool find_end_location = false;
  string station1;
  
  while( !in_stream_stations1.fail())
    {
      while(getline(in_stream_stations1, station1))
	{
	  if( m[station_location_row][station_location_column] == station1[0])
	    {
	      end_station = station1.substr(2, station1.length());
	      find_end_location= true;
	    }
	}
    }
  
  in_stream_stations1.close();
  
  if(!find_end_location)
    return ERROR_ROUTE_ENDPOINT_IS_NOT_STATION;

  
  //#####################################################################################################
  
  destination = end_station;
  
  return number_of_station_change;
}










 

