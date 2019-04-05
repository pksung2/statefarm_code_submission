#include "lib.h"
#include "i8259.h"
#include "types.h"
#include "keyboard.h"
#include "terminal.h"
#include "system_calls.h"

/* 
*  KBDUS means US Keyboard Layout. This is a scancode table
*  used to layout a standard US keyboard. I have left some
*  comments in to give you an idea of what key is what, even
*  though I set it's array index to 0. You can change that to
*  whatever you want using a macro, if you wish! 
*
*  CREDIT:http://www.osdever.net/bkerndev/Docs/keyboard.htm 
*/

unsigned char kbdus[NUM_SCANCODES] =
{
  0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 
  ' ', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0,
  0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 
  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,     
  '*', 0, ' ', 0
};  

// KBDUS scancode table when shift is pressed
unsigned char kbdus_shift[NUM_SCANCODES] =
{
  0,  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 
  ' ', 'Q', 'W', 'E', 'R','T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 0,
  0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 
  0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 
  '*', 0, ' ', 0
};

// KBDUS scancode table when caps is pressed
unsigned char kbdus_caps[NUM_SCANCODES] =
{
  0,  0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 
  ' ', 'Q', 'W', 'E', 'R','T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0, 
  0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 
  0, '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, 
  '*', 0, ' ', 0
};

// KBDUS scancode table when caps and shift are pressed
unsigned char kbdus_caps_shift[NUM_SCANCODES] = 
{
  0,  0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 0, 
  ' ', 'q', 'w', 'e', 'r','t', 'y', 'u', 'i', 'o', 'p', '{', '}', 0,
  0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '\"', '~', 
  0, '|', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '<', '>', '?', 0,
  '*', 0, ' ', 0
};

int curr_terminal;
terminal_t terminals[TERMINALS_MAX];

int kbdus_mode = 0;
char input_buf[BUF_SIZE]; //Stores user input.
char input_history[TERMINALS_MAX][HISTORY_SIZE][BUF_SIZE]; //Stores previous input.
int history_index = 0; //Input history index

int horiz_index = 0; //Determines the x-value of the cursor.
int vert_index = 0; //Determines the y-value of the cursor.

int input_index = 0;  //Determines the location in the input.
int input_size = 0; //Counts how many characters the user input.

int alt_flag = 0; //ALT pressed
int cntrl_flag = 0; //CTRL pressed
int history_flag = 0; //At least one input has been passed to terminal read

int annoyed[TERMINALS_MAX]; //Give terminal some character.

int ATTRIB; //Used to modify ATTRIB in lib.c
int curr_attrib;

int processes_open; //Number of current processes open

/*  keyboard_init
 *  DESCRIPTION: This function initializes the keyboard on IRQ 1
 *  INPUTS: NONE
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: NONE
 */
void keyboard_init()
{
  enable_irq(kb_irq_num);
}


/*  keyboard_handler
 *  DESCRIPTION: This function handles keyboard interrupts
 *  INPUTS: NONE
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Outputs key press to terminal
 */
void keyboard_handler()
{
  int scancode = 0;
  vert_index = get_y();
  horiz_index = get_x();
  
  cli(); 
  // Get scancode from 0x60
  scancode = inb(KB_DATA_PORT);

  // Check for SHIFT, CAPS presses
  switch(scancode) 
  {
    // Set kbdus_mode (array selector) based on SHIFT press/release
    case LSHIFT_PRESS:
    case RSHIFT_PRESS:
      if(kbdus_mode == CAPS)
        kbdus_mode = SHIFT_CAPS;
      else
        kbdus_mode = SHIFT;
      break;
    case LSHIFT_RELEASE:
    case RSHIFT_RELEASE:
      if(kbdus_mode == SHIFT_CAPS)
        kbdus_mode = CAPS;
      else
        kbdus_mode = DEFAULT;
      break;
    // Set kbdus_mode (array selector) based on CAPS toggle
    case CAPS_PRESS:
      if(kbdus_mode == DEFAULT)
        kbdus_mode = CAPS;
      else if(kbdus_mode == SHIFT)
        kbdus_mode = SHIFT_CAPS;
      else if(kbdus_mode == CAPS)
        kbdus_mode = DEFAULT;
      else
        kbdus_mode = SHIFT;
      break;
    case ENTER_PRESS:
      terminals[curr_terminal].enter = 1;
      handle_enter(0);
      break;
    case BACKSPACE_PRESS:
      handle_backspace();
      break;
    case DELETE_PRESS:
      handle_delete();
      break;
    case LEFT_PRESS:
      handle_arrows(LEFT);
      break;
    case UP_PRESS:
      handle_arrows(UP);
      break;
    case RIGHT_PRESS:
      handle_arrows(RIGHT);
      break;
    case DOWN_PRESS:
      handle_arrows(DOWN);
      break;
    case CRTL_PRESS:
      cntrl_flag = 1;
      break;
    case CRTL_RELEASE:
      cntrl_flag = 0;
      break;
    case ALT_PRESS:
      alt_flag = 1;
      break;
    case ALT_RELEASE:
      alt_flag = 0;
      break;

    // Handle all other keypresses by calling handle_scancode
    default:
      handle_scancode(scancode);
      break;
  }

  // Send end of interrupt
  send_eoi(kb_irq_num);
  sti();
}


