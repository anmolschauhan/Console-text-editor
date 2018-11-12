//ANMOL SINGH CHAUHAN
//ROLL NO : 20172099

#include <iostream>
#include <list>
#include <termios.h>
#include <sys/ioctl.h>
#include <fstream>
#include <string>
#include <cstdlib>

#define INSERT_MODE 1
#define NORMAL_MODE 2
#define COMMAND_MODE 3


using namespace std;

void ClearScreen()
{
	cout << "\033c";
}

void GotoXY(int x, int y)
{
	cout << "\033[" << x << ";" << y << "f";
}

void SaveSate(struct termios &o)
{
	tcgetattr(0, &o);
}

void SetCan(bool flag, struct termios original)
{
	struct termios t;
	setvbuf(stdin, NULL, _IONBF, 0);
	tcgetattr(0, &t);
	if(flag)
	{ 
    	t.c_lflag &= ~ICANON;
    	t.c_lflag &= ~ECHO;
    	tcsetattr(0, TCSANOW, &t);
    }
    else
    {
    	tcsetattr(0, TCSANOW, &original);
    }   
}

class InternalBuffer
{
public :
	list<char> beforeCursor;
	list<char> afterCursor;
	int rowMax, colMax;
	int row, col;
	int tempRow, tempCol;
	int rowReference;
	int mode;
	string command;

	InternalBuffer();
	void setMaxXY(void);
	void Display(void);
	void updateXY(void);
	void Right(void)
	{
		if(afterCursor.size() > 0)
		{
			beforeCursor.push_back(afterCursor.front());
			afterCursor.pop_front();
		}
	}
	void Left(void)
	{
		if(beforeCursor.size() > 0)
		{
			afterCursor.push_front(beforeCursor.back());
			beforeCursor.pop_back();
		}
	}
};

InternalBuffer::InternalBuffer()
{
	mode = NORMAL_MODE;
	row = 1;
	col = 1;
	rowReference = 0;
	setMaxXY();
}

void InternalBuffer::setMaxXY(void)
{
	struct winsize size;
	ioctl(0,TIOCGWINSZ, &size);
	rowMax = size.ws_row;
	colMax = size.ws_col;
}

void InternalBuffer::Display(void)
{
	//list<char>::iterator i;

	row = 1;
	col = 1;
	auto i = beforeCursor.begin();

	for(int j = 0; j < rowReference; j++)
	{
		for(int k = 0; k < colMax; k++)
		{
			if(*i == '\n')
			{
				i++;
				break;
			}
			i++;
		}
	}

	while(i != beforeCursor.end())
	{
		GotoXY(row, col);
		cout << *i;
		if(*i == '\n')
		{
			col = 1;
			row++;
			i++;
			continue;
		}
		col++;
		if(col > colMax)
		{
			col = 1;
			row++;
		}
		i++;	
	}
	tempRow = row;
	tempCol = col;
	i = afterCursor.begin();
	while(i != afterCursor.end())
	{
		if(row == rowMax-1) // test for scrolling
		{
			break;
		}
		GotoXY(row, col);
		cout << *i;
		if(*i == '\n')
		{
			col = 1;
			row++;
			i++;
			continue;
		}
		col++;
		if(col > colMax)
		{
			col = 1;
			row++;
		}
		i++;	
	}
	if(tempRow >= (rowMax - 1))
	{
		rowReference++;
	}	
	if(tempRow <= 1)
	{
		rowReference--;
	}

	/*if(tempRow <= 1)
	{
		rowReference++;
	}*/

	for(int i = 1; i <= colMax; i++)
	{
		GotoXY(rowMax-1, i);
		cout << '-';
	}
	GotoXY(rowMax, 2);
	if(mode == NORMAL_MODE)
		cout << " NORMAL ";
	else if(mode == INSERT_MODE)
		cout << " INSERT ";
	else if(mode == COMMAND_MODE)
		cout << " COMMAND :";

	if(mode == NORMAL_MODE || mode == INSERT_MODE)
	{
		GotoXY(rowMax, colMax - 18);
		cout << "row : " << tempRow << " col : " << tempCol;
		GotoXY(tempRow, tempCol);
	}
	if(mode == COMMAND_MODE)
	{
		GotoXY(rowMax, 12);
		cout << command;
	}
}

