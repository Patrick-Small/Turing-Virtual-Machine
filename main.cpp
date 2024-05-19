/*
    Patrick Small
    CS_441 Computer Architecture
    02/28/2024

    This code is a Virtual Machine that takes a binary file with instructions and a tape file with data.
    Per the binary file's inructions and using the encodings specified in the included encodings.h file,
    the data will be altered and acted upon until instructed to stop. The final result of each tape will
    output, along with the number of head movements and instructions read for each tape. At the end of
    the program, it will output the total number of head moves and instructions read along ALL tapes.
*/

#include <iostream>
#include <fstream>
#include <string>
#include <deque>

#include "encodings.h"

using namespace std;

#define RAM_INSTRUCTIONS (1 << 11) //bit shifts 1 to the left 11 times
#define RAM_BYTES (RAM_INSTRUCTIONS * sizeof(tm_encoding))

//Function to execute
void execute(tm_encoding r[2048], deque<char>&, int&, int&);

int main(int argc, char* argv[])
{

    //Check the input #
    if (argc != 3) 
    {
        cerr << "Wrong number of inputs/no inputs" << endl;
    }

    //Opening and reading binary file contents into array of characters
    ifstream f(argv[1], ios::binary);

    tm_encoding ram[RAM_INSTRUCTIONS];
    memset(ram, 0, RAM_BYTES);
    f.read((char*)ram, RAM_BYTES);
    f.close();

    //Opening and reading data file contents
    ifstream dat(argv[2]);

    if (!dat)
    {
        cerr << "Error: File cannot be opened" << endl;
    }

    //Storing data into deque
    string data_string;
    deque<char> my_deq;
    int totHead = 0; //Head movements across each tape
    int totInst = 0; //Instructions read across each tape

    while (getline(dat, data_string))
    {
        //Filling the deque
        for (int i = 0; i < data_string.length(); i++) 
        {
            my_deq.push_back(data_string[i]);
        }

        //Filling the deque if blank line
        if (my_deq.size() == 0)
        {
            my_deq.push_back(' ');
        }

        //Go do stuff!
        execute(ram, my_deq, totHead, totInst);

        //Clear the deque
        my_deq.clear();
    }

    //Output the final counts
    /*
    Totals across all tapes...

           moves: 218

    instructions: 1849
    */

    cout << "Totals across all tapes..." << endl;
    cout << "       moves: " << totHead << endl;
    cout << "instructions: " << totInst << endl;

    //Exit
    return EXIT_SUCCESS;
}