/*  handle_scancode
 *  DESCRIPTION: This function handles different scancodes (including special keys)
 *  INPUTS: scancode -- from keyboard
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Prints character pressed on keyboard
 */
void handle_scancode(int scancode)
{
  char ascii_value = 0;
  int i;
  send_eoi(kb_irq_num);
  sti();

  //Switch terminal if ALT+F1 or F2 or F3 is pressed
  if(alt_flag)
  {
    switch(scancode)
    {

      //Terminal 1
      case F1_PRESS:
        if(F1+1>TERMINALS_MAX)
          return;

        //Checks to see if terminal is started and if a new process is allowed to open.
        //Also checks to see if trying to access the current terminal.
        if((terminals[F1].started!=1 && processes_open==PROCESSES_MAX) || curr_terminal==F1)
        {
          max_terminal(F1);
          return;
        }

        //Initializes Terminal character
        if(terminals[F1].started!=1)
          annoyed[F1] = 0;

        //Starts new terminal with defined text attribute.
        ATTRIB = WHITE;
        start_terminal(F1);
        update_cursor(terminals[F1].y, terminals[F1].x);
        break;
      
      //Terminal 2
      case F2_PRESS:
        if(F2+1>TERMINALS_MAX)
          return;

        if((terminals[F2].started!=1 && processes_open==PROCESSES_MAX) || curr_terminal==F2)
        {
          max_terminal(F2);
          return;
        }

        if(terminals[F2].started!=1)
          annoyed[F2] = 0;

        ATTRIB = TEAL;
        start_terminal(F2);
        update_cursor(terminals[F2].y, terminals[F2].x);
        break;

      //Terminal 3
      case F3_PRESS:
        if(F3+1>TERMINALS_MAX)
          return;

        if((terminals[F3].started!=1 && processes_open==PROCESSES_MAX) || curr_terminal==F3)
        {
          max_terminal(F3);
          return;
        }

        if(terminals[F3].started!=1)
          annoyed[F3] = 0;

        ATTRIB = ZEST;
        start_terminal(F3);
        update_cursor(terminals[F3].y, terminals[F3].x);
        break;

      //Terminal 4
      case F4_PRESS:
        if(F4+1>TERMINALS_MAX)
          return;

        if((terminals[F4].started!=1 && processes_open==PROCESSES_MAX) || curr_terminal==F4)
        {
          max_terminal(F4);
          return;
        }

        if(terminals[F4].started!=1)
          annoyed[F4] = 0;

        ATTRIB++;
        start_terminal(F4);
        update_cursor(terminals[F4].y, terminals[F4].x);
        break;
      
      //Terminal 5
      case F5_PRESS:
        if(F5+1>TERMINALS_MAX)
          return;

        if((terminals[F5].started!=1 && processes_open==PROCESSES_MAX) || curr_terminal==F5)
        {
          max_terminal(F5);
          return;
        }

        if(terminals[F4].started!=1)
          annoyed[F4] = 0;

        ATTRIB++;
        start_terminal(F5);
        update_cursor(terminals[F5].y, terminals[F5].x);
        break;

      //Terminal 6
      case F6_PRESS:
        if(F6+1>TERMINALS_MAX)
          return;

        if((terminals[F6].started!=1 && processes_open==PROCESSES_MAX) || curr_terminal==F6)
        {
          max_terminal(F6);
          return;
        }

        if(terminals[F4].started!=1)
          annoyed[F4] = 0;

        ATTRIB++;
        start_terminal(F6);
        update_cursor(terminals[F6].y, terminals[F6].x);
        break;

      case ONE...ZERO:
        ATTRIB = scancode;
        break;

      default:
        break;
    }

    curr_attrib = ATTRIB;

    return;
  }

  // Return if scancode is not in kbdus array bounds
  if(scancode < 0 || scancode >= NUM_SCANCODES)
    return;

  /* Choose array based on kbdus_mode (shift/caps)
   * 0: normal
   * 1: shift
   * 2: caps
   * 3: shift + caps
   */
  switch(kbdus_mode)
  {
    case DEFAULT:
      ascii_value = kbdus[scancode];
      break;
    case SHIFT:
      ascii_value = kbdus_shift[scancode];
      break;
    case CAPS:
      ascii_value = kbdus_caps[scancode];
      break;
    case SHIFT_CAPS:
      ascii_value = kbdus_caps_shift[scancode];
      break;
    default:
      break;
  }

  // Return if non-handled key is pressed
  if(ascii_value == 0)
    return;

  //Clear screen if CTRL + L is pressed.
  if(cntrl_flag)
  {
    if(ascii_value == 'l')
    {
      clear();

      //Reset all params.
      input_index = 0;
      input_size = 0;
      horiz_index = 0;
      vert_index = 0;
      history_index = 0;

      //Clear input buffer
      for(i = 0; i < BUF_SIZE; i++)
      {
        input_buf[i] = '\0';
      }
      printf("391OS> ");

      return;
    }

  }

  // If upper 4 bits have data, then interrupt is generated on key release (ignore it)
  if((scancode & KEY_RELEASE_CHECK) == 0)
  { 
    if(scancode != ascii_value)
    {
      if(input_size < BUF_SIZE - 1)
      {
        // Add character to line buffer and print it
        int repeat = 1;
        int i;
        char temp_input_buf[BUF_SIZE];

        //4 SPACE characters if TAB is pressed
        if(scancode == TAB_PRESS)
          repeat = 4 - (horiz_index + 1)%4;

        //Increase the size counter
        input_size += repeat;

        //Shift over the input 'repeat' amount of times and store it in a temporary input buffer.
        for(i = input_index + repeat; i < input_size; i++)
        {
          temp_input_buf[i] = input_buf[i-repeat];
        }

        //Print character to screen and store in input buffer 'repeat' amount of times.
        for(i = 0; i < repeat; i ++)
        {
          putc(ascii_value);
          input_buf[input_index] = ascii_value;
          input_history[curr_terminal][0][input_index] = ascii_value;
          
          input_index++;
          horiz_index = get_x();
          vert_index = get_y();
        }

        //Fill in the current input buffer with the shifted input.
        for(i = input_index; i < input_size; i++)
        {
          input_buf[i] = temp_input_buf[i];
          input_history[curr_terminal][0][i] = input_buf[i];
          putc(input_buf[i]);
        }
        if((vert_index==NUM_ROWS-1) && (input_size!=input_index) && (horiz_index + input_size - input_index==NUM_COLS))
          vert_index--;
      }
    }
  }
  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index); 
}


