#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include <string>
#include <fstream>
//#include <mmsystem.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <vector>
#include"socketKhai.h"
#include <windows.h>

using namespace std;

//use MACRO -> Nice
#define FOR_ALL_POINT_I_J for (int i = 0; i < boardSize; i++)\
	for (int j = 0; j < boardSize; j++) 

const string mapPlayer1 = "mapPlayer1.txt";
const string mapPlayer2 = "mapPlayer2.txt";
const char isWATER = '0';
const char isHIT = 'X';
const char isMISS = 'M';
const char blue[] = "\x1B[34m";
const char normal[] = "\x1B[0m";
const char bblue[] = "\x1B[94m";
const char red[] = "\x1B[31m";
const char yellow[] = "\x1B[93m";

class CORD {
public:
	int x;
	int y;
	CORD(int x, int y) {
		this->x = x;
		this->y = y;
	}
};

struct PLAYER {
	string map;
	char** grid = NULL;
	int boom = 0;
	int score = 0;
};

bool gameRunning = false;
int boardSize = 0;
int isServer;

int SelectMode() {
	cout << "_________***BATTLESHIP***_________" << endl;

	cout << "1. Human vs Human Offline Match" << endl;
	cout << "2. Human vs AI Match" << endl;
	cout << "3. Human vs Human Online Match" << endl;
	int ch = 0;
	while (ch != 1 && ch != 2 && ch!=3) {
		cout << "Please select one game mode...	" << endl;
		cin >> ch;
	}
	return ch;
}

bool LoadMap(PLAYER* player) {
	ifstream fileInput(player->map);
	if (fileInput.fail()) return false;
	fileInput >> boardSize; //Load the size of map(board)
	if (boardSize <= 0) return false;
	//Creat dynamic memory
	player->grid = new char* [boardSize];
	for (int i = 0; i < boardSize; i++) {
		(player->grid)[i] = new char[boardSize];
	}
	//Read each point in map of player
	while (!fileInput.eof()) {
		FOR_ALL_POINT_I_J{
				fileInput >> player->grid[i][j];
		}
	}
	fileInput.close();
	return true;
}

bool IsNewShip(vector<char> shipName, char thisPos) {
	for (int k = 0; k < shipName.size(); k++)
		if ((thisPos == shipName.at(k))) return false;
	if (thisPos != isWATER) return true;
	return false;
}

void DrawBoard(int thisPlayer, PLAYER player, vector<char> shipName)
{
	//Draws the board for a player (thisPlayer)
	cout << "PLAYER " << thisPlayer << "'s GAME BOARD\n";
	cout << "----------------------\n";

	//Loop through top row (board_width) and number columns
	cout << "   ";
	for (int w = 0; w < boardSize; ++w) {
		if (w < 10)
			//Numbers only 1 character long, add two spaces after
			cout << w << "  ";
		else if (w >= 10)
			//Numbers 2 characters long, add only 1 space after
			cout << w << " ";
	}
	cout << "\n";

	//Loop through each grid point and display to console
	for (int h = 0; h < boardSize; ++h) {
		for (int w = 0; w < boardSize; ++w) {

			//If this is the FIRST (left) grid point, number the grid first
			if (w == 0) cout << h << " ";
			//If h was 1 character long, add an extra space to keep numbers lined up
			if (w < 10 && w == 0) cout << " ";
			//Display contents of this grid (if game isn't running yet, we are placing ships
			//so display the ships
			if (gameRunning == false) cout << player.grid[h][w] << "  ";
			//Don't show ships, BUT show damage if it's hit
			int logic = 1;
			for (int i = 0; i < shipName.size(); i++) logic = logic && (player.grid[h][w] != shipName.at(i));
			if (gameRunning == true && logic)
			{
				if (player.grid[h][w] == isHIT) cout << red << player.grid[h][w] << normal << "  ";
				else if (player.grid[h][w] == isMISS) cout << yellow << player.grid[h][w] << normal << "  ";
				else cout << blue << player.grid[h][w] << normal << "  ";
			}
			logic = !logic;
			if (gameRunning == true && logic)
			{
				cout << blue << isWATER << normal << "  ";
			}
			//If we have reached the border.. line feed
			if (w == boardSize - 1) cout << "\n";
		}
	}
	cout << "----------------------\n";
}