void InternalBuffer::updateXY(void)
{
	row = 1;
	col = 1;
	auto i = beforeCursor.begin();

	for(int j = 0; j < rowReference; j++)
	{
		for(int k = 0; k < colMax; k++)
		{
			if(*i == '\n')
			{
				i++;
				break;
			}
			i++;
		}
	}
	while(i != beforeCursor.end())
	{
		if(*i == '\n')
		{
			col = 1;
			row++;
			i++;
			continue;
		}
		col++;
		if(col > colMax)
		{
			col = 1;
			row++;
		}
		i++;	
	}
	tempRow = row;
	tempCol = col;
	i = afterCursor.begin();
	while(i != afterCursor.end())
	{
		if(row == rowMax-1) // test for scrolling
		{
			break;
		}
		if(*i == '\n')
		{
			col = 1;
			row++;
			i++;
			continue;
		}
		col++;
		if(col > colMax)
		{
			col = 1;
			row++;
		}
		i++;	
	}
	if(tempRow >= (rowMax - 1))
	{
		rowReference++;
	}	
	if(tempRow <= 1)
	{
		rowReference--;
	}
}

void ArrowKeyControl(char &temp, InternalBuffer &internalBuffer)
{
	if((int)temp == 27)
	{
		char temp1 = getchar();
		char temp2 = getchar();
		if(temp1 == 27 && temp2 == 27 && internalBuffer.mode == INSERT_MODE)
		{
			internalBuffer.mode = NORMAL_MODE;
		}
		if(temp2 == 65)					// UP
		{
			for(int i = 0; i < internalBuffer.rowMax; i++)
			{
				if(internalBuffer.beforeCursor.back() == '\n')
				{
					internalBuffer.updateXY();
					i += (internalBuffer.colMax - internalBuffer.tempCol);
				}
				internalBuffer.Left();
			}
			return;
		}
		else if(temp2 == 66)				// DOWN
		{
			for(int i = 0; i < internalBuffer.rowMax; i++)
			{
				if(internalBuffer.afterCursor.front() == '\n')
				{
					internalBuffer.updateXY();
					i += (internalBuffer.colMax - internalBuffer.tempCol);
				}
				internalBuffer.Right();
			}	
			return;
		}
		else if(temp2 == 67)				// RIGHT
		{
			internalBuffer.Right();
			return;
		}
		else if(temp2 == 68)				// LEFT
		{
			internalBuffer.Left();
			return;
		}
		else if(temp1 == 91 && temp2 == 51 && internalBuffer.mode == INSERT_MODE)	// DELETE
		{
			char temp3 = getchar();
			if(temp3 == 126)
			{
				internalBuffer.afterCursor.pop_front();
			}
			return;
		}
	}
	/*else if(internalBuffer.mode == NORMAL_MODE)
	{
		if(temp == 73) // i
		{
			for(int i = 0; i < internalBuffer.rowMax; i++)
			{
				if(internalBuffer.beforeCursor.back() == '\n')
				{
					internalBuffer.updateXY();
					i += (internalBuffer.colMax - internalBuffer.tempCol);
				}
				internalBuffer.Left();
			}
			return;
		}
		else if(temp == 74) // j
		{
			internalBuffer.Left();
			return;
		}
		else if(temp == 75) // k
		{
			for(int i = 0; i < internalBuffer.rowMax; i++)
			{
				if(internalBuffer.afterCursor.front() == '\n')
				{
					internalBuffer.updateXY();
					i += (internalBuffer.colMax - internalBuffer.tempCol);
				}
				internalBuffer.Right();
			}	
			return;
		}
		else if(temp == 76) // l
		{
			internalBuffer.Right();
			return;
		}
	}*/
}

void InsertModeControl(char &temp, InternalBuffer &internalBuffer)
{
	if((int)temp >= 32 && (int)temp <= 125)
	{
		internalBuffer.beforeCursor.push_back(temp);
	}
		else if((int)temp == 32)
	{
		internalBuffer.beforeCursor.push_back(temp);
	}
	else if((int)temp == 10)		//	Enter handling
	{
		internalBuffer.beforeCursor.push_back(temp);
	}
	else if((int)temp == 9)
	{
		ClearScreen();
		internalBuffer.Display();
		for(int x = 0; x < (5 - (internalBuffer.tempCol % 5)); x++)
		{
			internalBuffer.beforeCursor.push_back((char)32);
		}
	}
	else if((int)temp == 127)		//Backspace handling
	{	
		if(internalBuffer.beforeCursor.size() > 0)
			internalBuffer.beforeCursor.pop_back();
	}
}

