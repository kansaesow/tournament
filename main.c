/*
$$ input format of tally sheets $$
must consist: 1.board 2.who plays who (2 columns) 3.order of game 
4.winner - embedded in the table header (if draw, order doesn't matter)
5.who plays first 6.points of 2 people
accumulate to 7 column - can be more than this, but these 7 are mandatory
The specific names in the order are:
game | board | first player | winner | loser | winner's score | loser's score

$$ input format of name list $$
must consist: 1.ID 2.Name & Surname (English)
The specific names are:
ID | name

---------------------------------------------------------------------------------
structure of variables
        game     
game:    8             (maximum 255 games - limited to 16 by no. of win)
       first board
board:   1     7       (maximum 127 boards - limited to 127 by no. of player)
        player
player:   8            (maximum 255 players, player 0 is bye win)
        win  point
point:   5     11      (win = 2, lose = 0, draw = 1; care must be taken when sort)
Maximum 255 players and 256 (add game 0 for later use) games = 65280
RAM 65.28k * (1+1+1+2) = 326.4kB <- must be malloc, maybe based on list of players
formula for size is 256 * no. of player => RAM 1280 * no. of player [Bytes]
---------------------------------------------------------------------------------
To-Do
  - maloc game, board, player, point

*/
#include <direct.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   // malloc
#include <math.h>
#include <sys/time.h>
#include <string.h> // strlen

#define game_h          0
#define board_h         1
#define first_h         2
#define winner_h        3
#define loser_h         4
#define winner_score_h  5
#define loser_score_h   6