bool UserInputAttack(int& x, int& y, int theplayer, int gameMode, PLAYER enemyPlayer)
{
	bool goodInput = false;
	if (gameMode == 2 && theplayer == 2) {
		cout << "\nAI IS ATTACKING ..";
		//Your mission is to creat x,y for AI
		Sleep(1000);
		cout << "..";
		Sleep(2000);
		cout << "..";
		Sleep(1000);
		{

			srand(time(NULL));
			vector<CORD> cord;
			FOR_ALL_POINT_I_J
				if (enemyPlayer.grid[i][j] != isHIT && enemyPlayer.grid[i][j] != isMISS)
					if (((i % 2 == 1) && (j % 2 == 1)) || ((i % 2 == 0) && (j % 2 == 0)))
						cord.push_back(CORD(i, j));

			if (cord.size() == 0) {
				FOR_ALL_POINT_I_J
					if (enemyPlayer.grid[i][j] != isHIT && enemyPlayer.grid[i][j] != isMISS)
						cord.push_back(CORD(i, j));
			}

			int hunt = rand() % cord.size();
			x = cord.at(hunt).x;
			y = cord.at(hunt).y;
		}
		//x = 4; y = 0;
		goodInput = true;
	}
	if ((gameMode == 2 && theplayer == 1) || gameMode == 1) {
		cout << "\nPLAYER " << theplayer << ", ENTER COORDINATES TO ATTACK: ";

		cin >> x >> y; //the first one is horizontal
		if (x < 0 || x >= boardSize) return goodInput;
		if (y < 0 || y >= boardSize) return goodInput;
		goodInput = true;

	}
	return goodInput;
}

bool UserInputAttackOnline(int& x, int& y, int thePlayer, SOCKET server, SOCKET client)
{
	bool goodInput = false;
	if (isServer == 1)
		if (thePlayer == 1) {
			cout << "\nPLAYER " << thePlayer << ", ENTER COORDINATES TO ATTACK: ";

			cin >> x >> y; //the first one is horizontal
			if (x < 0 || x >= boardSize) return goodInput;
			if (y < 0 || y >= boardSize) return goodInput;
			SendCord(client, x, y);
			goodInput = true;
			
		}
		else {
			cout << "\nPLAYER " << thePlayer << ", IS ENTERING COORDINATES TO ATTACK...";
			Sleep(1000);
			ReceiveCord(client, x, y);
			goodInput = true;
		}
	else 
		if (thePlayer == 2) {
			cout << "\nPLAYER " << thePlayer << ", ENTER COORDINATES TO ATTACK: ";

			cin >> x >> y; //the first one is horizontal
			if (x < 0 || x >= boardSize) return goodInput;
			if (y < 0 || y >= boardSize) return goodInput;
			SendCord(server, x, y);
			goodInput = true;

		}
		else {
			cout << "\nPLAYER " << thePlayer << ", IS ENTERING COORDINATES TO ATTACK...";
			Sleep(1000);
			ReceiveCord(server, x, y);
			goodInput = true;
		}
	
	return goodInput;
}

int GameOverCheck(PLAYER thisPlayer, int thisIndex, PLAYER enemyPlayer, int enemyIndex, vector<char> shipName)
{
	int winner = thisIndex;
	//Loop through enemy board
	for (int w = 0; w < boardSize; ++w) {
		for (int h = 0; h < boardSize; ++h) {
			//If any ships remain, game is NOT over
			for (int k = 0; k < shipName.size(); k++) if (shipName.at(k) == enemyPlayer.grid[h][w])
			{
				winner = 0;
			}
		}
	}
	if ((enemyPlayer.boom == 0) && (thisPlayer.boom == 0))
		if (thisPlayer.score > enemyPlayer.score) winner = thisIndex;
		else if (thisPlayer.score < enemyPlayer.score) winner = enemyIndex;
		else {
			winner = 3;
		}

	//If we get here, thisPlayer won, game over!
	return winner;
}

int SizeOfShip(PLAYER player1, char shipName) {
	int count = 0;
	FOR_ALL_POINT_I_J if (player1.grid[i][j] == shipName) count++;
	return count;
}