int main(int argc, char * argv[])
{
	struct termios original;
	InternalBuffer internalBuffer;
	ifstream is(argv[1]);
	
	
	internalBuffer.mode = NORMAL_MODE;

	char c;
	while(is.get(c))
		internalBuffer.beforeCursor.push_back(c);

	internalBuffer.beforeCursor.swap(internalBuffer.afterCursor);

	SaveSate(original); 
	SetCan(true, original);
	//ClearScreen();

	internalBuffer.Display();

	char temp;
	while(true)
	{	
		ClearScreen();
		internalBuffer.setMaxXY();
		internalBuffer.Display();
		
		temp = getchar();

		if(internalBuffer.mode == NORMAL_MODE)
		{
			ArrowKeyControl(temp, internalBuffer);
			if(temp == 'i')
			{
				internalBuffer.mode = INSERT_MODE;
			}
			else if(temp == ':')
			{
				internalBuffer.mode = COMMAND_MODE;
			}
			else if(temp == 'g')
			{
				if(getchar() == 'g')
				{
					while(internalBuffer.beforeCursor.size() > 0)
					{
						internalBuffer.Left();
					}
					internalBuffer.updateXY();
				}
			}
			else if(temp == 'G')
			{
				while(internalBuffer.afterCursor.size() > 0)
				{
					internalBuffer.Right();
				}
				internalBuffer.updateXY();
			}
			else if(temp == 'r')
			{
				char rep = getchar();
				internalBuffer.afterCursor.pop_front();
				internalBuffer.afterCursor.push_front(rep);
				internalBuffer.updateXY();
			}
		}
		else if(internalBuffer.mode == INSERT_MODE)
		{
			ArrowKeyControl(temp, internalBuffer);
			InsertModeControl(temp, internalBuffer);
		}
		else if(internalBuffer.mode == COMMAND_MODE)
		{
			if((int)temp >= 32 && (int)temp <= 125)// 97 122
			{
				internalBuffer.command += temp;
			}
			if((int)temp == 127 && internalBuffer.command.size() > 0)
			{
				internalBuffer.command.erase(internalBuffer.command.end()-1);
			}
			if((int)temp == 10)
			{
				if(internalBuffer.command == "wq")
				{
					ofstream os(argv[1]);
					list<char>::iterator p;
					p = internalBuffer.beforeCursor.begin();

					while(p != internalBuffer.beforeCursor.end())
					{
						os.put(*p);
						p++;
					}
					p = internalBuffer.afterCursor.begin();

					while(p != internalBuffer.afterCursor.end())
					{
						os.put(*p);
						p++;
					}
					is.close();
					os.close();
					break;
				}
				else if(internalBuffer.command == "w")
				{
					ofstream os(argv[1]);
					list<char>::iterator p;
					p = internalBuffer.beforeCursor.begin();

					while(p != internalBuffer.beforeCursor.end())
					{
						os.put(*p);
						p++;
					}
					p = internalBuffer.afterCursor.begin();

					while(p != internalBuffer.afterCursor.end())
					{
						os.put(*p);
						p++;
					}
					internalBuffer.mode = NORMAL_MODE;
					internalBuffer.command.clear();
				}
				else if(internalBuffer.command == "q!")
				{
					is.close();
					break;
				}
				else
				{
					ClearScreen();
					GotoXY(1,1);
					system(internalBuffer.command.c_str());
					getchar();
					internalBuffer.command.clear();
					internalBuffer.mode = NORMAL_MODE;
				}
			}
		}
		
	}

	
	/*ofstream os(argv[1]);
	list<char>::iterator p;
	p = internalBuffer.beforeCursor.begin();

	while(p != internalBuffer.beforeCursor.end())
	{
		os.put(*p);
		p++;
	}
	p = internalBuffer.afterCursor.begin();

	while(p != internalBuffer.afterCursor.end())
	{
		os.put(*p);
		p++;
	}*/
	

	ClearScreen();
	SetCan(false, original);
	GotoXY(1,1);
	
	return 0;
}