int main()
{
  struct timeval time;
  char list_file_name[]   = "\\list.csv";   /*"\\list.csv"*/
  char tally_file_name[]  = "\\tally.csv";  /*"\\tally.csv"*/
  FILE *tally_csv;      // in format of tally sheets
  FILE *name_list_csv;  // in format of name list
  char* full_list_file_name;
  char* full_tally_file_name;
  char* test;
  char* working_dir;
  uint8_t** matching_table;
  uint8_t* game;
  uint8_t* board;
  uint8_t* player;
  uint16_t* point;
  uint8_t* present_player;
  uint16_t summary_row = 0;
  uint8_t max_game = 0;
  int16_t foo = 0;
  int16_t tally_row[7] = {-1, -1, -1, -1, -1, -1, -1};
  int16_t player_count = 0; // active player in the latest game
  int16_t part_count = 0; // participant count
  int16_t line_count = 0;
  int8_t header_count = 0;
  uint16_t winner_index = 0;
  uint16_t loser_index = 0;
  int8_t map_count = 0;
  uint8_t buffer_index = 0;
  uint8_t sorted = 0;
  uint8_t found = 0;
  int8_t table_map[2][7] = {{-1,-1,-1,-1,-1,-1,-1}, {-1,-1,-1,-1,-1,-1,-1}};
  uint8_t max_column = 0;
  char buffer[256];
  char s2n[8];
  char tally_header[7][16] = {"game" , "board", "first player", "winner", "loser", "winner's score", "loser's score"};

  working_dir = _getcwd( NULL, 0 );

  full_list_file_name = malloc(sizeof(list_file_name) + strlen(working_dir));
  strcpy(full_list_file_name, working_dir);
  strcat(full_list_file_name, list_file_name);
  full_tally_file_name = malloc(sizeof(tally_file_name) + strlen(working_dir));
  strcpy(full_tally_file_name, working_dir);
  strcat(full_tally_file_name, tally_file_name);

  name_list_csv = fopen(full_list_file_name, "r");
  if(name_list_csv == NULL){printf("Error Openning List File!\n");}
  part_count = 0;
  while(fgets(buffer, 256, name_list_csv) != NULL){part_count++;}
  part_count = (int16_t)(part_count - 1);
  fclose(name_list_csv);

  tally_csv = fopen(full_tally_file_name, "r");
  if(tally_csv == NULL){printf("Error Openning Tally File!\n");}
  line_count = 0;
  while(fgets(buffer, 256, tally_csv) != NULL)
  {
    line_count++;
  }
  line_count = (int16_t)(line_count - 1);
  fclose(tally_csv);

  
  tally_csv = fopen(full_tally_file_name, "r");
  fgets(buffer, 256, tally_csv);
  buffer_index = 0;
  header_count = 0;
  foo = 0;
  while(1)
  {
    for(int count = 0; count < 7; count++)
    {
      found = 1;
      int count_2;
      for(count_2 = 0; tally_header[count][count_2]; count_2++)
      {
        if(buffer[buffer_index+count_2] != tally_header[count][count_2]){found = 0; break;}
      }
      if(!((buffer[buffer_index+count_2] == ',') || (buffer[buffer_index+count_2] == 0))){found = 0;}
      if(found)
      {
        table_map[0][count] = (int8_t)foo++;
        table_map[1][count] = header_count;
        buffer_index += (uint8_t)count_2 + 1;
      }
      header_count++;
    }
    if(buffer[buffer_index-1] == 0){break;}
  }

  for(int count = 0; count < 7; count++)
  {
    max_column = max_column < table_map[1][count] ? table_map[1][count] : max_column;
  }
  while(!sorted)
  {
    sorted = 1;
    for(int count = 1; count < 7; count++)
    {
      if(table_map[1][count] < table_map[1][count-1])
      {
        foo = table_map[1][count];
        table_map[1][count] = table_map[1][count-1];
        table_map[1][count-1] = table_map[1][count];
        foo = table_map[0][count];
        table_map[0][count] = table_map[0][count-1];
        table_map[0][count-1] = table_map[0][count];
        sorted = 0;
      }
    }
  }

  game =    malloc((unsigned int)line_count * 2 * sizeof(uint8_t));
  board =   malloc((unsigned int)line_count * 2 * sizeof(uint8_t));
  player =  malloc((unsigned int)line_count * 2 * sizeof(uint8_t));
  point =   malloc((unsigned int)line_count * 2 * sizeof(uint16_t));

  winner_index = 0;
  loser_index = 0;
  // row count
  for(int count = 0; count < line_count; count++)
  {
    fgets(buffer, 256, tally_csv);
    map_count = 0;
    // column count
    for(int count_2 = 0; count_2 < max_column; count_2++)
    {
      int count_3 = 0;
        // each text count in column
      for(count_3 = 0; !((buffer[buffer_index + count_3] == ',') || (buffer[count_3] == '\n')); count_3++)
      {
        if(table_map[1][map_count] == count_2)
        {
          s2n[count_3] = buffer[buffer_index + count_3];
        }
      }
      if(table_map[1][map_count] == count_2)
      {
        sscanf(s2n, "%hd", tally_row + table_map[0][map_count]);
      }
      buffer_index = (uint8_t)(buffer_index + 1 + (uint8_t)count_3);
    }
      foo = tally_row[winner_score_h] - tally_row[loser_score_h];
      if(tally_row[board_h] >> 7){printf("Warning: The number of board exceeds 255.");}
      if(tally_row[winner_h] > part_count){printf("Warning: The winner's ID (%d) of game %d, board %d exceeds the number of participant (%d).", tally_row[winner_h], tally_row[game_h], tally_row[board_h], part_count);}
      if((tally_row[winner_score_h] > 1000) || (tally_row[loser_score_h] > 1000)){printf("Warning: The point of game %d, board %d is more than 1000 (%d vs %d). Maybe something fishy.", tally_row[game_h], tally_row[board_h], tally_row[winner_score_h], tally_row[loser_score_h]);}
      if(foo < 0){printf("Warning: In game %d, board %d, the winner scored lower than loser (%d vs %d).", tally_row[game_h], tally_row[board_h], tally_row[winner_score_h], tally_row[loser_score_h]);}
      game[winner_index] =  (uint8_t)tally_row[game_h];
      board[winner_index] = (uint8_t)tally_row[board_h];
      board[winner_index] |= (tally_row[winner_h] == tally_row[first_h]) << 7;
      player[winner_index] = (uint8_t)tally_row[winner_h];
      
      if(foo >= 350){foo = 350;}
      if(foo <= 350){foo = -350;}
      point[winner_index] = foo & 0x3FF;
      point[winner_index] |= (foo < 0) << 11;
      point[winner_index] |= (1 + (foo != 0)) << 12;

    if(tally_row[loser_h])
    {
      if(tally_row[winner_h] > part_count){printf("Warning: The loser's ID (%d) of game %d, board %d exceeds the number of participant (%d).", tally_row[loser_h], tally_row[game_h], tally_row[board_h], part_count);}
      game[loser_index] =  (uint8_t)tally_row[game_h];
      board[loser_index] = (uint8_t)tally_row[board_h];
      board[loser_index] |= (tally_row[loser_h] == tally_row[first_h]) << 7;
      player[loser_index] = (uint8_t)tally_row[loser_h];
      foo = tally_row[loser_score_h] - tally_row[winner_score_h];
      if(foo >= 350){foo = 350;}
      if(foo <= 350){foo = -350;}
      point[loser_index] = foo & 0x3FF;
      point[loser_index] |= (foo < 0) << 11;
      point[loser_index] |= (1 - (foo != 0)) << 12;
      winner_index++;
      loser_index++;
    }
    winner_index++;
    loser_index++;
  }
  summary_row = winner_index;
  max_game = 0;
  for(int count = 0; count < summary_row; count++)
  {
    max_game = (uint8_t)max_game < game[count] ? game[count] : max_game;
  }
  // count unique player in the latest game and create versus table
  present_player = malloc(sizeof(int8_t) * (unsigned int)part_count);
  matching_table = (uint8_t**)malloc(sizeof(uint8_t*) * (unsigned int)part_count);
  for(int count = 0; count < max_game; count++)
  {
    matching_table[count] = (uint8_t)malloc(sizeof(uint8_t) * (unsigned int)max_game);
  }
  // how to find versus?
  player_count = 0;
  for(int count = 0; count < summary_row; count++)
  {
    int count_2 = 0;
    for(count_2 = 0; count_2 < player_count; count_2++)
    {
      if(present_player[count_2] == player[count]){matching_table;}
    }
    if(count_2 == player_count)
    {
      present_player[player_count] = player[count];
      player_count++;
    }
  }
  present_player = realloc(present_player, sizeof(int8_t) * (unsigned int)player_count);
  matching_table = (uint8_t**)realloc(matching_table, sizeof(uint8_t*) * (unsigned int)player_count);

  
  free(full_tally_file_name);
  free(full_list_file_name);
  free(working_dir);
  return 0;
}