bool IsPlaceable(int x, int y, int dir, PLAYER& player2, char shipName, int shipSize) {
	bool logic = true;
	srand(time(NULL));

	if (dir == 0)
	{
		int left = rand() % 2;
		if (left == 1) {
			for (int i = y - shipSize + 1; i < y + 1; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[x][i] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if(logic==true) for (int i = y - shipSize + 1; i < y + 1; i++) player2.grid[x][i] = shipName;
		}
		else {
			for (int i = y; i < y + shipSize; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[x][i] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = y; i < y + shipSize; i++) player2.grid[x][i] = shipName;
		}
	}

	if (dir == 1)
	{
		int up = rand() % 2;
		if (up == 0) {
			for (int i = x - shipSize + 1; i < x + 1; i++)
				if (0 <=i && i < boardSize)
					if (player2.grid[i][y] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = x - shipSize + 1; i < x + 1; i++) player2.grid[i][y] = shipName;
		}
		else {
			for (int i = x; i < x + shipSize; i++)
				if (0 <= i && i < boardSize)
					if (player2.grid[i][y] != isWATER) {
						logic = false;
						break;
					}
					else;
				else {
					logic = false;
					break;
				}
			if (logic == true) for (int i = x; i < x + shipSize; i++) player2.grid[i][y] = shipName;
		}
	}
	return logic;
}

void CreatMapAI(PLAYER player1, PLAYER& player2, vector<char> shipName) {
	//Your mission is to creat a mapPlayer2.txt which has AI's map, depending on mapPlayer1.txt
	vector<int> shipSize; //take all the sizes of ships to a equivalent vector like shipName
	for (int thisShip = 0; thisShip < shipName.size(); thisShip++) shipSize.push_back(SizeOfShip(player1, shipName.at(thisShip)));

	srand(time(NULL));
	int x, y, dir;
	for (int thisShip = 0; thisShip < shipName.size(); thisShip++) {
		do {
			x = rand() % boardSize; //the first set point of a ship
			y = rand() % boardSize;
			dir = rand() % 2; //0 is horizontal, 1 is vertical

		} while (!IsPlaceable(x, y, dir, player2, shipName.at(thisShip),shipSize.at(thisShip)));
	}
}


int main()
{
	int gameMode = SelectMode();
	if (gameMode == 3) {
		PLAYER player[3]; //Use player 1 & 2, 0 is ignored
		player[1].map = mapPlayer1;
		player[2].map = mapPlayer2;
		cout << "If you are server, you are player 1. Will you be server ? (1/0) " << endl;
		cin >> isServer;
		if (isServer == 1) {
			WSADATA WSAData;

			SOCKET server, client;

			SOCKADDR_IN serverAddr, clientAddr;

			WSAStartup(MAKEWORD(2, 0), &WSAData);
			server = socket(AF_INET, SOCK_STREAM, 0);

			serverAddr.sin_addr.s_addr = INADDR_ANY;
			serverAddr.sin_family = AF_INET;
			serverAddr.sin_port = htons(5555);

			bind(server, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
			listen(server, 0);

			cout << "Listening for incoming connections..." << endl;

			int clientAddrSize = sizeof(clientAddr);
			if ((client = accept(server, (SOCKADDR*)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
			{
				cout << "Client connected!" << endl;
			}
			else {
				cout << "Can't connect to client" << endl;
				return -1;
			}
			cout << "PLEASE CREATE YOUR OWN FLEE IN TXT FILE !!! " << endl;
			system("pause");
			ReceiveFile(client, "mapPlayer2.txt");
			Sleep(500);
			SendFile(client, "mapPlayer1.txt");
			Sleep(500);

			if (!LoadMap(&player[1])) {
				cout << "Failed to load player 1's map!" << endl;
				return -1;
			}
			vector<char> shipName; //vector saves types of ship
			shipName.push_back('1');

			FOR_ALL_POINT_I_J{
						if (IsNewShip(shipName, player[1].grid[i][j])) shipName.push_back(player[1].grid[i][j]);
			}
			player[1].boom = (player[2].boom = boardSize * boardSize / 2); //config the number of turns = boom

			if (!LoadMap(&player[2])) {
				cout << "Failed to load player 2's map!" << endl;
				return -1;
			}

			//ASCII 0 - 9: 48 - 57	|	A - B: 65 - 90


			//for (int i = 0; i < shipName.size(); i++) cout << shipName.at(i);

			/*
			FOR_ALL_POINT_I_J cout << player[1].grid[i][j] << ' ';
			*/

			cout << "PLAYERS' MAPS LOADED, READY TO FIGHT!!!" << endl;
			system("pause");
			//************************************
			//Ready to play the game
			gameRunning = true;
			int thisPlayer = 1;
			int aWin = 0;
			do {
				//Because we are ATTACKING now, the 
				//opposite players board is the display board
				int enemyPlayer;
				if (thisPlayer == 1) enemyPlayer = 2;
				if (thisPlayer == 2) enemyPlayer = 1;
				system("cls");
				DrawBoard(enemyPlayer, player[enemyPlayer], shipName);

				cout << endl << "PLAYER " << thisPlayer << "'S SCORE: " << player[thisPlayer].score << endl;
				cout << "PLAYER " << thisPlayer << "'S BOOMS LEFT: " << player[thisPlayer].boom << endl;
				//Get attack coords from this player
				bool goodInput = false;
				int x = 0, y = 0;
				while (goodInput == false) {
					goodInput = UserInputAttackOnline(x, y, thisPlayer, server, client);
				}
				player[thisPlayer].boom--;
				//Check board; if a ship is there, set as HIT.. otherwise MISS
				for (int thisShip = 0; thisShip < shipName.size(); thisShip++)
					if (player[enemyPlayer].grid[x][y] == shipName.at(thisShip)) {//nice shot
						PlaySound(TEXT("Explosion+2.wav"), NULL, SND_FILENAME | SND_ASYNC);
						//Sleep(3500);
						FOR_ALL_POINT_I_J
							if (player[enemyPlayer].grid[i][j] == shipName.at(thisShip)) {
								player[enemyPlayer].grid[i][j] = isHIT;
								player[thisPlayer].score++;
							}
					}
				if (player[enemyPlayer].grid[x][y] == isWATER) player[enemyPlayer].grid[x][y] = isMISS;

				system("cls");
				DrawBoard(enemyPlayer, player[enemyPlayer], shipName);
				cout << endl;
				system("pause");

				//Check to see if the game is over
				//If 0 is returned, nobody has won yet
				aWin = GameOverCheck(player[thisPlayer], thisPlayer, player[enemyPlayer], enemyPlayer, shipName);
				if (aWin != 0) {
					gameRunning = false;
					break;
				}

				//Alternate between each player as we loop back around
				thisPlayer = (thisPlayer == 1) ? 2 : 1;
			} while (gameRunning);

			if (aWin == 3) {
				system("cls");
				cout << "	WE HAVE A TIE GAME!!! \n\n\n\n";
				delete[](player[1].grid);
				delete[](player[2].grid);
				return 0;
			}
			if (aWin == 2 && gameMode == 2) {
				system("cls");
				cout << "YOU HAVE LOST TO AI!!! \n\n\n\n";
				delete[](player[1].grid);
				delete[](player[2].grid);
				return 0;
			}
			system("cls");
			PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "\n\nCONGRATULATIONS!!!  PLAYER " << aWin << " HAS WON THE GAME!\n\n\n\n";

			system("pause");
			closesocket(client);
			delete[](player[1].grid);
			delete[](player[2].grid);
			return 0;

		}
		else {
			WSADATA WSAData;
			SOCKET server, client;
			SOCKADDR_IN addr;

			WSAStartup(MAKEWORD(2, 0), &WSAData);
			server = socket(AF_INET, SOCK_STREAM, 0);

			addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // replace the ip with your futur server ip address. If server AND client are running on the same computer, you can use the local ip 127.0.0.1
			addr.sin_family = AF_INET;
			addr.sin_port = htons(5555);

			connect(server, (SOCKADDR*)&addr, sizeof(addr));
			cout << "Connected to server!" << endl;
			cout << "PLEASE CREATE YOUR OWN FLEE IN TXT FILE !!! " << endl;
			system("pause");
			SendFile(server, "mapPlayer2.txt");
			Sleep(500);
			ReceiveFile(server, "mapPlayer1.txt");
			Sleep(500);

			if (!LoadMap(&player[2])) {
				cout << "Failed to load player 2's map!" << endl;
				return -1;
			}
			vector<char> shipName; //vector saves types of ship
			shipName.push_back('1');

			FOR_ALL_POINT_I_J{
						if (IsNewShip(shipName, player[2].grid[i][j])) shipName.push_back(player[2].grid[i][j]);
			}
			player[1].boom = (player[2].boom = boardSize * boardSize / 2); //config the number of turns = boom

			if (!LoadMap(&player[1])) {
				cout << "Failed to load player 2's map!" << endl;
				return -1;
			}

			//ASCII 0 - 9: 48 - 57	|	A - B: 65 - 90


			//for (int i = 0; i < shipName.size(); i++) cout << shipName.at(i);

			/*
			FOR_ALL_POINT_I_J cout << player[1].grid[i][j] << ' ';
			*/

			cout << "PLAYERS' MAPS LOADED, READY TO FIGHT!!!" << endl;
			system("pause");
			//************************************
			//Ready to play the game
			gameRunning = true;
			int thisPlayer = 1;
			int aWin = 0;
			do {
				//Because we are ATTACKING now, the 
				//opposite players board is the display board
				int enemyPlayer;
				if (thisPlayer == 1) enemyPlayer = 2;
				if (thisPlayer == 2) enemyPlayer = 1;
				system("cls");
				DrawBoard(enemyPlayer, player[enemyPlayer], shipName);

				cout << endl << "PLAYER " << thisPlayer << "'S SCORE: " << player[thisPlayer].score << endl;
				cout << "PLAYER " << thisPlayer << "'S BOOMS LEFT: " << player[thisPlayer].boom << endl;
				//Get attack coords from this player
				bool goodInput = false;
				int x = 0, y = 0;
				while (goodInput == false) {
					goodInput = UserInputAttackOnline(x, y, thisPlayer, server, NULL);
				}
				player[thisPlayer].boom--;
				//Check board; if a ship is there, set as HIT.. otherwise MISS
				for (int thisShip = 0; thisShip < shipName.size(); thisShip++)
					if (player[enemyPlayer].grid[x][y] == shipName.at(thisShip)) {//nice shot
						PlaySound(TEXT("Explosion+2.wav"), NULL, SND_FILENAME | SND_ASYNC);
						//Sleep(3500);
						FOR_ALL_POINT_I_J
							if (player[enemyPlayer].grid[i][j] == shipName.at(thisShip)) {
								player[enemyPlayer].grid[i][j] = isHIT;
								player[thisPlayer].score++;
							}
					}
				if (player[enemyPlayer].grid[x][y] == isWATER) player[enemyPlayer].grid[x][y] = isMISS;

				system("cls");
				DrawBoard(enemyPlayer, player[enemyPlayer], shipName);
				cout << endl;
				system("pause");

				//Check to see if the game is over
				//If 0 is returned, nobody has won yet
				aWin = GameOverCheck(player[thisPlayer], thisPlayer, player[enemyPlayer], enemyPlayer, shipName);
				if (aWin != 0) {
					gameRunning = false;
					break;
				}

				//Alternate between each player as we loop back around
				thisPlayer = (thisPlayer == 1) ? 2 : 1;
			} while (gameRunning);

			if (aWin == 3) {
				system("cls");
				cout << "	WE HAVE A TIE GAME!!! \n\n\n\n";
				delete[](player[1].grid);
				delete[](player[2].grid);
				return 0;
			}
			if (aWin == 2 && gameMode == 2) {
				system("cls");
				cout << "YOU HAVE LOST TO AI!!! \n\n\n\n";
				delete[](player[1].grid);
				delete[](player[2].grid);
				return 0;
			}
			system("cls");
			PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
			cout << "\n\nCONGRATULATIONS!!!  PLAYER " << aWin << " HAS WON THE GAME!\n\n\n\n";

			system("pause");

			closesocket(server);
			WSACleanup();
			delete[](player[1].grid);
			delete[](player[2].grid);
			return 0;
		}
	}
	else {
		PLAYER player[3]; //Use player 1 & 2, 0 is ignored
		player[1].map = mapPlayer1;
		player[2].map = mapPlayer2;
		cout << "PLEASE CREATE YOUR OWN FLEE IN TXT FILE !!! " << endl;
		system("pause");
		if (!LoadMap(&player[1])) {
			cout << "Failed to load player 1's map!" << endl;
			return -1;
		}
		vector<char> shipName; //vector saves types of ship
		shipName.push_back('1');

		FOR_ALL_POINT_I_J{
					if (IsNewShip(shipName, player[1].grid[i][j])) shipName.push_back(player[1].grid[i][j]);
		}
		player[1].boom = (player[2].boom = boardSize * boardSize / 2); //config the number of turns = boom

		if (gameMode == 2) {
			player[2].grid = new char* [boardSize];
			for (int i = 0; i < boardSize; i++) {
				(player[2].grid)[i] = new char[boardSize];
			}
			FOR_ALL_POINT_I_J player[2].grid[i][j] = isWATER;
			CreatMapAI(player[1], player[2], shipName);
		}
		else if (!LoadMap(&player[2])) {
			cout << "Failed to load player 2's map!" << endl;
			return -1;
		}

		//ASCII 0 - 9: 48 - 57	|	A - B: 65 - 90


		//for (int i = 0; i < shipName.size(); i++) cout << shipName.at(i);

		/*
		FOR_ALL_POINT_I_J cout << player[1].grid[i][j] << ' ';
		*/

		cout << "PLAYERS' MAPS LOADED, READY TO FIGHT!!!" << endl;
		system("pause");
		//************************************
		//Ready to play the game
		gameRunning = true;
		int thisPlayer = 1;
		int aWin = 0;
		do {
			//Because we are ATTACKING now, the 
			//opposite players board is the display board
			int enemyPlayer;
			if (thisPlayer == 1) enemyPlayer = 2;
			if (thisPlayer == 2) enemyPlayer = 1;
			system("cls");
			DrawBoard(enemyPlayer, player[enemyPlayer], shipName);

			cout << endl << "PLAYER " << thisPlayer << "'S SCORE: " << player[thisPlayer].score << endl;
			cout << "PLAYER " << thisPlayer << "'S BOOMS LEFT: " << player[thisPlayer].boom << endl;
			//Get attack coords from this player
			bool goodInput = false;
			int x = 0, y = 0;
			while (goodInput == false) {
				goodInput = UserInputAttack(x, y, thisPlayer, gameMode, player[enemyPlayer]);
			}
			player[thisPlayer].boom--;
			//Check board; if a ship is there, set as HIT.. otherwise MISS
			for (int thisShip = 0; thisShip < shipName.size(); thisShip++)
				if (player[enemyPlayer].grid[x][y] == shipName.at(thisShip)) {//nice shot
					PlaySound(TEXT("Explosion+2.wav"), NULL, SND_FILENAME | SND_ASYNC);
					//Sleep(3500);
					FOR_ALL_POINT_I_J
						if (player[enemyPlayer].grid[i][j] == shipName.at(thisShip)) {
							player[enemyPlayer].grid[i][j] = isHIT;
							player[thisPlayer].score++;
						}
				}
			if (player[enemyPlayer].grid[x][y] == isWATER) player[enemyPlayer].grid[x][y] = isMISS;

			system("cls");
			DrawBoard(enemyPlayer, player[enemyPlayer], shipName);
			cout << endl;
			system("pause");

			//Check to see if the game is over
			//If 0 is returned, nobody has won yet
			aWin = GameOverCheck(player[thisPlayer], thisPlayer, player[enemyPlayer], enemyPlayer, shipName);
			if (aWin != 0) {
				gameRunning = false;
				break;
			}

			//Alternate between each player as we loop back around
			thisPlayer = (thisPlayer == 1) ? 2 : 1;
		} while (gameRunning);

		if (aWin == 3) {
			system("cls");
			cout << "	WE HAVE A TIE GAME!!! \n\n\n\n";
			delete[](player[1].grid);
			delete[](player[2].grid);
			return 0;
		}
		if (aWin == 2 && gameMode == 2) {
			system("cls");
			cout << "YOU HAVE LOST TO AI!!! \n\n\n\n";
			delete[](player[1].grid);
			delete[](player[2].grid);
			return 0;
		}
		system("cls");
		PlaySound(TEXT("win.wav"), NULL, SND_FILENAME | SND_ASYNC);
		cout << "\n\nCONGRATULATIONS!!!  PLAYER " << aWin << " HAS WON THE GAME!\n\n\n\n";

		system("pause");
		delete[](player[1].grid);
		delete[](player[2].grid);
		return 0;
	}
}