/*  handle_enter
 *  DESCRIPTION: This function handles the enter key press
 *  INPUTS: sameline - used only when a user input is over the screen width limit
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Prints a newline on the screen.
 */
void handle_enter(int sameline)
{
  int i;
  int j;
  vert_index++;
  int temp_input_index = input_index;

  if(vert_index >= NUM_ROWS)
  {  
    vert_index--;        
  }
  
  while(temp_input_index != input_size)
  {
    temp_input_index++;
    handle_arrows(RIGHT);
  }

  putc('\n');
  input_buf[input_size] = '\n';          
  
  if(input_index == input_size)
    input_size++;

  input_index++;

  if(sameline)
    input_buf[input_size] = '\0';

  // 0: user pressed ENTER
  // 1: user input is filling up more than one line
  if(!sameline)
  { 
    if(input_size > 1)
    {
      input_buf[input_size] = '\n';

      //Push current input into the bottom row of history.
      for(i = 0; i < BUF_SIZE; i++)
      {
        input_history[curr_terminal][0][i] = input_buf[i];
      }

      //Shift all history up. This gets rid of the oldest input.
      for(i = 1; i < HISTORY_SIZE; i++)
      {
        //This loop traverses over the full length of one input buffer.
        for(j = 0; j < BUF_SIZE; j++)
        {
          input_history[curr_terminal][HISTORY_SIZE - i][j] = input_history[curr_terminal][HISTORY_SIZE - i - 1][j];

          //Clear the bottom of history once it's been copied
          if(i == HISTORY_SIZE - 1)
          {
            input_history[curr_terminal][0][j] = '\0';
          }

          //Get rid of newline characters in history
          if(input_history[curr_terminal][HISTORY_SIZE - i][j] == '\n')
            input_history[curr_terminal][HISTORY_SIZE - i][j] = '\0';
        }
      }
    }
    //Clear input params.
    input_size = 0;

    history_index = 0;
    history_flag = 1;
  }

  //Update local variables for cursor.
  horiz_index = 0;
  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index);       
}