//Function to execute the virtual machine
void execute(tm_encoding r[2048], deque<char>& d, int& totH, int& totI) //Don't I need to specify the length of the arrays here???
{
    //cout << "testing testing" << endl;
    //Local declarations & flags
    int addr = 0; //Address pointer to ram
    bool EQ = false; //Initialize EQ to false or 0
    bool NE = false; //Initialize NE to false or 0
    bool OR = false; //Initialize OR to false or 0
    int PC = 0; //PC initialized to NULL (0x000)
    int HC = 0; //Head Counter
    bool alpha[256]; //alpha flags start each new tape with all 256 ASCII characters NOT in the acceptable alphabet by default
    for (int i = 0; i < 256; i++)
    {
        alpha[i] = false;
    }
    alpha[32] = true; //Set the space to true
    int HEAD = 0; //Head placed on the left of tape
    bool FAIL = false; //Universal for stopping the machine, but also determines the output
    bool HALT = false; //Determines the output

    //Loop through the instructions until it ends
    while (r[addr].word.u != 0 && !FAIL)
    {

        tm_encoding IR = r[addr];

        switch (IR.generic.opcode)
        {

        case TM_OPCODE_ALPHA:

            //Set the binary to true
            alpha[IR.alpha.letter] = true;

            //Next instruction
            addr++;

            break;

        case TM_OPCODE_CMP:

            //Compare OR
            if (IR.cmp.oring)
            {
                OR = true;
            }

            else
            {
                OR = false;
            }

            //Compare BLANK
            if (IR.cmp.blank)
            {

                //If the character is a blank
                if ((uint16_t)(d[HEAD]) == 32)
                {
                    EQ = true;

                    //Next instruction
                    addr++;
                }

                else
                {
                    //Proceed
                    NE = true;

                    //Next instruction
                    addr++;
                }

            }

            //Compare LETTER
            else
            {

                //Check if the character is included
                if (alpha[d[HEAD]])
                {
                    //Check if head is pointing to that letter
                    if ((uint16_t)(d[HEAD]) == IR.cmp.letter)
                    {
                        EQ = true;

                        //Next instruction
                        addr++;
                    }

                    //It's not equal, so set NE to true
                    else
                    {
                        NE = true;

                        //Next instruction
                        addr++;
                    }

                }

                //Character isn't included, so the VM fails
                else
                {
                    FAIL = true;
                }

            }

            break;

        case TM_OPCODE_JMP:

            //Normal jump
            if (IR.jmp.eq && IR.jmp.ne)
            {
                //Set flags to false
                EQ = false;
                NE = false;

                //Set next instruction
                addr = IR.jmp.addr;
            }

            //Jump if EQ
            else if (IR.jmp.eq)
            {

                //If EQ flag set
                if (EQ)
                {
                    //Set flags to false
                    EQ = false;
                    NE = false;

                    //Set next instruction
                    addr = IR.jmp.addr;
                }
                else
                {
                    //Next Instruction
                    addr++;
                }

            }

            //Jump if NE
            else if (IR.jmp.ne)
            {

                //If NE flag set
                if (NE)
                {
                    //Set flags to false
                    EQ = false;
                    NE = false;

                    //Set next instruction
                    addr = IR.jmp.addr;
                }
                else
                {
                    //Next instruction
                    addr++;
                }
            }

            //Badly written assembly
            else
            {
                cout << "THIS SHOULD NEVER HAPPEN" << endl;
                break;
            }

            break;

        case TM_OPCODE_DRAW:

            if (IR.draw.blank)
            {
                //Place a 'space' at the head
                d[HEAD] = (char)(32);

                //Next instruction
                addr++;
            }
            else
            {
                //Place the letter at the head
                d[HEAD] = (char)(IR.draw.letter);

                //Next instruction
                addr++;
            }

            break;

        case TM_OPCODE_MOVE:

            //Left movement
            if (IR.move.amount < 0)
            {
                //If the head is already at the far left
                if (HEAD == 0)
                {
                    d.push_front(32);
                }

                //If there's room for the head to move left
                else
                {
                    HEAD--;
                }

                //Next instruction
                addr++;
            }

            //Right movement
            else
            {
                //If the head needs space to move to on the right
                if ((HEAD) == d.size() - 1)
                {
                    d.push_back(32);
                }

                //Move the HEAD right
                HEAD++;

                //Next instruction
                addr++;
            }

            //Increase Head Count
            HC++;

            break;

        case TM_OPCODE_STOP:

            if (IR.stop.halt)
            {
                FAIL = true;
                HALT = true;
            }
            else
            {
                FAIL = true;
            }

            break;

        default:

            cout << "unknown opcode " << IR.generic.opcode;
            cout << " at address " << addr << endl;

            break;

        }

        //Increment PC
        PC++;

    }

    //Store PC and HC into total values
    totH += HC;
    totI += PC;

    //Output results
    
    //VM halted
    if (HALT)
    {
        cout << "Halted after " << HC << " moves and " << PC << " instructions executed" << endl;
    }

    //VM failed
    else
    {
        cout << "Failed after " << HC << " moves and " << PC << " instructions executed" << endl;
    }
    
    //Need to figure out how many spaces exist at the beginning of string
    int spaces = 0; //How many spaces exist at the front of the tape

    //Count the amount of spaces
    for (int i = 0; i < HEAD; i++)
    {
        if (d[i] == 32)
        {
            spaces++;
        }
    }

    //Move HEAD to the front of the tape without whitespace
    HEAD = HEAD - spaces;

    //Output data
    for (int i = spaces; i < d.size(); i++)
    {
        cout << d[i];
    }
    cout << endl;

    //Output head placement
    for (int i = 0; i < HEAD; i++)
    {
        cout << " ";
    }
    cout << "^" << endl << endl;

    //Exit function
    return;
}