/*  handle_backspace
 *  DESCRIPTION: This function handles the backspace key press
 *  INPUTS: NONE
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Deletes a character off of the screen. Shifts input.
 */
void handle_backspace()
{
  int new_horiz;
  int new_vert;
  int i;

  //Check if within the current input range
  if((input_index < 1) || (input_size < 1))
    return;
  //Check if there is an input
  else if(horiz_index == 0)
  { 
    //Check if valid for edge case
    if(vert_index == 0)
      return;
    //Update input position to traverse back a line
    horiz_index = NUM_COLS - 1;
    vert_index--;
  }
  else
    horiz_index--;

  //Point to the place cursor will be after the deletion
  new_horiz = horiz_index;
  new_vert = vert_index;
  input_index--;

  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index); 

  //Shift everything back one
  for(i = input_index; i < input_size; i++)
  { 
    input_buf[i] = input_buf[i+1];
    input_history[curr_terminal][0][i] = input_buf[i];
    putc(input_buf[i]);
  }

  input_size--;

  update_x(new_horiz);
  update_y(new_vert);
  update_cursor(new_vert, new_horiz); 
}


/*  handle_delete
 *  DESCRIPTION: This function handles the delete key press
 *  INPUTS: NONE
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Deletes a character off of the screen. Shifts input.
 */
void handle_delete()
{
  int i;

  //Check if within the current input range
  if((input_index < 0) || (input_size < 1) || (input_index==input_size))
    return;

  //Shift everything back one
  for(i = input_index; i < input_size; i++)
  { 
    input_buf[i] = input_buf[i+1];
    input_history[curr_terminal][0][i] = input_buf[i];
    putc(input_buf[i]);
  }

  input_size--;

  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index); 
}


/*  restore_prev_input
 *  DESCRIPTION: This function fills the current input buffer with the last input.
 *  INPUTS: idx - index of input buffer to restore
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Places a new buffer on the screen and changes contents of current input buffer.
 */
void restore_prev_input(int idx)
{
  int i;
  int check = 0;

  //Check if there's a history
  if(!history_flag)
    return;

  while(input_history[curr_terminal][idx][0] == '\0')
  {
    idx++;
    //Check if at top of history
    if(idx >= HISTORY_SIZE-1)
      return;
  }

  if(input_index > horiz_index)
  {
    horiz_index = NUM_COLS - (input_index - horiz_index);
    vert_index--;
  }
  else
  {
    horiz_index -= input_index;
  }

  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index);

  input_index = 0;
  input_size = 0;

  //Fill current input buffer with old one
  for(i = 0; i < BUF_SIZE; i++)
  {
    if(input_history[curr_terminal][idx][i] == '\0')
    {
      if(input_buf[i] != '\0')
        check = 1;
      else
        break;
    }

    input_buf[i] = input_history[curr_terminal][idx][i];

    //Increment values that need it
    if(input_buf[i] != '\0')
    {
      input_index++;
      input_size++;
      if(horiz_index==NUM_COLS-1)
      {
        horiz_index = 0;
        if(vert_index < NUM_ROWS-1)
          vert_index++;
      }
      else
        horiz_index++;
    }

    //Output to screen
    if(check)
      putc('\0');
    else
      putc(input_buf[i]);  

    check = 0;
  }

  history_index++;
  update_cursor(vert_index, horiz_index);
}

/*  restore_next_input
 *  DESCRIPTION: This function fills the current input buffer with the next input.
 *  INPUTS: idx - index of input buffer to restore
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Places a new buffer on the screen and changes contents of current input buffer.
 */
void restore_next_input(int idx)
{
  int i;
  int check = 0;

  while((input_history[curr_terminal][idx][0] == '\0') && (idx != 0))
  {
    idx--;
    //Check if at bottom of history
    if(idx <= 0)
      return;
  }

  if(input_index > horiz_index)
  {
    horiz_index = NUM_COLS - (input_index - horiz_index);
    vert_index--;
  }
  else
  {
    horiz_index -= input_index;
  }

  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index);

  input_index = 0;
  input_size = 0;

  //Fill current input buffer with newer one
  for(i = 0; i < BUF_SIZE; i++)
  {
    if(input_history[curr_terminal][idx][i] == '\0')
    {
      if(input_buf[i] != '\0')
        check = 1;
      else
        break;
    }

    input_buf[i] = input_history[curr_terminal][idx][i];

    //Increment values that need it
    if(input_buf[i] != '\0')
    {
      input_index++;
      input_size++;
      if(horiz_index==NUM_COLS-1)
      {
        horiz_index = 0;
        if(vert_index < NUM_ROWS-1)
          vert_index++;
      }
      else
        horiz_index++;
    }

    //Output to screen
    if(check)
      putc('\0');
    else
      putc(input_buf[i]);

    check = 0;
  }

  history_index--;
  update_cursor(vert_index, horiz_index);
}

/*  handle_arrows
 *  DESCRIPTION: This function handles the arrow keypresses
 *  INPUTS: NONE
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Moves cursor and user position on screen.
 */
void handle_arrows(int direction)
{
  switch(direction)
  {
    case LEFT:
      //Keypress invalid if no input or top left corner of screen.
      if((input_index == 0) || ((vert_index == 0) && (horiz_index == 0)))
        break;

      //Traverses back a line within a long input
      if(horiz_index == 0)
      {
        vert_index--;
        horiz_index = NUM_COLS - 1;
      }
      else
        horiz_index--;

      input_index--;
      break;

    case UP:
      //Access last used input
      restore_prev_input(history_index + 1);
      break;

    case RIGHT:
      //Keypress invalid if no input or bottom right corner of screen.
      if((input_index >= input_size) || ((vert_index == NUM_ROWS - 1) && (horiz_index == NUM_COLS - 1)))
        break;

      //Traverses forward in a line with a long input
      if(horiz_index == NUM_COLS - 1)
      {
        vert_index++;
        horiz_index = 0;
      }
      else
        horiz_index++;

      input_index++;
      break;

    //To be continued
    case DOWN:
      //Access last used input
      restore_next_input(history_index - 1);
      break;
  }
  update_x(horiz_index);
  update_y(vert_index);
  update_cursor(vert_index, horiz_index);       
}


/*  max_terminal
 *  DESCRIPTION: A fun function that adds a bit of character when
                 trying to switch terminals.
 *  INPUTS: NONE
 *  OUTPUTS: NONE
 *  RETURN VALUE: NONE
 *  SIDE EFFECTS: Prints messages on the screen.
 */
void max_terminal(int terminal_number)
{
  int i;

   //Reset all params.
   input_index = 0;
   input_size = 0;
   history_index = 0;

   //Clear input buffer
   for(i = 0; i < BUF_SIZE; i++)
   {
     input_buf[i] = '\0';
   }

   //A BUNCH of different cases. Basically a mini AI.
  if(annoyed[curr_terminal] < NOT_ANNOYED)
  {
    if(terminal_number==curr_terminal)
    {
      printf("\nYou are still in Terminal %d.\n", terminal_number+1, curr_terminal+1);
    }
    else
    {
      printf("\nCannot open terminal %d. Maximum processes open. You are still in Terminal %d.\n", terminal_number+1, curr_terminal+1);
    }
  }
  else if(annoyed[curr_terminal] < KINDA_ANNOYED-1)
  {
    printf("\n");
  }
  else if(annoyed[curr_terminal] < KINDA_ANNOYED)
  {
    if(terminal_number==curr_terminal)
    {
      printf("\nReally, you're in Terminal %d.\n", terminal_number+1, curr_terminal+1);
    }
    else
    {
      printf("\nTerminal %d won't open. You're still in Terminal %d.\n", terminal_number+1, curr_terminal+1);
    }
  }
  else if(annoyed[curr_terminal] < REALLY_ANNOYED-1)
  {
    printf("\n");
  }
  else if(annoyed[curr_terminal] < REALLY_ANNOYED)
  {
    if(terminal_number==curr_terminal)
    {
      printf("\n...You're really pushing my buttons.\n", terminal_number+1, curr_terminal+1);
    }
    else
    {
      printf("\nHey. Exit a shell or something, I can't open more!\n", terminal_number+1, curr_terminal+1);
    }
  }
  else if(annoyed[curr_terminal] < DEATH)
  {
    printf("\n");
  }
  else if (annoyed[curr_terminal] >= DEATH)
  {
    clear();
    update_x(DEATH_X);
    update_y(NUM_ROWS/2);
    ATTRIB = RED;
    printf("YOU ARE GOING TO DI");
    ATTRIB = curr_attrib;
    update_x(0);
    update_y(0);
    printf("Resetting 'annoyed' counter to prevent AI takeover.\n", terminal_number+1, curr_terminal+1);
    printf("391OS> ");
    annoyed[curr_terminal] = 0;
    return;
  }

  annoyed[curr_terminal]++;
  printf("391OS> ");
  vert_index=get_y();
  horiz_index=get_x();
}
